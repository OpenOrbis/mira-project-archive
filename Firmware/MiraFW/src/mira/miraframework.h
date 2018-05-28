#pragma once
#include <oni/framework.h>

struct logserver_plugin_t;
struct filetransfer_plugin_t;
struct debugger_plugin_t;
struct pluginloader_t;
struct trainermanager_t;

struct miraframework_t
{
	struct framework_t framework;

	// Hold references to our internal plugins
	struct debugger_plugin_t* debuggerPlugin;
	struct logserver_plugin_t* logServerPlugin;
	struct filetransfer_plugin_t* fileTransferPlugin;

	struct pluginloader_t* pluginLoader;
	struct trainermanager_t* trainerManager;
};

struct miraframework_t* mira_getFramework();
