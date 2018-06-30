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
int overlayfs_onClose(struct thread* td, int fd);
int overlayfs_onRead(struct thread *td, int fd, struct uio *auio);
int overlayfs_onWrite(struct thread* td, struct write_args* uap);

#define kdlsym_addr_dmem_start_app_process 0x002468E0
#define kdlsym_addr_exec_new_vmspace 0x0038A940
#define kdlsym_addr_kern_open 0x0072AB50
#define kdlsym_addr_kern_openat 0x0033B640
#define kdlsym_addr_kern_readv 0x00152A10
#define kdlsym_addr_kern_close  0x000C0F40

void overlayfs_init(struct overlayfs_t* fs)
{
	if (!fs)
		return;

	void * (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	memset(fs->redirectPath, 0, sizeof(fs->redirectPath));

	// Set up all of the hooks
	fs->execNewVmspaceHook = hook_create(kdlsym(exec_new_vmspace), overlayfs_onExecNewVmspace);

	fs->openHook = hook_create(kdlsym(kern_openat), overlayfs_onOpenAt);
	fs->closeHook = hook_create(kdlsym(kern_close), overlayfs_onClose);
	fs->readHook = hook_create(kdlsym(kern_readv), overlayfs_onRead);
	fs->writeHook = NULL;
	//fs->writeHook = hook_create(kdlsym(kern_write), overlayfs_onWrite);

	fs->currentProc = NULL;

	hook_enable(fs->execNewVmspaceHook);
}

int overlayfs_onExecNewVmspace(struct image_params* imgp, struct sysentvec* sv)
{
	//
	//	This is purely for updating the current drive and mount point
	//

	// Get the framework
	struct overlayfs_t* fs = mira_getFramework()->overlayfs;

	int	(*exec_new_vmspace)(struct image_params *, struct sysentvec *) = hook_getFunctionAddress(fs->execNewVmspaceHook);
	
	// Call the original exec_new_vmspace
	hook_disable(fs->execNewVmspaceHook);

	int result = exec_new_vmspace(imgp, sv);

	hook_enable(fs->execNewVmspaceHook);

	// Find/assign the drive path if it hasn't already been found
	char* redirectPath = overlayfs_findDrivePath(fs);
	if (redirectPath == NULL)
	{
		WriteLog(LL_Info, "drive not found");
		hook_disable(fs->openHook);
		hook_disable(fs->closeHook);
		//hook_disable(fs->readHook);
		return result;
	}

	WriteLog(LL_Debug, "drive path found: %s", redirectPath);

	// Enable all of the hooks
	hook_enable(fs->openHook);
	hook_enable(fs->closeHook);
	hook_enable(fs->readHook);
	//hook_enable(fs->writeHook);

	return result;
}

int overlayfs_onOpenAt(struct thread *td, int fd, char *path, enum uio_seg pathseg, int flags, int mode)
{
	struct overlayfs_t* fs = mira_getFramework()->overlayfs;

	char* (*strstr)(const char*, const char*) = kdlsym(strstr);
	void * (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	int(*snprintf)(char *str, size_t size, const char *format, ...) = kdlsym(snprintf);
	int(*kern_openat)(struct thread *td, int fd, char *path, enum uio_seg pathseg, int flags, int mode) = hook_getFunctionAddress(fs->openHook);

	//
	//	Check the current process on open in order to update
	//
	if (strstr(td->td_proc->p_elfpath, "eboot") != NULL)
	{
		// Set our current proc
		fs->currentProc = td->td_proc;
	}

	// If overlayfs is not enabled, call the original function and return normally
	if (!overlayfs_isEnabled(fs) || // Check that the internal overlayfs is configured properly
		!overlayfs_isThreadInProc(fs, td) || // Check that the current accessing thread is in our tracked proc
		td->td_proc != fs->currentProc) // Check that the currentProc is the thread proc
	{
		hook_disable(fs->openHook);

		int result = kern_openat(td, fd, path, pathseg, flags, mode);

		hook_enable(fs->openHook);

		return result;
	}

	// First we need to get the new redirected path
	// This is assuming that the redirectPath in fs is /mnt/usb0 with no trailing slash
	// This is also assuming that path begins with /path so like /eboot.bin or /sce_module/blah.sprx
	// That way when combined it's /mnt/usb0/sce_module/blah.sprx
	// And we use the kernel privs in order to make the request, not the userland so it ignores sandbox
	char redirPath[OVERLAYFS_MAXPATH];
	memset(redirPath, 0, sizeof(redirPath));
	snprintf(redirPath, sizeof(redirPath), "%s%s", fs->redirectPath, path);

	WriteLog(LL_Debug, "checking for (%s)", redirPath);

	// Check to see if the file exists on USB
	if (!overlayfs_fileExists(fs, redirPath))
	{
		//WriteLog(LL_Error, "(%s)", redirPath);
		hook_disable(fs->openHook);

		int result = kern_openat(td, fd, path, pathseg, flags, mode);

		hook_enable(fs->openHook);

		return result;
	}

	WriteLog(LL_Debug, "attempting open to %s", redirPath);

	// Disable the hook to allow normal function
	hook_disable(fs->openHook);

	// Call open using the current thread, (not game thread)
	int result = kern_openat(mira_getMainThread(), fd, redirPath, pathseg, flags, mode); //kopen_t(redirPath, flags, mode, mira_getMainThread());

	WriteLog(LL_Warn, "overlayfs open %s returned %d fd: %d err: %d", redirPath, result, fd, td->td_retval[0]);

	// Re-enable the hook
	hook_enable(fs->openHook);
	
	// Return the descriptor or error untouched
	return result;
}

int overlayfs_onClose(struct thread* td, int fd)
{
	struct overlayfs_t* fs = mira_getFramework()->overlayfs;
	int(*kern_close)(struct thread* td, int fd) = hook_getFunctionAddress(fs->closeHook);

	// If overlayfs is not enabled, call the original function and return normally
	if (!overlayfs_isEnabled(fs) || // Check that the internal overlayfs is configured properly
		!overlayfs_isThreadInProc(fs, td) || // Check that the current accessing thread is in our tracked proc
		td->td_proc != fs->currentProc) // Check that the currentProc is the thread proc
	{
		hook_disable(fs->closeHook);

		int result = kern_close(td, fd);

		hook_enable(fs->closeHook);

		return result;
	}
	
	WriteLog(LL_Debug, "checking to close (%d)", fd);

	hook_disable(fs->closeHook);

	int result = kern_close(mira_getMainThread(), fd);

	hook_enable(fs->closeHook);

	return result;
}

int overlayfs_onRead(struct thread *td, int fd, struct uio *auio)
{
	struct overlayfs_t* fs = mira_getFramework()->overlayfs;
	int(*kern_readv)(struct thread* td, int fd, struct uio* auio) = hook_getFunctionAddress(fs->readHook);

	// If overlayfs is not enabled, call the original function and return normally
	if (!overlayfs_isEnabled(fs) || // Check that the internal overlayfs is configured properly
		!overlayfs_isThreadInProc(fs, td) || // Check that the current accessing thread is in our tracked proc
		td->td_proc != fs->currentProc) // Check that the currentProc is the thread proc
	{
		hook_disable(fs->readHook);

		int result = kern_readv(td, fd, auio);

		hook_enable(fs->readHook);

		return result;
	}

	WriteLog(LL_Debug, "checking to read (%d)", fd);

	hook_disable(fs->readHook);

	int result = kern_readv(mira_getMainThread(), fd, auio);

	hook_enable(fs->readHook);

	return result;
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
	// We cannot printf in here, or it crashes.
	if (!fs)
		return NULL;

	void * (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	int(*snprintf)(char *str, size_t size, const char *format, ...) = kdlsym(snprintf);

	const uint32_t cMaxUsbDrives = 10;
	char fileNameBuffer[64];

	// To keep this robust, we will automatically iterate over all usb drives
	for (uint32_t i = 0; i < cMaxUsbDrives; ++i)
	{
		// Zero out the previous buffer
		memset(fileNameBuffer, 0, sizeof(fileNameBuffer));

		// sprintf the name
		snprintf(fileNameBuffer, sizeof(fileNameBuffer), "/mnt/usb%d/_overlayfs", i);

		// If this file does not exist, we skip on to the next usb device
		if (!overlayfs_fileExists(fs, fileNameBuffer))
			continue;

		// Zero out the redirect path
		memset(fs->redirectPath, 0, sizeof(fs->redirectPath));
		snprintf(fs->redirectPath, sizeof(fs->redirectPath), "/mnt/usb%d", i);

		return fs->redirectPath;
	}

	memset(fs->redirectPath, 0, sizeof(fs->redirectPath));
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