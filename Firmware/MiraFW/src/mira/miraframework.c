#include "miraframework.h"


//
//	Oni-Framework core
//
#include <oni/messaging/messagemanager.h>
#include <oni/rpc/rpcserver.h>
#include <oni/init/initparams.h>

//
//	Built-in plugins
//
#include <oni/plugins/pluginmanager.h>
#include <mira/plugins/filetransfer/filetransfer_plugin.h>
#include <mira/plugins/logserver/logserver_plugin.h>
#include <mira/plugins/debugger/debugger_plugin.h>
#include <mira/plugins/pluginloader.h>	// Load plugins from file
#include <mira/plugins/orbisutils/orbisutils_plugin.h>
#include <mira/plugins/cheat/cheat_plugin.h>

//
//	Utilities
//
#include <oni/utils/logger.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/cpu.h>

#include <oni/utils/hook.h>

//
//	Filesystems
//
#include <mira/fs/overlay/overlayfs.h>

//
//	Free-BSD Specifics
//
#include <sys/eventhandler.h>
#include <sys/proc.h>					// proc
#include <sys/sysent.h>

#define MIRA_CONFIG_PATH	"/user/mira.ini"

uint8_t __noinline mira_installDefaultPlugins();
uint8_t miraframework_installHandlers(struct miraframework_t* framework);
uint8_t miraframework_loadSettings(struct miraframework_t* framework, const char* iniPath);
uint8_t miraframework_installHooks(struct miraframework_t* framework);

//
//	Event hadling stuff
//

// Handle the kernel panics due to suspending
//static void mira_onSuspend(struct miraframework_t* framework);
//static void mira_onResume(struct miraframework_t* framework);
//static void mira_onShutdown(struct miraframework_t* framework);

// Handle execution of new processes for trainers
//static void mira_onExec(struct miraframework_t* framework);

typedef void(*callback_fn)(struct miraframework_t*);

//
//	Start code
//

struct miraframework_t* mira_getFramework()
{
	void * (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	if (!gFramework)
	{
		// We intentionally allocate the larger struct and cast to the basic
		struct miraframework_t* framework = (struct miraframework_t*)kmalloc(sizeof(struct miraframework_t));
		memset(framework, 0, sizeof(*framework));

		if (!framework)
		{
			// Consider this a fatal error
			WriteLog(LL_Error, "could not allocate framework.");

			for (;;)
				__asm__("nop");

			return NULL;
		}

		// Assign the global variable
		gFramework = (struct framework_t*)framework;
	}

	return (struct miraframework_t*)gFramework;
}

uint8_t miraframework_initialize(struct miraframework_t* framework)
{
	if (!framework)
		return false;

	// Zero initialize everything
	void * (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	memset(framework, 0, sizeof(*framework));

	// Load the settings from file if it exists
	WriteLog(LL_Info, "loading settings from %s", MIRA_CONFIG_PATH);
	if (miraframework_loadSettings(framework, MIRA_CONFIG_PATH))
		WriteLog(LL_Error, "mira configuration file not found, skipping...");

	// Initialize the message manager
	WriteLog(LL_Debug, "initializing message manager");
	framework->framework.messageManager = (struct messagemanager_t*)kmalloc(sizeof(struct messagemanager_t));
	if (!framework->framework.messageManager)
		return false;
	messagemanager_init(framework->framework.messageManager);

	// Initialize the plugin manager
	WriteLog(LL_Debug, "initializing plugin manager");
	framework->framework.pluginManager = (struct pluginmanager_t*)kmalloc(sizeof(struct pluginmanager_t));
	if (!framework->framework.pluginManager)
		return false;
	pluginmanager_init(framework->framework.pluginManager);

	// Initialize the default plugins
	WriteLog(LL_Info, "installing default plugins");
	if (!mira_installDefaultPlugins(framework))
	{
		WriteLog(LL_Error, "could not initialize plugins");
		return false;
	}

	// Register our event handlers
	WriteLog(LL_Info, "installing event handlers");
	if (!miraframework_installHandlers(framework))
	{
		WriteLog(LL_Error, "could not install handlers");
		return false;
	}

	WriteLog(LL_Info, "installing hooks");
	miraframework_installHooks(framework);

	// Disable overlayfs while it's a broke sack of shit
	WriteLog(LL_Info, "allocating overlayfs");
	framework->overlayfs = (struct overlayfs_t*)kmalloc(sizeof(struct overlayfs_t));
	if (!framework->overlayfs)
	{
		WriteLog(LL_Error, "could not allocate overlayfs");
		return false;
	}
	overlayfs_init(framework->overlayfs);

	WriteLog(LL_Info, "miraframework initialized successfully");

	return true;
}

uint8_t miraframework_installHooks(struct miraframework_t* framework)
{
	if (!framework)
		return false;
	
	return true;
}

uint8_t miraframework_installHandlers(struct miraframework_t* framework)
{
	if (!framework)
		return false;

	/*eventhandler_tag
	(*eventhandler_register)(struct eventhandler_list *list, const char *name,
		void *func, void *arg, int priority) = kdlsym(eventhandler_register);*/

	// Register our event handlers
	//const int32_t prio = 1337;
	//EVENTHANDLER_REGISTER(system_suspend_phase3, mira_onSuspend, framework, EVENTHANDLER_PRI_LAST + prio);
	//EVENTHANDLER_REGISTER(system_resume_phase4, mira_onResume, framework, EVENTHANDLER_PRI_LAST + prio);
	//EVENTHANDLER_REGISTER(shutdown_pre_sync, mira_onShutdown, framework, EVENTHANDLER_PRI_LAST + prio);

	// Register our process event handlers
	//EVENTHANDLER_REGISTER(process_ctor, mira_onProcessCtor, framework, EVENTHANDLER_PRI_LAST + prio);
	//EVENTHANDLER_REGISTER(process_dtor, mira_onProcessDtor, framework, EVENTHANDLER_PRI_LAST + prio);

	return true;
}

uint8_t miraframework_loadSettings(struct miraframework_t* framework, const char* iniPath)
{
	if (!framework || !iniPath)
		return false;

	// TODO: Load the home directory
	framework->framework.homePath = "";

	// TODO: Load the configuration directory
	framework->framework.configPath = "";

	// TODO: Load the download path
	framework->framework.downloadPath = "";

	// TODO: Load the plugins path
	framework->framework.pluginsPath = "";

	return true;
}

//static void mira_onSuspend(struct miraframework_t* framework)
//{
//	if (!framework)
//		return;
//
//	// Notify the user that we are suspending
//	WriteLog(LL_Warn, "ON SUSPEND %p", framework);
//
//	// Stop the RPC server
//	WriteLog(LL_Info, "stopping RPC server.");
//	if (!rpcserver_shutdown(framework->framework.rpcServer))
//		WriteLog(LL_Error, "there was an error stopping the rpc server.");
//	
//	// Stop the klog server
//	WriteLog(LL_Info, "Shutting down plugin manager");
//	pluginmanager_shutdown(framework->framework.pluginManager);
//
//	WriteLog(LL_Info, "Disabling hooks");
//
//	WriteLog(LL_Info, "Everything *should* be stable m8");
//
//}
//
//static void mira_onResume(struct miraframework_t* framework)
//{
//	if (!framework)
//		return;
//
//	// TODO: Handle resuming
//	WriteLog(LL_Warn, "ON RESUME %p", framework);
//
//	// Initialize the default plugins
//	if (!mira_installDefaultPlugins(framework))
//		WriteLog(LL_Error, "could not initialize plugins");
//
//	WriteLog(LL_Info, "enabling hooks");
//}
//
//static void mira_onShutdown(struct miraframework_t* framework)
//{
//	if (!framework)
//		return;
//
//	// Shut down everything, we packin' our bags bois
//	WriteLog(LL_Warn, "ON SHUTDOWN %p", framework);
//}

uint8_t __noinline mira_installDefaultPlugins(struct miraframework_t* framework)
{
	// Initialize default plugins
	//
	//	Initialize the orbis utilities plugin
	//
	WriteLog(LL_Info, "allocating orbis utilities");
	if (framework->orbisUtilsPlugin)
	{
		kfree(framework->fileTransferPlugin, sizeof(*framework->fileTransferPlugin));
		framework->fileTransferPlugin = NULL;
	}
	struct orbisutils_plugin_t* orbisUtilsPlugin = (struct orbisutils_plugin_t*)kmalloc(sizeof(struct orbisutils_plugin_t));
	if (!orbisUtilsPlugin)
	{
		WriteLog(LL_Error, "error allocating orbis utils plugin");
		return false;
	}
	orbisutils_init(orbisUtilsPlugin);
	pluginmanager_registerPlugin(framework->framework.pluginManager, &orbisUtilsPlugin->plugin);

	// Register file transfer plugin
	WriteLog(LL_Info, "allocating file transfer plugin");
	if (framework->fileTransferPlugin)
	{
		kfree(framework->fileTransferPlugin, sizeof(*framework->fileTransferPlugin));
		framework->fileTransferPlugin = NULL;
	}
	
	framework->fileTransferPlugin = (struct filetransfer_plugin_t*)kmalloc(sizeof(struct filetransfer_plugin_t));
	if (!framework->fileTransferPlugin)
	{
		WriteLog(LL_Error, "error allocating file transfer plugin");
		return false;
	}
	filetransfer_plugin_init(framework->fileTransferPlugin);
	pluginmanager_registerPlugin(framework->framework.pluginManager, &framework->fileTransferPlugin->plugin);

	WriteLog(LL_Info, "allocating logserver");
	if (framework->logServerPlugin)
	{
		kfree(framework->logServerPlugin, sizeof(*framework->logServerPlugin));
		framework->logServerPlugin = NULL;
	}
		

	framework->logServerPlugin = (struct logserver_plugin_t*)kmalloc(sizeof(struct logserver_plugin_t));
	if (!framework->logServerPlugin)
	{
		WriteLog(LL_Error, "could not allocate log server.");
		return false;
	}
	logserver_init(framework->logServerPlugin);
	pluginmanager_registerPlugin(framework->framework.pluginManager, &framework->logServerPlugin->plugin);

	// Initialize the plugin loader to read from file
	WriteLog(LL_Info, "allocating pluginloader");
	if (framework->pluginLoader)
	{
		kfree(framework->pluginLoader, sizeof(*framework->pluginLoader));
		framework->pluginLoader = NULL;
	}

	framework->pluginLoader = (struct pluginloader_t*)kmalloc(sizeof(struct pluginloader_t));
	if (!framework->pluginLoader)
	{
		WriteLog(LL_Error, "error allocating plugin loader.");
		return false;
	}
	pluginloader_init(framework->pluginLoader);
	pluginloader_loadPlugins(framework->pluginLoader);

	// Debugger
	WriteLog(LL_Debug, "allocating debugger");

	if (framework->debuggerPlugin)
	{
		kfree(framework->debuggerPlugin, sizeof(*framework->debuggerPlugin));
		framework->debuggerPlugin = NULL;
	}
		
	framework->debuggerPlugin = (struct debugger_plugin_t*)kmalloc(sizeof(struct debugger_plugin_t));
	if (!framework->debuggerPlugin)
	{
		WriteLog(LL_Error, "could not allocate debugger plugin");
		return false;
	}
	debugger_plugin_init(framework->debuggerPlugin);
	pluginmanager_registerPlugin(framework->framework.pluginManager, &framework->debuggerPlugin->plugin);

	// Cheat plugin
	WriteLog(LL_Info, "allocating cheating plugin");

	framework->cheatPlugin = (struct cheat_plugin_t*)kmalloc(sizeof(struct cheat_plugin_t));
	if (!framework->cheatPlugin)
	{
		WriteLog(LL_Error, "could not allocate cheat plugin");
		return false;
	}
	cheat_plugin_init(framework->cheatPlugin);
	pluginmanager_registerPlugin(framework->framework.pluginManager, &framework->cheatPlugin->plugin);
	
	// Kick off the rpc server thread
	WriteLog(LL_Debug, "allocating rpc server");
	if (framework->framework.rpcServer)
	{
		kfree(framework->framework.rpcServer, sizeof(*framework->framework.rpcServer));
		framework->framework.rpcServer = NULL;
	}

	framework->framework.rpcServer = (struct rpcserver_t*)kmalloc(sizeof(struct rpcserver_t));
	if (!framework->framework.rpcServer)
	{
		WriteLog(LL_Error, "could not allocate rpc server.");
		return false;
	}
	rpcserver_init(framework->framework.rpcServer, curthread->td_proc);

	// Startup the server, it will kick off the thread
	WriteLog(LL_Info, "starting rpc server");
	if (!rpcserver_startup(framework->framework.rpcServer, ONI_RPC_PORT))
	{
		WriteLog(LL_Error, "rpcserver_startup failed");
		return false;
	}

	return true;
}

struct proc* mira_getProc()
{
	struct miraframework_t* fw = mira_getFramework();
	if (!fw)
		return NULL;

	struct initparams_t* params = fw->initParams;
	if (!params)
		return NULL;

	return params->process;
}

struct thread* mira_getMainThread()
{
	struct proc* process = mira_getProc();

	if (process->p_numthreads > 1)
		return process->p_threads.tqh_first;
	else if (process->p_numthreads == 1)
		return process->p_singlethread;
	else
		return NULL;
}