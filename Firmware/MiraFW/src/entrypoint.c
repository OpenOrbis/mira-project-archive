#include <oni/framework.h>
#include <oni/config.h>
#include <oni/init/initparams.h>
#include <oni/utils/types.h>
#include <oni/utils/kdlsym.h>
#include <unistd.h>

#include <oni/messaging/messagemanager.h>
#include <oni/plugins/pluginmanager.h>
#include <oni/rpc/rpcserver.h>

#include <oni/utils/sys_wrappers.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/memory/install.h>
#include <oni/utils/log/logger.h>

#include <mira/boot/patches.h>
#include <mira/plugins/filetransfer/filetransfer_plugin.h>

#include <sys/sysent.h>					// sysent_t
#include <sys/proc.h>					// proc

const char* gNull = "(null)";
uint8_t* gKernelBase = NULL;
struct logger_t* gLogger = NULL;
struct initparams_t* gInitParams = NULL;

struct framework_t* gFramework = NULL;

#define kdlsym_addr_self_orbis_sysvec						 0x019bbcd0

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

	// TODO: Configure parameters
	// TODO: Call Kernel Initialization
	
	// TODO: Send notification with text from userland
	/*
		Credits: CrazyVoid

		int module;
		loadModule("libSceSysUtil.sprx", &module);
		RESOLVE(module, sceSysUtilSendSystemNotificationWithText);

		void notify(char *message) {
			char buffer[512];
			sprintf(buffer, "%s\n\n\n\n\n\n\n", message);
			sceSysUtilSendSystemNotificationWithText(36, 0x10000000, buffer);
		}
	*/

	// TODO: Check for updates (user or kernel?)
	// TODO: Download Updates
	// TODO: Prompt the user to update on restart

	return true;
}

void oni_kernelInitialization(void* args)
/*
	This function handles the kernel (ring-0) mode initialization
*/
{
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
	WriteLog(LL_Info, "[!] oni_kernel_startup in new process!\n");

	gFramework = (struct framework_t*)kmalloc(sizeof(struct framework_t));
	if (!gFramework)
	{
		WriteLog(LL_Error, "could not allocate framework :(");
		kthread_exit();
		return;
	}

	gFramework->configPath = "/user/config";
	gFramework->downloadPath = "/user/download";
	gFramework->homePath = ONI_BASE_PATH;

	// Initialize the rpc dispatcher
	WriteLog(LL_Debug, "MessageManager initialization");
	gFramework->messageManager = (struct messagemanager_t*)kmalloc(sizeof(struct messagemanager_t));
	if (!gFramework->messageManager)
	{
		kthread_exit();
		return;
	}
	messagemanager_init(gFramework->messageManager);

	// Initialize the plugin manager
	WriteLog(LL_Debug, "[+] Initializing plugin manager");
	gFramework->pluginManager = (struct pluginmanager_t*)kmalloc(sizeof(struct pluginmanager_t));
	if (!gFramework->pluginManager)
	{
		kthread_exit();
		return;
	}
	pluginmanager_init(gFramework->pluginManager);

	// Initialize default plugins
	{
		// Register file transfer plugin
		struct filetransfer_plugin_t* filetransferPlugin = (struct filetransfer_plugin_t*)kmalloc(sizeof(struct filetransfer_plugin_t));
		if (!filetransferPlugin)
		{
			WriteLog(LL_Error, "Error allocating file transfer plugin");
			kthread_exit();
			return;
		}
		filetransfer_plugin_init(filetransferPlugin);
		pluginmanager_registerPlugin(gFramework->pluginManager, &filetransferPlugin->plugin);
	}

	// Kick off the rpc server thread
	WriteLog(LL_Debug, "[+] Allocating rpc server");
	gFramework->rpcServer = (struct rpcserver_t*)kmalloc(sizeof(struct rpcserver_t));
	if (!gFramework->rpcServer)
	{
		kthread_exit();
		return;
	}
	rpcserver_init(gFramework->rpcServer, gInitParams->process);
	WriteLog(LL_Debug, "[+] Finished Initializing rpc server proc: %p", gInitParams->process);

	// Startup the server, it will kick off the thread
	if (!rpcserver_startup(gFramework->rpcServer, 9999))
	{
		WriteLog(LL_Error, "[-] rpcserver_startup failed");
		kthread_exit();
		return;
	}

	// At this point we don't need kernel context anymore
	WriteLog(LL_Info, "[!] Mira initialization complete");
	kthread_exit();
}