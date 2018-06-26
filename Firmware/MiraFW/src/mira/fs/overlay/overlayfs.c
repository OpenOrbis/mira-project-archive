#include "overlayfs.h"
#include <sys/eventhandler.h>
#include <sys/fcntl.h>
#include <sys/sysproto.h>
#include <sys/uio.h>
#include <sys/proc.h>
#include <sys/imgact.h>

#include <oni/utils/logger.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/hook.h>
#include <oni/init/initparams.h>

#include <mira/miraframework.h>

void overlayfs_onProcessCtor(struct overlayfs_t* fs);
void overlayfs_onProcessDtor(struct overlayfs_t* fs);

struct dmem_start_app_process_args
{
	char unknown0[0xB0];
	int32_t pid;
};
int overlayfs_onExecNewVmspace(struct image_params* imgp, struct sysentvec* sv);
void overlayfs_onDmemStartAppProcess(struct dmem_start_app_process_args* args);
int overlayfs_onOpenAt(struct thread *td, int fd, char *path, enum uio_seg pathseg, int flags, int mode);
int overlayfs_onClose(struct thread* td, struct close_args* uap);
int overlayfs_onRead(struct thread* td, struct read_args* uap);
int overlayfs_onWrite(struct thread* td, struct write_args* uap);

#define kdlsym_addr_dmem_start_app_process 0x002468E0
#define kdlsym_addr_exec_new_vmspace 0x0038A940
#define kdlsym_addr_kern_open 0x0072AB50
#define kdlsym_addr_kern_openat 0x0033B640

void overlayfs_init(struct overlayfs_t* fs)
{
	if (!fs)
		return;

	// Set up all of the hooks
	fs->dmemStartAppProcessHook = hook_create(kdlsym(exec_new_vmspace), overlayfs_onExecNewVmspace);
	hook_enable(fs->dmemStartAppProcessHook);

	fs->openHook = hook_create(kdlsym(kern_openat), overlayfs_onOpenAt);
	fs->closeHook = hook_create(kdlsym(sys_close), overlayfs_onClose);
	fs->readHook = hook_create(kdlsym(sys_read), overlayfs_onRead);
	fs->writeHook = hook_create(kdlsym(sys_write), overlayfs_onWrite);

	fs->currentProc = NULL;

}

int overlayfs_onExecNewVmspace(struct image_params* imgp, struct sysentvec* sv)
{
	struct overlayfs_t* fs = mira_getFramework()->overlayfs;
	WriteLog(LL_Debug, "overlayfs: %p", fs);

	int(*exec_new_vmspace)(struct image_params* imgp, struct sysentvec* sv) = hook_getFunctionAddress(fs->dmemStartAppProcessHook);

	hook_disable(fs->dmemStartAppProcessHook);

	WriteLog(LL_Debug, "calling original exec_new_vmspace %p %p", imgp, sv);

	int result = exec_new_vmspace(imgp, sv);

	hook_enable(fs->dmemStartAppProcessHook);

	// Find/assign the drive path if it hasn't already been found
	char* redirectPath = overlayfs_findDrivePath(fs);
	if (redirectPath == NULL)
	{
		WriteLog(LL_Info, "drive not found");
		return result;
	}

	hook_enable(fs->openHook);

	return result;
}

void overlayfs_onDmemStartAppProcess(struct dmem_start_app_process_args* args)
{
	int32_t pid = args->pid;

	WriteLog(LL_Info, "dmem_start_app_process called pid: %d", pid);

	void(*_mtx_unlock_sleep)(struct mtx *m, int opts, const char *file, int line) = kdlsym(mtx_unlock_sleep);
	
	struct overlayfs_t* fs = mira_getFramework()->overlayfs;
	WriteLog(LL_Debug, "overlayfs: %p", fs);

	// Find the proc structure via pid
	struct  proc* (*pfind)(pid_t) = kdlsym(pfind);

	WriteLog(LL_Debug, "HOE ASS MUFUCKA");

	// This will either be NULL or contain our locked proc
	fs->currentProc = pfind(pid);
	WriteLog(LL_Debug, "pid %d proc %p", pid, fs->currentProc);

	// Unlock the proc
	PROC_UNLOCK(fs->currentProc);

	void(*dmem_start_app_process)(void* args) = hook_getFunctionAddress(fs->dmemStartAppProcessHook);

	hook_disable(fs->dmemStartAppProcessHook);

	WriteLog(LL_Debug, "calling original dmem_start_app_process %p", args);

	dmem_start_app_process(args);

	hook_enable(fs->dmemStartAppProcessHook);

	// Find/assign the drive path if it hasn't already been found
	char* redirectPath = overlayfs_findDrivePath(fs);
	if (redirectPath == NULL)
	{
		WriteLog(LL_Info, "drive not found");
		return;
	}

	uint8_t enabled = overlayfs_isEnabled(fs);
	if (enabled)
	{
		hook_enable(fs->openHook);
	}
	WriteLog(LL_Info, "isEnabled: %s", enabled ? "true" : "false");
}

int overlayfs_onOpenAt(struct thread *td, int fd, char *path, enum uio_seg pathseg, int flags, int mode)
{
	//WriteLog(LL_Debug, "overlayfs openhook: %p %d %s %x %d %d", td, fd, path, pathseg, flags, mode);
	struct overlayfs_t* fs = mira_getFramework()->overlayfs;

	// 5.01 - TODO: If this makes it move it
#define kdlsym_addr_strstr 0x0017DEA0
	char* (*strstr)(const char*, const char*) = kdlsym(strstr);

	WriteLog(LL_Info, "elfpath: %s", td->td_proc->p_elfpath);
	if (strstr(td->td_proc->p_elfpath, "eboot") == NULL)
	{
		fs->currentProc = td->td_proc;

		// If the path is not set attempt to set it
		if (fs->redirectPath[0] == '\0')
		{
			overlayfs_findDrivePath(fs);
			/*char* redirectPath = overlayfs_findDrivePath(fs);
			if (redirectPath == NULL)
				WriteLog(LL_Info, "drive not found");*/
		}
	}

	//WriteLog(LL_Debug, "fs: %p", fs);

	// If overlayfs is not enabled, call the original function and return normally
	if (!overlayfs_isEnabled(fs) || !overlayfs_isThreadInProc(fs, td))
	{
		//WriteLog(LL_Debug, "overlayfs disabled, calling original kern_open");
		hook_disable(fs->openHook);

		int(*kern_openat)(struct thread *td, int fd, char *path, enum uio_seg pathseg, int flags, int mode) = hook_getFunctionAddress(fs->openHook);

		int result = kern_openat(td, fd, path, pathseg, flags, mode);

		hook_enable(fs->openHook);

		return result;
	}

	void * (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	int(*snprintf)(char *str, size_t size, const char *format, ...) = kdlsym(snprintf);

	//WriteLog(LL_Debug, "open overlayfs %s", path);

	// First we need to get the new redirected path
	// This is assuming that the redirectPath in fs is /mnt/usb0 with no trailing slash
	// This is also assuming that path begins with /path so like /eboot.bin or /sce_module/blah.sprx
	// That way when combined it's /mnt/usb0/sce_module/blah.sprx
	// And we use the kernel privs in order to make the request, not the userland so it ignores sandbox
	char redirPath[OVERLAYFS_MAXPATH];
	memset(redirPath, 0, sizeof(redirPath));
	snprintf(redirPath, sizeof(redirPath), "%s%s", fs->redirectPath, path);

	if (!overlayfs_fileExists(fs, redirPath))
	{
		WriteLog(LL_Debug, "file on usb does not exist, calling original");
		hook_disable(fs->openHook);

		int(*kern_openat)(struct thread *td, int fd, char *path, enum uio_seg pathseg, int flags, int mode) = hook_getFunctionAddress(fs->openHook);

		int result = kern_openat(td, fd, path, pathseg, flags, mode);

		hook_enable(fs->openHook);

		return result;
	}

	WriteLog(LL_Debug, "attempting open to %s", redirPath);

	// Disable the hook to allow normal function
	hook_disable(fs->openHook);

	// Call open using the current thread, (not game thread)
	int result = kopen_t(redirPath, flags, mode, mira_getMainThread());

	WriteLog(LL_Debug, "overlayfs open %s returned %d", redirPath, fd);

	// Re-enable the hook
	hook_enable(fs->openHook);
	
	// Return the descriptor or error untouched
	return result;
}

int overlayfs_onClose(struct thread* td, struct close_args* uap)
{
	return -1;
}

int overlayfs_onRead(struct thread* td, struct read_args* uap)
{
	return -1;
}

int overlayfs_onWrite(struct thread* td, struct write_args* uap)
{
	return -1;
}

uint8_t overlayfs_isEnabled(struct overlayfs_t* fs)
{
	if (!fs)
		return false;

	if (fs->redirectPath[0] == '\0')
		return false;

	if (fs->currentProc == NULL)
		return false;

	return true;
}

char* overlayfs_findDrivePath(struct overlayfs_t* fs)
{
	void * (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	int(*snprintf)(char *str, size_t size, const char *format, ...) = kdlsym(snprintf);

	const uint32_t cMaxUsbDrives = 10;
	char fileNameBuffer[64];

	//WriteLog(LL_Debug, "iterating all usb drives");

	// To keep this robust, we will automatically iterate over all usb drives
	for (uint32_t i = 0; i < cMaxUsbDrives; ++i)
	{
		// Zero out the previous buffer
		memset(fileNameBuffer, 0, sizeof(fileNameBuffer));

		// sprintf the name
		snprintf(fileNameBuffer, sizeof(fileNameBuffer), "/mnt/usb%d/_overlayfs", i);

		//WriteLog(LL_Debug, "attempting to open %s", fileNameBuffer);

		// If this file does not exist, we skip on to the next usb device
		if (!overlayfs_fileExists(fs, fileNameBuffer))
			continue;

		// Zero out the redirect path
		memset(fs->redirectPath, 0, sizeof(fs->redirectPath));
		snprintf(fs->redirectPath, sizeof(fs->redirectPath), "/mnt/usb%d", i);

		//WriteLog(LL_Debug, "set usb path to %s", fs->redirectPath);

		return fs->redirectPath;
	}

	//WriteLog(LL_Debug, "cannot find drive path");

	return NULL;
}

uint8_t overlayfs_fileExists(struct overlayfs_t* fs, char* path)
{
	// Verify arguments
	if (!fs || !path)
		return false;

	struct thread* td = mira_getMainThread();
	if (!td)
		return false;

	// Attempt to open the file for read only
	int fd = kopen_t(path, O_RDONLY, 0, td);
	if (fd < 0)
		return false;

	// Close the handle
	kclose_t(fd, td);

	return true;
}

uint8_t overlayfs_isThreadInProc(struct overlayfs_t* fs, struct thread* td)
{
	if (!fs || !td || !fs->currentProc)
		return false;

	//void(*_mtx_unlock_sleep)(struct mtx *m, int opts, const char *file, int line) = kdlsym(mtx_unlock_sleep);
	//void(*_mtx_lock_sleep)(struct mtx *m, uintptr_t tid, int opts, const char *file, int line) = kdlsym(mtx_lock_sleep);
	uint8_t isInside = false;

	//WriteLog(LL_Debug, "checking if thread is in proc fs: %p thread: %p proc: %p", fs, td, fs->currentProc);

	//PROC_LOCK(fs->currentProc);

	//WriteLog(LL_Debug, "proc locked");

	struct thread* t = NULL;
	FOREACH_THREAD_IN_PROC(fs->currentProc, t)
	{
		if (t != td)
			continue;

		isInside = true;
		break;
	}
	//PROC_UNLOCK(fs->currentProc);

	return isInside;
}