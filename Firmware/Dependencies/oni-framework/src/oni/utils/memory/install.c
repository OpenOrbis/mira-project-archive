#include <oni/utils/memory/install.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/kernel.h>
#include <oni/init/initparams.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/patches.h>

#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/proc.h>
#include <sys/kthread.h>
#include <sys/imgact.h>
#include <sys/filedesc.h>
#include <vm/vm.h>
#include <vm/vm_page.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>
#include <vm/vm_param.h>
#include <unistd.h>

#include <oni/utils/cpu.h>
#include <oni/utils/syscall.h>

struct kexec_uap
{
	void* func;
	void* arg0;
};

void SelfElevateAndRunPayloadStage2(struct thread* td, struct kexec_uap* uap);

uint8_t SelfElevateAndRun(struct initparams_t* userInitParams)
{
	// Verify arguments are valid, but do not deref here
	if (!userInitParams)
		return false;

	if (userInitParams->isElf)
		return false;
	else
		syscall2(11, SelfElevateAndRunPayloadStage2, userInitParams);

	return true;
}

void SelfElevateAndRunPayloadStage2(struct thread* td, struct kexec_uap* uap)
{
	// If we do not have a valid parameter passed, kick back
	if (!uap->arg0)
		return;

	struct initparams_t* userInitParams = uap->arg0;

	// Fill the kernel base address
	gKernelBase = (uint8_t*)kernelRdmsr(0xC0000082) - kdlsym_addr_Xfast_syscall;

	//0x40000;
	void(*critical_enter)(void) = kdlsym(critical_enter);
	void(*crtical_exit)(void) = kdlsym(critical_exit);
	vm_offset_t(*kmem_alloc)(vm_map_t map, vm_size_t size) = kdlsym(kmem_alloc);
	void(*kmem_free)(void* map, void* addr, size_t size) = kdlsym(kmem_free);
	void(*printf)(char *format, ...) = kdlsym(printf);
	int(*kproc_create)(void(*func)(void*), void* arg, struct proc** newpp, int flags, int pages, const char* fmt, ...) = kdlsym(kproc_create);
	vm_map_t map = (vm_map_t)(*(uint64_t *)(kdlsym(kernel_map)));
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);
	int(*copyin)(const void* uaddr, void* kaddr, size_t len) = kdlsym(copyin);

	// Escalate privileges & Escape the sandbox of the payload executer

	struct ucred* cred = td->td_proc->p_ucred;
	struct filedesc* fd = td->td_proc->p_fd;

	cred->cr_uid = 0;
	cred->cr_ruid = 0;
	cred->cr_rgid = 0;
	cred->cr_groups[0] = 0;

	cred->cr_prison = *(void**)kdlsym(prison0);
	fd->fd_rdir = fd->fd_jdir = *(void**)kdlsym(rootvnode);

	// Apply patches
	critical_enter();
	cpu_disable_wp();

	oni_installPrePatches();

	cpu_enable_wp();
	crtical_exit();

	printf("fucknutzzzzzz: %p\n", userInitParams);

	// Create launch parameters, this is floating in "free kernel space" so the other process should
	// be able to grab and use the pointer directly
	struct initparams_t* initParams = (struct initparams_t*)kmem_alloc(map, sizeof(struct initparams_t));
	if (!initParams)
	{
		printf("[-] could not allocate initialization parameters.\n");
		return;
	}
	memset(initParams, 0, sizeof(*initParams));

	// Copyin our new arguments from userland
	int copyResult = copyin(userInitParams, initParams, sizeof(*initParams));
	if (copyResult != 0)
	{
		kmem_free(map, initParams, sizeof(*initParams));
		printf("[-] could not copyin initalization parameters (%d)\n", copyResult);
		return;
	}

	// Verify that the entry point is non-null
	if (initParams->entrypoint == NULL)
	{
		kmem_free(map, initParams, sizeof(*initParams));
		printf("[-] invalid entry point, aborting.");
		return;
	}

	printf("[*] b: %llx s: %llx ep: %llx\n", initParams->payloadBase, initParams->payloadSize, initParams->entrypoint);

	// Verify that the entry point is within bounds
	if (((uint64_t)initParams->entrypoint) < initParams->payloadBase ||
		((uint64_t)initParams->entrypoint) > initParams->payloadBase + initParams->payloadSize)
	{
		kmem_free(map, initParams, sizeof(*initParams));
		printf("[-] invalid entry point, out of bounds aborting.");
		return;
	}

	// We are ignoring any of the process information passed from userland
	initParams->process = NULL;

	uint64_t payloadSize = initParams->payloadSize;
	uint64_t payloadBase = initParams->payloadBase;

	printf("[*] Got userland payload %p %x\n", payloadBase, payloadSize);

	// Allocate some memory
	uint8_t* kernelPayload = (uint8_t*)kmem_alloc(map, payloadSize);
	if (!kernelPayload)
	{
		kmem_free(map, initParams, sizeof(*initParams));
		printf("[-] could not allocate kernel payload.\n");
		return;
	}

	printf("[+] Allocated kernel executable memory %p\n", kernelPayload);

	// Find the exported init_kernelStartup symbol
	uint64_t kernelStartupSlide = (uint64_t)initParams->entrypoint - (uint64_t)payloadBase;
	printf("[*] Kernel startup slide %x\n", kernelStartupSlide);

	uint8_t* kernelStartup = kernelPayload + kernelStartupSlide;
	if (!kernelStartup)
		return;

	printf("[*] Kernel startup address %p\n", kernelStartup);

	// Update the entrypoint from a userland offset, to the new kernel entry point
	initParams->entrypoint = (void(*)(void*))kernelStartup;

	printf("[+] Copying payload from user-land\n");
	memset(kernelPayload, 0, payloadSize);

	copyResult = copyin((void*)payloadBase, kernelPayload, payloadSize);
	if (copyResult != 0)
	{
		// Intentionally blow the fuck up
		printf("fuck, this is bad...\n");
		for (;;)
			__asm__("nop");
	}

	memcpy(kernelPayload, (void*)payloadBase, payloadSize);

	initParams->payloadBase = (uint64_t)kernelPayload;

	printf("[*] Kernel payload peek: %02X %02X %02X %02X %02X\n", kernelPayload[0], kernelPayload[1], kernelPayload[2], kernelPayload[3], kernelPayload[4]);
	printf("[*] Kernel oni_kernel_startup peek: %02X %02X %02X %02X %02X\n", kernelStartup[0], kernelStartup[1], kernelStartup[2], kernelStartup[3], kernelStartup[4]);

	// Create new process
	printf("[+] Calling kproc_create(%p, %p, %p, %d, %d, %s);\n", kernelStartup, initParams, &initParams->process, 0, 0, "install");

	critical_enter();
	int processCreateResult = kproc_create((void(*)(void*))kernelStartup, initParams, &initParams->process, 0, 0, "install");
	crtical_exit();

	if (processCreateResult != 0)
		printf("[-] Failed to create process.\n");
	else
		printf("[+] Kernel process created. Result %d\n", processCreateResult);
}