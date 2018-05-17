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
#include <mira/plugins/pluginloader.h>	// Load plugins from file

//
//	Console specific patches
//
#include <mira/boot/patches.h>

//
//	Built-in plugins
//
#include <mira/plugins/filetransfer/filetransfer_plugin.h>
#include <mira/plugins/logserver/logserver_plugin.h>
#include <mira/plugins/debugger/debugger_plugin.h>

//
//	Utilities
//
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/memory/install.h>
#include <oni/utils/log/logger.h>
#include <oni/utils/syscall.h>
#include <oni/utils/types.h>
#include <oni/utils/kdlsym.h>

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

// TODO: Enable better management of plugins
struct debugger_plugin_t* gDebugger = NULL;

// TODO: Move this somewhere
#define kdlsym_addr_self_orbis_sysvec						 0x019bbcd0

// Forward declarations
int init_oni();
uint8_t __noinline mira_installDefaultPlugins();

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
	
	// Prompt the user
	int moduleId = syscall1(594, "libSceSysUtil.sprx");
	int(*sceSysUtilSendSystemNotificationWithText)(int messageType, int userID, char* message) = NULL;

	syscall3(591, (void*)moduleId, (void*)"sceSysUtilSendSystemNotificationWithText", (void*)&sceSysUtilSendSystemNotificationWithText);

	if (sceSysUtilSendSystemNotificationWithText)
	{
		char* initMessage = "Mira Project Loaded\nRPC Server Port: 9999\nkLog Server Port: 9998\n";
		sceSysUtilSendSystemNotificationWithText(36, 0x10000000, initMessage);
	}

	return true;
}

struct hook_t* gHook = NULL;

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

	// Set configuration paths
	gFramework->homePath = "/user/mira";
	gFramework->configPath = "/user/mira/config.ini";
	gFramework->downloadPath = "/user/mira/download";
	gFramework->pluginsPath = "/user/mira/plugins";

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

	// Initialize the default plugins
	if (!mira_installDefaultPlugins())
	{
		WriteLog(LL_Error, "could not initialize plugins");
		kthread_exit();
		return;
	}

	// At this point we don't need kernel context anymore
	WriteLog(LL_Info, "[!] Mira initialization complete");
	kthread_exit();
}

uint8_t __noinline mira_installDefaultPlugins()
{
	// Initialize default plugins

	// Register file transfer plugin
	struct filetransfer_plugin_t* filetransferPlugin = (struct filetransfer_plugin_t*)kmalloc(sizeof(struct filetransfer_plugin_t));
	if (!filetransferPlugin)
	{
		WriteLog(LL_Error, "Error allocating file transfer plugin");
		return false;
	}
	filetransfer_plugin_init(filetransferPlugin);
	pluginmanager_registerPlugin(gFramework->pluginManager, &filetransferPlugin->plugin);

	WriteLog(LL_Info, "Allocating logserver");
	struct logserver_plugin_t* logServer = (struct logserver_plugin_t*)kmalloc(sizeof(struct logserver_plugin_t));
	if (!logServer)
	{
		WriteLog(LL_Error, "Could not allocate log server.");
		return false;
	}
	logserver_init(logServer);
	pluginmanager_registerPlugin(gFramework->pluginManager, &logServer->plugin);

	// Initialize the plugin loader to read from file
	struct pluginloader_t* pluginLoader = (struct pluginloader_t*)kmalloc(sizeof(struct pluginloader_t));
	if (!pluginLoader)
	{
		WriteLog(LL_Error, "Error allocating plugin loader.");
		return false;
	}
	pluginloader_init(pluginLoader);

	pluginloader_loadPlugins(pluginLoader);

	// Debugger
	WriteLog(LL_Debug, "Allocating debugger");
	gDebugger = (struct debugger_plugin_t*)kmalloc(sizeof(struct debugger_plugin_t));
	if (!gDebugger)
	{
		WriteLog(LL_Error, "could not allocate debugger plugin");
		return false;
	}
	debugger_plugin_init(gDebugger);
	pluginmanager_registerPlugin(gFramework->pluginManager, &gDebugger->plugin);

	// Kick off the rpc server thread
	WriteLog(LL_Debug, "[+] Allocating rpc server");
	gFramework->rpcServer = (struct rpcserver_t*)kmalloc(sizeof(struct rpcserver_t));
	if (!gFramework->rpcServer)
	{
		WriteLog(LL_Error, "could not allocate rpc server.");
		return false;
	}
	rpcserver_init(gFramework->rpcServer, gInitParams->process);
	
	WriteLog(LL_Debug, "[+] Finished Initializing rpc server proc: %p", gInitParams->process);

	// Startup the server, it will kick off the thread
	if (!rpcserver_startup(gFramework->rpcServer, 9999))
	{
		WriteLog(LL_Error, "[-] rpcserver_startup failed");
		return false;
	}

	return true;
}