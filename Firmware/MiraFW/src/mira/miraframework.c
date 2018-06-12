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
#include <mira/plugins/orbisutils/orbisutils_plugin.h>

//
//	Utilities
//
#include <oni/utils/logger.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/kdlsym.h>

//
//	Free-BSD Specifics
//
#include <sys/eventhandler.h>
#include <sys/proc.h>					// proc

#define MIRA_CONFIG_PATH	"/user/mira.ini"

uint8_t __noinline mira_installDefaultPlugins();
uint8_t miraframework_installHandlers(struct miraframework_t* framework);
uint8_t miraframework_loadSettings(struct miraframework_t* framework, const char* iniPath);

//
//	Event hadling stuff
//

// Handle the kernel panics due to suspending
//static void mira_onSuspend(struct miraframework_t* framework); // uncommenting this code would make Rest Mode not work
//static void mira_onResume(struct miraframework_t* framework); // uncommenting this code would make Rest Mode not work
static void mira_onShutdown(struct miraframework_t* framework);

// Handle execution of new processes for trainers
static void mira_onExec(struct miraframework_t* framework);

typedef void(*callback_fn)(struct miraframework_t*);

//
//	Start code
//

struct miraframework_t* mira_getFramework()
{
	void * (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	WriteLog(LL_Info, "here");
	if (!gFramework)
	{
		WriteLog(LL_Info, "here");
		// We intentionally allocate the larger struct and cast to the basic
		struct miraframework_t* framework = (struct miraframework_t*)kmalloc(sizeof(struct miraframework_t));
		memset(framework, 0, sizeof(*framework));

		WriteLog(LL_Info, "here");
		if (!framework)
		{
			// Consider this a fatal error
			WriteLog(LL_Error, "could not allocate framework.");

			for (;;)
				__asm__("nop");

			return NULL;
		}

		WriteLog(LL_Info, "here");
		// Assign the global variable
		gFramework = (struct framework_t*)framework;
	}

	WriteLog(LL_Info, "here");

	return (struct miraframework_t*)gFramework;
}

uint8_t miraframework_initialize(struct miraframework_t* framework)
{
	if (!framework)
		return false;

	WriteLog(LL_Info, "here");

	// Load the settings from file if it exists
	if (miraframework_loadSettings(framework, MIRA_CONFIG_PATH))
		WriteLog(LL_Error, "mira configuration file not found, skipping...");

	// Initialize the message manager
	WriteLog(LL_Debug, "MessageManager initialization");
	framework->framework.messageManager = (struct messagemanager_t*)kmalloc(sizeof(struct messagemanager_t));
	if (!framework->framework.messageManager)
		return false;

	WriteLog(LL_Info, "here");

	messagemanager_init(framework->framework.messageManager);

	// Initialize the plugin manager
	WriteLog(LL_Debug, "[+] Initializing plugin manager");
	framework->framework.pluginManager = (struct pluginmanager_t*)kmalloc(sizeof(struct pluginmanager_t));
	if (!framework->framework.pluginManager)
		return false;

	WriteLog(LL_Info, "here");
	pluginmanager_init(framework->framework.pluginManager);

	WriteLog(LL_Info, "here");
	// Initialize the default plugins
	if (!mira_installDefaultPlugins(framework))
	{
		WriteLog(LL_Error, "could not initialize plugins");
		return false;
	}

	WriteLog(LL_Info, "here");

	// Register our event handlers
	if (!miraframework_installHandlers(framework))
	{
		WriteLog(LL_Error, "could not install handlers");
		return false;
	}

	return true;
}

uint8_t miraframework_installHandlers(struct miraframework_t* framework)
{
	eventhandler_tag
	(*eventhandler_register)(struct eventhandler_list *list, const char *name,
		void *func, void *arg, int priority) = kdlsym(eventhandler_register);

	WriteLog(LL_Info, "here");

	// Register our event handlers
	//EVENTHANDLER_REGISTER(system_suspend_phase1, mira_onSuspend, framework, EVENTHANDLER_PRI_FIRST); // uncommenting this code would make Rest Mode not work
	//EVENTHANDLER_REGISTER(system_resume_phase1, mira_onResume, framework, EVENTHANDLER_PRI_FIRST); // uncommenting this code would make Rest Mode not work
	EVENTHANDLER_REGISTER(shutdown_pre_sync, mira_onShutdown, framework, EVENTHANDLER_PRI_ANY);
	EVENTHANDLER_REGISTER(process_exec, mira_onExec, framework, EVENTHANDLER_PRI_LAST);

	WriteLog(LL_Info, "here");

	return true;
}

uint8_t miraframework_loadSettings(struct miraframework_t* framework, const char* iniPath)
{
	WriteLog(LL_Info, "here");
	if (!framework || !iniPath)
		return false;

	WriteLog(LL_Info, "here");

	// TODO: Load the home directory
	framework->framework.homePath = "";

	WriteLog(LL_Info, "here");
	// TODO: Load the configuration directory
	framework->framework.configPath = "";

	WriteLog(LL_Info, "here");
	// TODO: Load the download path
	framework->framework.downloadPath = "";

	WriteLog(LL_Info, "here");
	// TODO: Load the plugins path
	framework->framework.pluginsPath = "";

	WriteLog(LL_Info, "here");
	return true;
}

/*static void mira_onSuspend(struct miraframework_t* framework) // uncommenting this code would make Rest Mode not work
{
	if (!framework)
		return;

	// Notify the user that we are suspending
	WriteLog(LL_Warn, "ON SUSPEND %p", framework);

	// Stop the RPC server
	WriteLog(LL_Info, "stopping RPC server.");
	if (!rpcserver_shutdown(framework->framework.rpcServer))
		WriteLog(LL_Error, "there was an error stopping the rpc server.");
	
	// Stop the klog server
	WriteLog(LL_Info, "Shutting down plugin manager");
	pluginmanager_shutdown(framework->framework.pluginManager);

	WriteLog(LL_Info, "Everything *should* be stable m8");

}

static void mira_onResume(struct miraframework_t* framework)
{
	if (!framework)
		return;

	// TODO: Handle resuming
	WriteLog(LL_Warn, "ON RESUME %p", framework);

	// Initialize the default plugins
	if (!mira_installDefaultPlugins(framework))
		WriteLog(LL_Error, "could not initialize plugins");
}*/

static void mira_onShutdown(struct miraframework_t* framework)
{
	if (!framework)
		return;

	// Shut down everything, we packin' our bags bois
	WriteLog(LL_Warn, "ON SHUTDOWN %p", framework);
}

static void mira_onExec(struct miraframework_t* framework)
{
	if (!framework)
		return;

	// TOOD: Handle trainers
	WriteLog(LL_Warn, "ON EXEC %p", framework);
}

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
