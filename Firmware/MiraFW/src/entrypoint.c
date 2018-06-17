//
//	Oni-Framework core
//
#include <oni/plugins/pluginmanager.h>
#include <oni/messaging/messagemanager.h>
#include <oni/rpc/rpcserver.h>
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
#include <oni/utils/kdlsym.h>
#include <oni/utils/dynlib.h>

//
//	Free-BSD Specifics
//
#include <sys/sysent.h>					// sysent_t
#include <sys/proc.h>					// proc

//
//	Required framework variables
//
const char* gNull = "(null)";
uint8_t* gKernelBase = NULL;
struct logger_t* gLogger = NULL;
struct initparams_t* gInitParams = NULL;
struct framework_t* gFramework = NULL;

// Forward declarations
int init_oni();

void* mira_entry(void* args)
/*
	This is the entry point for the userland payload

	args - pointer to struct initparams_t in userland memory
*/
{
	// Initialize the Oni Framework
	int result = init_oni();
	if (!result)
		return NULL;

	// Prompt the user
	int64_t moduleId = sys_dynlib_load_prx("libSceSysUtil.sprx");
	int(*sceSysUtilSendSystemNotificationWithText)(int messageType, int userID, char* message) = NULL;

	// TODO: Fix, this call fails and never populates sceSysUtilSendSystemNotificationWithText investigate why
	sys_dynlib_dlsym(moduleId, "sceSysUtilSendSystemNotificationWithText", &sceSysUtilSendSystemNotificationWithText);

	if (sceSysUtilSendSystemNotificationWithText)
	{
		char* initMessage = "Mira Project Loaded\nRPC Server Port: 9999\nkLog Server Port: 9998\n";
		sceSysUtilSendSystemNotificationWithText(36, 0x10000000, initMessage);
	}

	sys_dynlib_unload_prx(moduleId);

	return NULL;
}

int init_oni()
/*
	This is the OniFramework entry where platform specific
	configurations should be set up and the framework initialized
*/
{
	// Elevate to kernel
	SelfElevateAndRun((uint8_t*)0x926200000, 0x80000, oni_kernelInitialization);

	return true;
}

struct hook_t* gHook = NULL;

void oni_kernelInitialization(void* args)
/*
	This function handles the kernel (ring-0) mode initialization
*/
{
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

	// Verify that our initialization parameters are correct
	struct initparams_t* loaderInitParams = (struct initparams_t*)args;
	if (!loaderInitParams)
	{
		WriteLog(LL_Error, "invalid loader init params");
		kthread_exit();
		return;
	}

	gInitParams = (struct initparams_t*)kmalloc(sizeof(struct initparams_t));
	if (!gInitParams) // TODO: Do a better job
	{
		WriteLog(LL_Error, "invalid initparams");
		kthread_exit();
		return;
	}

	gInitParams->payloadBase = loaderInitParams->payloadBase;
	gInitParams->payloadSize = loaderInitParams->payloadSize;
	gInitParams->process = loaderInitParams->process;

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

	// create our own cred
	ksetuid(0);

	// set diag auth ID flags
	curthread->td_ucred->cr_sceAuthID = 0x3800000000000007ULL;

	// make system credentials
	curthread->td_ucred->cr_sceCaps[0] = 0xFFFFFFFFFFFFFFFFULL;
	curthread->td_ucred->cr_sceCaps[1] = 0xFFFFFFFFFFFFFFFFULL;

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

	// At this point we don't need kernel context anymore
	WriteLog(LL_Info, "Mira initialization complete");
	kthread_exit();
}