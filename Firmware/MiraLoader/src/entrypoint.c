#include <oni/utils/types.h>

#include <loader/elfloader.h>
#include <utils/notify.h>
#include <utils/utils.h>

#include <oni/init/initparams.h>
#include <oni/utils/syscall.h>
#include <oni/utils/dynlib.h>
#include <oni/utils/escape.h>
#include <oni/utils/patches.h>
#include <oni/utils/kernel.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/cpu.h>
#include <oni/utils/logger.h>
#include <oni/utils/sys_wrappers.h>


#include <sys/elf64.h>
#include <sys/socket.h>
#include <sys/proc.h>
#include <sys/unistd.h>
#include <sys/proc.h>
#include <sys/kthread.h>
#include <sys/imgact.h>
#include <sys/filedesc.h>
#include <sys/malloc.h>
#include <vm/vm.h>
#include <vm/vm_page.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>
#include <vm/vm_param.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <netinet/in.h>

struct kexec_uap
{
	void* func;
	void* arg0;
};

struct logger_t* gLogger = NULL;

void miraloader_kernelInitialization(struct thread* td, struct kexec_uap* uap);

void mira_escape(struct thread* td, void* uap)
{
	gKernelBase = (uint8_t*)kernelRdmsr(0xC0000082) - kdlsym_addr_Xfast_syscall;

	void(*critical_enter)(void) = kdlsym(critical_enter);
	void(*crtical_exit)(void) = kdlsym(critical_exit);

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
}


void* mira_entry(void* args)
{
	// Escape the jail and sandbox
	syscall2(11, mira_escape, NULL);

	//int32_t sysUtilModuleId = -1;
	int32_t netModuleId = -1;
	int32_t libcModuleId = -1;
	//int32_t libKernelWebModuleId = -1;

	{
		sys_dynlib_load_prx("libSceLibcInternal.sprx", &libcModuleId);

		sys_dynlib_dlsym(libcModuleId, "snprintf", &snprintf);
	}

	// Networking resolving
	{
		sys_dynlib_load_prx("libSceNet.sprx", &netModuleId);

		sys_dynlib_dlsym(netModuleId, "sceNetSocket", &sceNetSocket);
		sys_dynlib_dlsym(netModuleId, "sceNetSocketClose", &sceNetSocketClose);
		sys_dynlib_dlsym(netModuleId, "sceNetBind", &sceNetBind);
		sys_dynlib_dlsym(netModuleId, "sceNetListen", &sceNetListen);
		sys_dynlib_dlsym(netModuleId, "sceNetAccept", &sceNetAccept);
		sys_dynlib_dlsym(netModuleId, "sceNetRecv", &sceNetRecv);
	}



	// Allocate a 3MB buffer
	size_t bufferSize = ALLOC_5MB;
	uint8_t* buffer = (uint8_t*)_mmap(NULL, bufferSize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (!buffer)
	{
		WriteNotificationLog("could not allocate 5MB buffer");
		return NULL;
	}
	loader_memset(buffer, 0, bufferSize);

	// Hold our server socket address
	struct sockaddr_in serverAddress;
	loader_memset(&serverAddress, 0, sizeof(serverAddress));

	// Listen on port 9021
	serverAddress.sin_len = sizeof(serverAddress);
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = __bswap16(9021); // port 9020
	serverAddress.sin_family = AF_INET;

	// Create a new socket
	int32_t serverSocket = sceNetSocket("miraldr", AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0)
	{
		WriteNotificationLog("socket error");
		return NULL;
	}

	// Bind to localhost
	int32_t result = sceNetBind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	if (result < 0)
	{
		WriteNotificationLog("bind error");
		return NULL;
	}

	// Listen
	result = sceNetListen(serverSocket, 10);
	if (result < 0)
	{
		WriteNotificationLog("listen error");
		return NULL;
	}

	WriteNotificationLog("waiting for clients");

	// Wait for a client to send something
	int32_t clientSocket = sceNetAccept(serverSocket, NULL, NULL);
	if (clientSocket < 0)
	{
		WriteNotificationLog("accept errror");
		return NULL;
	}

	int32_t currentSize = 0;
	int32_t recvSize = 0;

	// Recv one byte at a time until we get our buffer
	while ((recvSize = sceNetRecv(clientSocket, buffer + currentSize, bufferSize - currentSize, 0)) > 0)
		currentSize += recvSize;

	// Close the client and server socket connections
	sceNetSocketClose(clientSocket);
	sceNetSocketClose(serverSocket);

	// Determine if we launch a elf or a payload
	if (buffer[0] == ELFMAG0 &&
		buffer[1] == ELFMAG1 &&
		buffer[2] == ELFMAG2 &&
		buffer[3] == ELFMAG3) // 0x7F 'ELF'
	{
		// Launch ELF
		// TODO: Check/Add a flag to the elf that determines if this is a kernel or userland elf
		ElfLoader_t loader;
		loader.isKernel = false;

		if (!elfloader_initFromMemory(&loader, buffer, currentSize))
		{
			WriteNotificationLog("could not init from memory");
			return NULL;
		}

		if (!elfloader_isElfValid(&loader))
		{
			WriteNotificationLog("elf not valid");
			return NULL;
		}

		if (!elfloader_handleRelocations(&loader))
		{
			WriteNotificationLog("could not handle relocation");
			return NULL;
		}

		void(*entryPoint)(void*) = NULL;
		if (!elfloader_getSymbolAddress(&loader, "mira_entry", (void**)&entryPoint))
		{
			WriteNotificationLog("could not find mira_entry");
			return NULL;
		}

		//if (!loader.elfMain)
		//{
		//	WriteNotificationLog("could not find main");
		//	return NULL;
		//}

		// TODO: Check for custom Mira elf section to determine launch type
		char buf[64];
		loader_memset(buf, 0, sizeof(buf));

		snprintf(buf, sizeof(buf), "elf: %p elfSize: %llx", buffer, bufferSize);
		WriteNotificationLog(buf);

		// Launch kernel
		uint8_t isKernelElf = true;
		if (isKernelElf)
		{
			struct initparams_t initParams;
			initParams.entrypoint = NULL;
			initParams.isElf = true;
			initParams.payloadBase = (uint64_t)buffer;
			initParams.payloadSize = bufferSize;
			initParams.process = NULL;

			loader.isKernel = true;

			syscall2(11, miraloader_kernelInitialization, &initParams);
		}
		else // Launch userland
			entryPoint(NULL);
	}
	else
	{
		// Launch Userland Payload
		WriteNotificationLog("launching payload");

		void(*payload_start)() = (void(*)())buffer;
		payload_start();
	}

	return NULL;
}

void miraloader_kernelInitialization(struct thread* td, struct kexec_uap* uap)
{
	// If we do not have a valid parameter passed, kick back
	if (!uap || !uap->arg0)
		return;

	struct initparams_t* userInitParams = uap->arg0;

	// Thread should already be escaped from earlier

	// Fill the kernel base address
	gKernelBase = (uint8_t*)kernelRdmsr(0xC0000082) - kdlsym_addr_Xfast_syscall;
	void(*critical_enter)(void) = kdlsym(critical_enter);
	void(*crtical_exit)(void) = kdlsym(critical_exit);
	vm_offset_t(*kmem_alloc)(vm_map_t map, vm_size_t size) = kdlsym(kmem_alloc);
	void(*kmem_free)(void* map, void* addr, size_t size) = kdlsym(kmem_free);
	void(*printf)(char *format, ...) = kdlsym(printf);
	int(*kproc_create)(void(*func)(void*), void* arg, struct proc** newpp, int flags, int pages, const char* fmt, ...) = kdlsym(kproc_create);
	vm_map_t map = (vm_map_t)(*(uint64_t *)(kdlsym(kernel_map)));
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	int(*copyin)(const void* uaddr, void* kaddr, size_t len) = kdlsym(copyin);

	// Allocate a new logger for the MiraLoader
	gLogger = (struct logger_t*)kmem_alloc(map, sizeof(struct logger_t));
	if (!gLogger)
	{
		printf("[-] could not allocate logger\n");
		return;
	}
	logger_init(gLogger);

	// Create launch parameters, this is floating in "free kernel space" so the other process should
	// be able to grab and use the pointer directly
	struct initparams_t* initParams = (struct initparams_t*)kmem_alloc(map, sizeof(struct initparams_t));
	if (!initParams)
	{
		WriteLog(LL_Error, "could not allocate initialization parameters.\n");
		return;
	}
	memset(initParams, 0, sizeof(*initParams));

	// Copyin our new arguments from userland
	int copyResult = copyin(userInitParams, initParams, sizeof(*initParams));
	if (copyResult != 0)
	{
		kmem_free(map, initParams, sizeof(*initParams));
		WriteLog(LL_Error, "could not copyin initalization parameters (%d)\n", copyResult);
		return;
	}

	// initparams are read from the uap in this syscall func
	uint64_t payloadSize = initParams->payloadSize;
	uint64_t payloadBase = initParams->payloadBase;

	// Allocate some memory
	uint8_t* kernelElf = (uint8_t*)kmem_alloc(map, payloadSize);
	if (!kernelElf)
	{
		// Free the previously allocated initialization parameters
		kmem_free(map, initParams, sizeof(*initParams));
		WriteLog(LL_Error, "could not allocate kernel payload.\n");
		return;
	}
	memset(kernelElf, 0, payloadSize);
	WriteLog(LL_Debug, "payloadBase: %p payloadSize: %llx kernelElf: %p\n", payloadBase, payloadSize, kernelElf);

	// Copy the ELF data from userland
	copyResult = copyin((const void*)payloadBase, kernelElf, payloadSize);
	if (copyResult != 0)
	{
		// Intentionally blow the fuck up
		WriteLog(LL_Error, "fuck, this is bad...\n");
		for (;;)
			__asm__("nop");
	}

	WriteLog(LL_Debug, "finished allocating and copying ELF from userland");

	// Determine if we launch a elf or a payload
	uint32_t magic = *(uint32_t*)kernelElf;
	if (magic != 0x464C457F)
	{
		printf("invalid elf header.\n");
		return;
	}
	WriteLog(LL_Debug, "elf header: %X\n", magic);

	// Launch ELF
	ElfLoader_t* loader = (ElfLoader_t*)kmem_alloc(map, sizeof(ElfLoader_t)); //malloc(sizeof(ElfLoader_t), M_LINKER, M_WAITOK);
	if (!loader)
	{
		printf("could not allocate loader\n");
		return;
	}

	// Don't forget to set the kernel flag in the loader
	loader->isKernel = true;

	WriteLog(LL_Debug, "loader allocated and zeroed\n");

	// Initialize (which copies the ELF to it's own memory owned by loader)
	if (!elfloader_initFromMemory(loader, kernelElf, payloadSize))
	{
		WriteLog(LL_Error, "could not init from memory\n");
		return;
	}
	WriteLog(LL_Debug, "initialized from memory\n");

	// Check in the loader if we have a valid ELF file
	if (!elfloader_isElfValid(loader))
	{
		printf("elf not valid");
		return;
	}
	WriteLog(LL_Debug, "elf validated\n");

	// Handle all ELF relocations
	if (!elfloader_handleRelocations(loader))
	{
		WriteLog(LL_Error, "could not handle relocation");
		return;
	}
	WriteLog(LL_Debug, "relocations handled\n");

	void(*entryPoint)(void*) = NULL;
	if (!elfloader_getSymbolAddress(loader, "mira_entry", (void**)&entryPoint))
	{
		WriteNotificationLog("could not find mira_entry");
		return;
	}

	// Get the main entry point
	if (!entryPoint)
	{
		WriteLog(LL_Error, "could not find main");
		return;
	}
	WriteLog(LL_Debug, "elfMain: %p\n", entryPoint);

	// Update the initialization parameters with the new data
	initParams->process = NULL;
	initParams->payloadBase = (uint64_t)loader->data;
	initParams->payloadSize = loader->dataSize;
	initParams->isElf = true;


#ifdef _DSADASD
	oni_threadEscape(curthread, NULL);

	int32_t fd = kopen("/mnt/usb0/dump.bin", O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		WriteLog(LL_Error, "could not open file for writing (%d).", fd);
		return;
	}

	ssize_t bytesWritten = kwrite(fd, loader->data, loader->dataSize);
	if (bytesWritten < 0)
	{
		WriteLog(LL_Error, "could not write to file (%d).", bytesWritten);
		return;
	}

	kclose(fd);

	WriteLog(LL_Debug, "elf written to usb");
	return;
#endif

	critical_enter();
	int processCreateResult = kproc_create(entryPoint, initParams, &initParams->process, 0, 0, "miraldr2");
	
	if (processCreateResult != 0)
		WriteLog(LL_Error, "failed to create process.\n");
	else
		WriteLog(LL_Debug, "kernel process created. result %d\n", processCreateResult);
	crtical_exit();

	// Since the ELF loader allocates it's own buffer, we can free our temp one
	kmem_free(map, kernelElf, payloadSize);
	kernelElf = NULL;
}