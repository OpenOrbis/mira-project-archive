#include "appinstaller_plugin.h"
#include <oni/utils/dynlib.h>
#include <oni/utils/logger.h>
#include <oni/utils/escape.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/kdlsym.h>

#include <sys/proc.h>
#include <sys/fcntl.h>
#include <sys/stat.h>

void appinstallerplugin_init(struct appinstallerplugin_t* plugin)
{
	if (!plugin)
		return;

	plugin->plugin.name = "appinstaller";
	plugin->plugin.description = "Installs homebrew pkg files from rpc";

	plugin->plugin.plugin_load = (uint8_t(*)(void*)) appinstaller_load;
	plugin->plugin.plugin_unload = (uint8_t(*)(void*)) appinstaller_unload;
}

uint8_t appinstaller_load(struct appinstallerplugin_t * plugin)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	if (!plugin)
		return false;

	
	// Escape this thread we are on
	struct thread_info_t threadInfo;
	memset(&threadInfo, 0, sizeof(threadInfo));

	oni_threadEscape(curthread, &threadInfo);

	// Change our auth id to ShellCore
	curthread->td_ucred->cr_sceAuthID = 0x3800000000000010;
	
	int32_t appInstUtilModule = -1;

	int64_t ret = sys_dynlib_load_prx("/system/common/lib/libSceAppInstUtil.sprx", &appInstUtilModule);
	if (ret)
	{
		WriteLog(LL_Error, "unable to load libSceAppInstUtil.sprx");
		goto err;
	}

	// Resolve the functions that we need
	ret = sys_dynlib_dlsym(appInstUtilModule, "sceAppInstUtilInitialize", &plugin->sceAppInstUtilInitialize);
	if (ret)
	{
		WriteLog(LL_Error, "could not resolve sceAppInstUtilInitialize");
		goto err;
	}

	ret = sys_dynlib_dlsym(appInstUtilModule, "sceAppInstUtilAppInstallPkg", &plugin->sceAppInstUtilAppInstallPkg);
	if (ret)
	{
		WriteLog(LL_Error, "could not resolve sceAppInstUtilAppInstallPkg");
		goto err;
	}

	ret = sys_dynlib_dlsym(appInstUtilModule, "sceAppInstUtilGetTitleIdFromPkg", &plugin->sceAppInstUtilGetTitleIdFromPkg);
	if (ret)
	{
		WriteLog(LL_Error, "could not resolve sceAppInstUtilGetTitleIdFromPkg");
		goto err;
	}

	ret = sys_dynlib_dlsym(appInstUtilModule, "sceAppInstUtilAppPrepareOverwritePkg", &plugin->sceAppInstUtilAppPrepareOverwritePkg);
	if (ret)
	{
		WriteLog(LL_Error, "could not resolve sceAppInstUtilAppPrepareOverwritePkg");
		goto err;
	}

	ret = sys_dynlib_dlsym(appInstUtilModule, "sceAppInstUtilGetPrimaryAppSlot", &plugin->sceAppInstUtilGetPrimaryAppSlot);
	if (ret)
	{
		WriteLog(LL_Error, "could not resolve sceAppInstUtilGetPrimaryAppSlot");
		goto err;
	}

	// TODO: Finish the debugger plugin
	// TODO: Finish this shit
	// TOOD: Before finishing this shit finally decide on how you want to 
	// deal with RPC requests and responses

	
	return true;

err:
	return false;
}

uint8_t appinstaller_unload(struct appinstallerplugin_t * plugin)
{
	if (!plugin)
		return false;



	return true;
}

void appinstaller_installPkg(struct ref_t* reference)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	struct thread_info_t threadInfo;
	memset(&threadInfo, 0, sizeof(threadInfo));

	// Escape the current thread
	oni_threadEscape(curthread, &threadInfo);

	// Set our credential to be SceShellCore
	curthread->td_ucred->cr_sceAuthID = 0x3800000000000010;

	int32_t ret = 0;
	int32_t fd = -1;

	// Open our new file for writing
	ret = fd = kopen("/user/app/TITLEID/app.pkg", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG);
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not open pkg file for writing (%d).", ret);
		return;
	}

	// Truncate the file to 0
	ret = kftruncate(fd, 0);
	if (ret)
	{
		WriteLog(LL_Error, "could not truncate file (%d).", ret);
		return;
	}

	WriteLog(LL_Info, "preallocating file sectors...");

	uint32_t status = 0;
	ret = gsched_set_slot_prio(fd, 1, 7, &status);
	if (ret)
	{
		WriteLog(LL_Error, "gsched_set_slot_prio failed (%d).", ret);
		return;
	}

	// Allocate the total block size
	ret = ffs_allocblocks(fd, 0x1000/*FinalSize*/, 0x80, 0);
	if (ret)
	{
		WriteLog(LL_Error, "ffs_allocblocks failed (%d).", ret);
	}

	// TODO: Copy the data over


	// Finally close the file descriptor
	kclose(fd);
}