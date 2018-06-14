#pragma once
#include <oni/framework.h>

struct debugger_plugin_t;
struct filetransfer_plugin_t;
struct logserver_plugin_t;
struct orbisutils_plugin_t;
struct pluginloader_t;
struct trainermanager_t;
struct hook_t;
struct thread;

struct miraframework_t
{
	struct framework_t framework;

	// Hold references to our internal plugins
	struct debugger_plugin_t* debuggerPlugin;
	struct logserver_plugin_t* logServerPlugin;
	struct filetransfer_plugin_t* fileTransferPlugin;
	struct orbisutils_plugin_t* orbisUtilsPlugin;

	struct pluginloader_t* pluginLoader;
	struct trainermanager_t* trainerManager;
	
};

struct miraframework_t* mira_getFramework();
uint8_t miraframework_initialize(struct miraframework_t* framework);
