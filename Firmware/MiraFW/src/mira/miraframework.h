#pragma once
#include <oni/framework.h>

struct cheat_plugin_t;
struct debugger_plugin_t;
struct filetransfer_plugin_t;
struct logserver_plugin_t;
struct orbisutils_plugin_t;
struct pluginloader_t;
struct trainermanager_t;
struct hook_t;
struct thread;
struct initparams_t;
struct consoleplugin_t;
struct overlayfs_t;

struct miraframework_t
{
	struct framework_t framework;

	// Hold references to our internal plugins
	struct debugger_plugin_t* debuggerPlugin;
	struct logserver_plugin_t* logServerPlugin;
	struct filetransfer_plugin_t* fileTransferPlugin;
	struct orbisutils_plugin_t* orbisUtilsPlugin;
	struct cheat_plugin_t* cheatPlugin;

	struct consoleplugin_t* consolePlugin;

	struct pluginloader_t* pluginLoader;
	struct trainermanager_t* trainerManager;

	struct overlayfs_t* overlayfs;

	struct initparams_t* initParams;
};

struct miraframework_t* mira_getFramework();
struct proc* mira_getProc();
struct thread* mira_getMainThread();

uint8_t miraframework_initialize(struct miraframework_t* framework);
