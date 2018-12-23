//
//	Oni-Framework core
//
#include <oni/plugins/pluginmanager.h>
#include <oni/messaging/messagemanager.h>
#include <oni/rpc/pbserver.h>
#include <oni/init/initparams.h>
#include <oni/framework.h>
#include <oni/config.h>

//
//	Mira Core
//
#include <mira/miraframework.h>

//
//	Console specific patches
//
#include <oni/boot/patches.h>

//
//	Utilities
//
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/memory/install.h>
#include <oni/utils/logger.h>
#include <oni/utils/syscall.h>
#include <oni/utils/types.h>
#include <oni/utils/kernel.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/dynlib.h>
#include <oni/utils/escape.h>

//
//	Free-BSD Specifics
//
#include <sys/sysent.h>					// sysent_t
#include <sys/proc.h>					// proc
#include <sys/filedesc.h>				// filedesc

//
//	Required framework variables
//
const char* gNull = "(null)";
uint8_t* gKernelBase = NULL;
struct logger_t* gLogger = NULL;
struct initparams_t* gInitParams = NULL;
struct framework_t* gFramework = NULL;

void mira_entry(void* args)
/*
	This is the entry point for the userland or kernel payload

	args - pointer to struct initparams_t in kernel memory or NULL if launching from userland
*/
{
	if (!args)
		return;

	struct initparams_t* initParams = (struct initparams_t*)args;

	gInitParams = initParams;

	oni_kernelInitialization(args);
	// If we have args at all, we will assume that we are running in the kernel context
	//if (args)
	//{
	//	// Init oni framework in kernel space
	//	
	//}
	//else // Handle userland launching the old fashion way
	//{
	//	//struct initparams_t userParams;
	//	//userParams.entrypoint = oni_kernelInitialization;
	//	//userParams.process = NULL;
	//	//userParams.payloadBase = 0x926200000;
	//	//userParams.payloadSize = 0x80000;

	//	//SelfElevateAndRun(&userParams);

	//	//// Prompt the user
	//	//int moduleId = -1;
	//	//sys_dynlib_load_prx("/system/common/lib/libSceSysUtil.sprx", &moduleId);

	//	//// This header doesn't work in > 5.00
	//	//int(*sceSysUtilSendSystemNotificationWithText)(int messageType, char* message) = NULL;

	//	//sys_dynlib_dlsym(moduleId, "sceSysUtilSendSystemNotificationWithText", &sceSysUtilSendSystemNotificationWithText);

	//	//if (sceSysUtilSendSystemNotificationWithText)
	//	//{
	//	//	char* initMessage = "Mira Project Loaded\nRPC Server Port: 9999\nkLog Server Port: 9998\n";
	//	//	sceSysUtilSendSystemNotificationWithText(222, initMessage);
	//	//}

	//	//sys_dynlib_unload_prx(moduleId);
	//}
}

#include <sys/mman.h>

void oni_kernelInitialization(void* args)
/*
This function handles the kernel (ring-0) mode initialization
*/
{

	// Fill the kernel base address
	gKernelBase = (uint8_t*)kernelRdmsr(0xC0000082) - kdlsym_addr_Xfast_syscall;

	void(*kthread_exit)(void) = kdlsym(kthread_exit);
	struct vmspace* (*vmspace_alloc)(vm_offset_t min, vm_offset_t max) = kdlsym(vmspace_alloc);
	void(*pmap_activate)(struct thread *td) = kdlsym(pmap_activate);
	struct sysentvec* sv = kdlsym(self_orbis_sysvec);
	void(*printf)(char *format, ...) = kdlsym(printf);

	// Let'em know we made it
	printf("[+] mira has reached stage 2\n");

	printf("[+] starting logging\n");
	gLogger = (struct logger_t*)kmalloc(sizeof(struct logger_t));
	if (!gLogger)
	{
		printf("[-] could not allocate logger\n");
		kthread_exit();
		return;
	}
	logger_init(gLogger);

	// Verify that our initialization parameters are correct, this is coming from the kernel copy
	gInitParams = (struct initparams_t*)args;
	if (!gInitParams)
	{
		WriteLog(LL_Error, "invalid loader init params");
		kthread_exit();
		return;
	}

	// Create new vm_space
	WriteLog(LL_Debug, "Creating new vm space");

	vm_offset_t sv_minuser = MAX(sv->sv_minuser, PAGE_SIZE);
	struct vmspace* vmspace = vmspace_alloc(sv_minuser, sv->sv_maxuser);
	if (!vmspace)
	{
		WriteLog(LL_Error, "vmspace_alloc failed\n");
		kthread_exit();
		return;
	}

	// Assign our new vmspace to our process
	gInitParams->process->p_vmspace = vmspace;
	if (gInitParams->process == curthread->td_proc)
		pmap_activate(curthread);

	// Because we have now forked into a new realm of fuckery
	// We need to reserve the first 3 file descriptors in our process
	int descriptor = kopen("/dev/console", 1, 0);
	kdup2(descriptor, 1);
	kdup2(1, 2);

	// Root and escape our thread
	oni_threadEscape(curthread, NULL);

	// Show over UART that we are running in a new process
	WriteLog(LL_Info, "oni_kernelInitialization in new process!\n");

	// Initialize miraframework
	gFramework = (struct framework_t*)mira_getFramework();
	if (!gFramework)
	{
		WriteLog(LL_Error, "FATAL ERROR: could not initialize the framework");
		kthread_exit();
	}
	miraframework_initialize((struct miraframework_t*)gFramework);

	// Set the initparams so we do not lose track of it
	((struct miraframework_t*)gFramework)->initParams = gInitParams;

	// At this point we don't need kernel context anymore
	WriteLog(LL_Info, "Mira initialization complete");
	kthread_exit();
}
