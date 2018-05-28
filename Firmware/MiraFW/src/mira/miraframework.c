#include "miraframework.h"


//
//	Oni-Framework core
//
#include <oni/messaging/messagemanager.h>
#include <oni/rpc/rpcserver.h>

//
//	Built-in plugins
//
#include <oni/plugins/pluginmanager.h>
#include <mira/plugins/filetransfer/filetransfer_plugin.h>
#include <mira/plugins/logserver/logserver_plugin.h>
#include <mira/plugins/debugger/debugger_plugin.h>
#include <mira/plugins/pluginloader.h>	// Load plugins from file

//
//	Utilities
//
#include <oni/utils/logger.h>

//
//	Free-BSD Specifics
//
#include <sys/eventhandler.h>
#include <sys/proc.h>					// proc

#define MIRA_CONFIG_PATH	"/user/mira.ini"

uint8_t miraframework_initalize(struct miraframework_t* framework);
uint8_t miraframework_loadSettings(struct miraframework_t* framework, const char* iniPath);

//
//	Event hadling stuff
//

// Handle the kernel panics due to suspending
static void mira_onSuspend(void* a __unused, u_int cmd, int* error);
static void mira_onResume(void* a __unused, u_int cmd, int* error);
static void mira_onShutdown(void* a __unused, u_int cmd, int* error);

// Handle execution of new processes for trainers
static void mira_onExec(void* a __unused, u_int cmd, int* error);

typedef void(*callback_fn)(void*, u_int, int*);
EVENTHANDLER_DECLARE(onSuspend, callback_fn);
EVENTHANDLER_DECLARE(onResume, callback_fn);
EVENTHANDLER_DECLARE(onShutdown, callback_fn);
EVENTHANDLER_DECLARE(onExec, callback_fn);

//
//	Start code
//

struct miraframework_t* mira_getFramework()
{
	if (!gFramework)
	{
		// We intentionally allocate the larger struct and cast to the basic
		gFramework = (struct framework_t*)kmalloc(sizeof(struct miraframework_t));
		if (!gFramework)
		{
			// Consider this a fatal error
			for (;;)
				WriteLog(LL_Error, "could not allocate framework.");

			return NULL;
		}

		// Initialize the original framework + extensions
		miraframework_initalize(gFramework);
	}

	return (struct miraframework_t*)gFramework;
}

uint8_t miraframework_initalize(struct miraframework_t* framework)
{
	if (!framework)
		return false;

	// Load the settings from file if it exists
	if (miraframework_loadSettings(framework, MIRA_CONFIG_PATH))
		WriteLog(LL_Error, "mira configuration file not found, skipping...");

	// Initialize the message manager
	WriteLog(LL_Debug, "MessageManager initialization");
	gFramework->messageManager = (struct messagemanager_t*)kmalloc(sizeof(struct messagemanager_t));
	if (!gFramework->messageManager)
		return false;

	messagemanager_init(gFramework->messageManager);

	// Initialize the plugin manager
	WriteLog(LL_Debug, "[+] Initializing plugin manager");
	gFramework->pluginManager = (struct pluginmanager_t*)kmalloc(sizeof(struct pluginmanager_t));
	if (!gFramework->pluginManager)
		return false;

	pluginmanager_init(gFramework->pluginManager);

	// Initialize the default plugins
	if (!mira_installDefaultPlugins())
	{
		WriteLog(LL_Error, "could not initialize plugins");
		return false;
	}

	// Register our event handlers
	if (!miraframework_installHandlers((struct miraframework_t*)gFramework))
	{
		WriteLog(LL_Error, "could not install handlers");
		return false;
	}

	return true;
}

uint8_t miraframework_installHandlers(struct miraframework_t* framework)
{
	// Register our event handlers
	EVENTHANDLER_REGISTER(system_suspend_phase1, mira_onSuspend, NULL, EVENTHANDLER_PRI_FIRST);
	EVENTHANDLER_REGISTER(system_resume_phase1, mira_onResume, NULL, EVENTHANDLER_PRI_FIRST);
	EVENTHANDLER_REGISTER(shutdown_pre_sync, mira_onShutdown, NULL, EVENTHANDLER_PRI_FIRST);
	EVENTHANDLER_REGISTER(process_exec, mira_onExec, NULL, EVENTHANDLER_PRI_FIRST);

	return true;
}

uint8_t miraframework_loadSettings(struct miraframework_t* framework, const char* iniPath)
{
	if (!framework || !iniPath)
		return false;

	// TODO: Load the home directory
	gFramework->homePath = "";

	// TODO: Load the configuration directory
	gFramework->configPath = "";

	// TODO: Load the download path
	gFramework->downloadPath = "";

	// TODO: Load the plugins path
	gFramework->pluginsPath = "";

	return true;
}

static void mira_onSuspend(void* a __unused, u_int cmd, int* error)
{
	if (!gFramework)
		return;

	// Notify the user that we are suspending
	WriteLog(LL_Warn, "ON SUSPEND");

	// Stop the RPC server
	WriteLog(LL_Info, "stopping RPC server.");
	if (!rpcserver_shutdown(gFramework->rpcServer))
		WriteLog(LL_Error, "there was an error stopping the rpc server.");
	
	// Stop the klog server
	WriteLog(LL_Info, "Shutting down plugin manager");
	pluginmanager_shutdown(gFramework->pluginManager);

	WriteLog(LL_Info, "Everything *should* be stable m8");
}

static void mira_onResume(void* a __unused, u_int cmd, int* error)
{
	// TODO: Handle resuming
	WriteLog(LL_Warn, "ON RESUME");
	// Initialize the default plugins
	if (!mira_installDefaultPlugins())
	{
		WriteLog(LL_Error, "could not initialize plugins");
		return false;
	}


}
static void mira_onShutdown(void* a __unused, u_int cmd, int* error)
{
	// Shut down everything, we packin' our bags bois
	WriteLog(LL_Warn, "ON SHUTDOWN");
}

static void mira_onExec(void* a __unused, u_int cmd, int* error)
{
	// TOOD: Handle trainers
	WriteLog(LL_Warn, "ON EXEC");
}

uint8_t __noinline mira_installDefaultPlugins()
{
	struct miraframework_t* framework = (struct miraframework_t*)gFramework;
	// Initialize default plugins

	// Register file transfer plugin
	framework->fileTransferPlugin = (struct filetransfer_plugin_t*)kmalloc(sizeof(struct filetransfer_plugin_t));
	if (!framework->fileTransferPlugin)
	{
		WriteLog(LL_Error, "error allocating file transfer plugin");
		return false;
	}
	filetransfer_plugin_init(framework->fileTransferPlugin);
	pluginmanager_registerPlugin(gFramework->pluginManager, &framework->fileTransferPlugin->plugin);

	WriteLog(LL_Info, "allocating logserver");
	framework->logServerPlugin = (struct logserver_plugin_t*)kmalloc(sizeof(struct logserver_plugin_t));
	if (!framework->logServerPlugin)
	{
		WriteLog(LL_Error, "could not allocate log server.");
		return false;
	}
	logserver_init(framework->logServerPlugin);
	pluginmanager_registerPlugin(gFramework->pluginManager, &framework->logServerPlugin->plugin);

	// Initialize the plugin loader to read from file
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

	framework->debuggerPlugin = (struct debugger_plugin_t*)kmalloc(sizeof(struct debugger_plugin_t));
	if (!((struct miraframework_t*)gFramework)->debuggerPlugin)
	{
		WriteLog(LL_Error, "could not allocate debugger plugin");
		return false;
	}
	debugger_plugin_init(framework->debuggerPlugin);
	pluginmanager_registerPlugin(gFramework->pluginManager, &framework->debuggerPlugin->plugin);

	// Kick off the rpc server thread
	WriteLog(LL_Debug, "allocating rpc server");
	gFramework->rpcServer = (struct rpcserver_t*)kmalloc(sizeof(struct rpcserver_t));
	if (!gFramework->rpcServer)
	{
		WriteLog(LL_Error, "could not allocate rpc server.");
		return false;
	}
	rpcserver_init(gFramework->rpcServer, curthread->td_proc);

	// Startup the server, it will kick off the thread
	WriteLog(LL_Info, "starting rpc server");
	if (!rpcserver_startup(gFramework->rpcServer, ONI_RPC_PORT))
	{
		WriteLog(LL_Error, "rpcserver_startup failed");
		return false;
	}

	return true;
}