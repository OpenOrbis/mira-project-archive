#include "overlayfs.h"
#include <sys/eventhandler.h>
#include <sys/fcntl.h>
#include <sys/sysproto.h>
#include <sys/uio.h>
#include <sys/proc.h>
#include <sys/imgact.h>
#include <sys/mount.h>
#include <sys/sysent.h>

#include <oni/utils/logger.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/hook.h>
#include <oni/init/initparams.h>
#include <oni/utils/sys_wrappers.h>

#include <mira/miraframework.h>


#include <oni/utils/cpu.h>

int overlayfs_sys_open(struct thread* td, struct open_args* uap);
static void
build_iovec(struct iovec **iov, int *iovlen, const char *name, void *val,
	size_t len);

// Taken directly from sbin/mount/getmntopts.c
static void
build_iovec(struct iovec **iov, int *iovlen, const char *name, void *val,
	size_t len)
{
	size_t (*strlen)(const char *str) = kdlsym(strlen);
	char * (*strdup)(const char *string, struct malloc_type* type) = kdlsym(strdup);
	void* (*realloc)(void *addr, unsigned long size, struct malloc_type *mtp, int flags) = kdlsym(realloc);
	struct malloc_type* M_MOUNT = kdlsym(M_MOUNT);

	int i;

	if (*iovlen < 0)
		return;

	i = *iovlen;

	*iov = realloc(*iov, sizeof **iov * (i + 2), M_MOUNT, 0);

	if (*iov == NULL) {
		*iovlen = -1;
		return;
	}

	(*iov)[i].iov_base = strdup(name, M_MOUNT);
	(*iov)[i].iov_len = strlen(name) + 1;
	i++;
	(*iov)[i].iov_base = val;

	if (len == (size_t)-1) {
		if (val != NULL)
			len = strlen(val) + 1;
		else
			len = 0;
	}

	(*iov)[i].iov_len = (int)len;
	*iovlen = ++i;
}


static int mount_fs(struct thread* td, char* device, char* mountpoint, char* fstype, char* mode, unsigned int flags)
{
	struct iovec* iov = NULL;
	int iovlen = 0;

	WriteLog(LL_Debug, "here");

	build_iovec(&iov, &iovlen, "fstype", fstype, -1);
	build_iovec(&iov, &iovlen, "fspath", mountpoint, -1);
	build_iovec(&iov, &iovlen, "from", device, -1);
	build_iovec(&iov, &iovlen, "large", "yes", -1);
	build_iovec(&iov, &iovlen, "timezone", "static", -1);
	build_iovec(&iov, &iovlen, "async", "", -1);
	build_iovec(&iov, &iovlen, "ignoreacl", "", -1);

	if (mode) 
	{
		build_iovec(&iov, &iovlen, "dirmask", mode, -1);
		build_iovec(&iov, &iovlen, "mask", mode, -1);
	}

	int(*nmount)(struct thread* thread, struct nmount_args*) = kdlsym(sys_nmount);
	if (!nmount)
		return -1;

	int error;
	struct nmount_args uap;

	// clear errors
	td->td_retval[0] = 0;

	// call syscall
	uap.iovp = iov;
	uap.iovcnt = iovlen;
	uap.flags = flags;

	error = nmount(td, &uap);
	if (error)
		return -error;

	// return socket
	return td->td_retval[0];
}


int overlayfs_onExecNewVmspace(struct image_params* imgp, struct sysentvec* sv);

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

	// Install the kernel process execution
	fs->execNewVmspaceHook = hook_create(kdlsym(exec_new_vmspace), overlayfs_onExecNewVmspace);
	hook_enable(fs->execNewVmspaceHook);

	// We are currently ignoring all proc's
	fs->pid = -1;
}

int(*_sys_open)(struct thread* td, struct open_args* uap) = NULL;

int overlayfs_onExecNewVmspace(struct image_params* imgp, struct sysentvec* sv)
{
	char* (*strstr)(const char*, const char*) = kdlsym(strstr);
	void(*critical_enter)(void) = kdlsym(critical_enter);
	void(*critical_exit)(void) = kdlsym(critical_exit);

	//
	//	This is purely for updating the current drive and mount point
	//
	// Get the framework
	struct overlayfs_t* fs = mira_getFramework()->overlayfs;

	int	(*exec_new_vmspace)(struct image_params *, struct sysentvec *) = hook_getFunctionAddress(fs->execNewVmspaceHook);
	
	// Call the original exec_new_vmspace
	hook_disable(fs->execNewVmspaceHook);

	int32_t result = exec_new_vmspace(imgp, sv);

	hook_enable(fs->execNewVmspaceHook);

	// We only want to catch eboots
	if (strstr(imgp->execpath, "eboot") == NULL)
	{
		WriteLog(LL_Debug, "%s not eboot", imgp->execpath);
		return result;
	}

	struct thread* td = imgp->proc->p_singlethread ? imgp->proc->p_singlethread : imgp->proc->p_threads.tqh_first;
	int32_t pid = td->td_proc->p_pid;

	WriteLog(LL_Debug, "Process Thread td %p pid %d", td, pid);

	// This may get called for each thread created, so we skip if it's the same pid
	if (fs->pid == pid)
		return result;

	// Update the pid
	fs->pid = pid;

	// Create a new directory to mount this path at
	int32_t mkdirResult = kmkdir_t("/mnt/sandbox/CUSA08034_000/usb0", 0777, mira_getMainThread());
	WriteLog(LL_Debug, "mkdir returned %d", mkdirResult);
	if (mkdirResult < 0)
	{
		if (mkdirResult != -EEXIST)
		{
			WriteLog(LL_Error, "could not create usb0 folder (%d)", mkdirResult);
			return result;
		}
	}

	// Mount the usb device to the newly created path, exfatfs, force mount
	int mountResult = mount_fs(td, "/dev/da1s1", "/mnt/sandbox/CUSA08034_000/usb0", "exfatfs", "511", MNT_FORCE);
	WriteLog(LL_Debug, "nmount returned: %d", mountResult);
	if (mountResult < 0)
	{
		WriteLog(LL_Warn, "could not mount usb0 to /mnt/usb0 (%d)", mountResult);
		return result;
	}

	WriteLog(LL_Debug, "overwriting sys_open from %p to %p", sv->sv_table[AUE_OPEN].sy_call, overlayfs_sys_open);

	_sys_open = (void*)sv->sv_table[AUE_OPEN].sy_call;

	critical_enter();
	cpu_disable_wp();

	sv->sv_table[AUE_OPEN].sy_call = (void*)overlayfs_sys_open;

	cpu_enable_wp();
	critical_exit();

	return result;
}

int overlayfs_sys_open(struct thread* td, struct open_args* uap)
{
	if (!_sys_open)
	{
		WriteLog(LL_Error, "HOLY FUCK THIS IS BAD");
		return -1;
	}

	struct overlayfs_t* fs = mira_getFramework()->overlayfs;

	void * (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	int(*snprintf)(char *str, size_t size, const char *format, ...) = kdlsym(snprintf);

	int(*copyinstr)(const void *uaddr, void *kaddr, size_t len, size_t *done) = kdlsym(copyinstr);

	// We need to copy the path from userland
	char inputPath[OVERLAYFS_MAXPATH];
	memset(inputPath, 0, sizeof(inputPath));

	size_t lengthCopied = 0;
	int copyResult = copyinstr(uap->path, inputPath, sizeof(inputPath), &lengthCopied);
	if (copyResult != 0)
	{
		WriteLog(LL_Error, "could not copyinstr path (%d)", copyResult);
		return _sys_open(td, uap);
	}

	// Format the redirected path
	char redirPath[OVERLAYFS_MAXPATH];
	memset(redirPath, 0, sizeof(redirPath));
	
	snprintf(redirPath, sizeof(redirPath), "%s%s", "/mnt/sandbox/CUSA08034_000/usb0", inputPath);

	if (!overlayfs_fileExists(fs, redirPath))
		return _sys_open(td, uap);

	WriteLog(LL_Debug, "attempting to open (%s) for replacement", redirPath);
	uap->path = redirPath;
		
	return  _sys_open(td, uap);
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