#include "overlayfs.h"
#include <sys/eventhandler.h>
#include <sys/fcntl.h>
#include <sys/sysproto.h>
#include <sys/uio.h>
#include <sys/proc.h>
#include <sys/imgact.h>
#include <sys/mount.h>
#include <sys/sysent.h>
#include <sys/filedesc.h>
#include <sys/file.h>

#include <oni/utils/logger.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/hook.h>
#include <oni/init/initparams.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/cpu.h>
#include <oni/utils/escape.h>

#include <mira/miraframework.h>

static void build_iovec(struct iovec **iov, int *iovlen, const char *name, void *val, size_t len);

// Taken directly from sbin/mount/getmntopts.c
static void build_iovec(struct iovec **iov, int *iovlen, const char *name, void *val, size_t len)
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

// Unmount syscall
int kunmount_t(char* path, int flags, struct thread* td) {	
	struct sysent* sysents = kdlsym(kern_sysents);
	int(*sys_unmount)(struct thread* thread, struct unmount_args*) = (void*)sysents[AUE_UNMOUNT].sy_call;
	if (!sys_unmount)
		return -1;

	int error;
	struct unmount_args uap;

	// clear errors
	td->td_retval[0] = 0;

	// call syscall
	uap.path = path;
	uap.flags = flags;

	error = sys_unmount(td, &uap);
	if (error)
		return -error;

	// return socket
	return td->td_retval[0];
}

// Chmod syscall
int kchmod_t(char* path, int mode, struct thread* td) {
	struct sysent* sysents = kdlsym(kern_sysents);
	int(*sys_chmod)(struct thread*, struct chmod_args*) = (void*)sysents[AUE_CHMOD].sy_call;
	if (!sys_chmod)
		return (int)-1337;

	int error;
	struct chmod_args uap;

	// clear errors
	td->td_retval[0] = 0;

	// call syscall
	uap.path = path;
	uap.mode = mode;
	error = sys_chmod(td, &uap);
	if (error)
		return (int)-error;

	// return
	return (int)td->td_retval[0];
}

static int mount_nullfs(struct thread* td, char* fspath, char* target, char* protection, unsigned int flags)
{
	struct iovec* iov = NULL;
	int iovlen = 0;

	WriteLog(LL_Debug, "NullFS here !");

	build_iovec(&iov, &iovlen, "fstype", "nullfs", (size_t)-1);
	build_iovec(&iov, &iovlen, "fspath", fspath, (size_t)-1); // Where i want to add
	build_iovec(&iov, &iovlen, "target", target, (size_t)-1); // What i want to add
	build_iovec(&iov, &iovlen, "allow_other", "", (size_t)-1); // What i want to add
	build_iovec(&iov, &iovlen, protection, "", (size_t)-1);

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

static int mount_unionfs(struct thread* td, char* fspath, char* from, unsigned int flags)
{
	struct iovec* iov = NULL;
	int iovlen = 0;

	WriteLog(LL_Debug, "here");

	build_iovec(&iov, &iovlen, "fstype", "unionfs", (size_t)-1);
	build_iovec(&iov, &iovlen, "fspath", fspath, (size_t)-1); // Where i want to add
	build_iovec(&iov, &iovlen, "from", from, (size_t)-1); // What i want to add
	build_iovec(&iov, &iovlen, "allow_other", "", (size_t)-1);
	build_iovec(&iov, &iovlen, "ro", "", (size_t)-1);

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
int overlayfs_unmountHook(struct thread* td, struct unmount_args* uap);

int (*_unmount)(struct thread* td, struct unmount_args* uap) = NULL;

// TODO : Resolve 5.05 / 5.01 function

void overlayfs_init(struct overlayfs_t* fs)
{
	if (!fs)
		return;

	struct sysent* sysents = kdlsym(kern_sysents); 
	void(*critical_enter)(void) = kdlsym(critical_enter);	
	void(*critical_exit)(void) = kdlsym(critical_exit);

	// Install the kernel process execution
	fs->execNewVmspaceHook = hook_create(kdlsym(exec_new_vmspace), overlayfs_onExecNewVmspace);
	
	// Hook the syscall nmount for reverse
	_unmount = (void*)sysents[22].sy_call;

	critical_enter();	
	cpu_disable_wp();

	sysents[22].sy_call = (sy_call_t*)overlayfs_unmountHook;

	cpu_enable_wp();	
	critical_exit();

	WriteLog(LL_Debug, "AUE_UNMOUNT: Hook installed (Original: %p) !\n", _unmount);

	hook_enable(fs->execNewVmspaceHook);

	// We are currently ignoring all proc's
	fs->pid = -1;
}

/*
struct proc *proc_find_by_name(char* name) {
	struct proc *p;

	int (*strcmp)(const char *s1, const char *s2) = kdlsym(strcmp);
	p = *(struct proc **)kdlsym(allproc);

	do {
		if (strcmp(p->p_comm, name) == 0) {
			return p;
		}
	} while ((p = p->p_list.le_next));

	return NULL;
}
*/

int overlayfs_unmountHook(struct thread* td, struct unmount_args* uap) {
	int (*copyinstr)(const void *uaddr, void *kaddr, size_t len, size_t *done) = kdlsym(copyinstr);
	char* (*strstr)(const char *haystack, const char *needle) = kdlsym(strstr);
	int (*strcmp)(const char *str1, const char* str2) = kdlsym(strcmp);
	void* (*memcpy)(void *restrict dst, const void *restrict src, size_t n) = kdlsym(memcpy);
	void* (*memset)(void *b, int c, size_t len) = kdlsym(memset);
	int (*snprintf)(char * restrict str, size_t size, const char * restrict format, ...) = kdlsym(snprintf);

	struct proc* p = td->td_proc;

	char path[300];
	size_t path_len;
	copyinstr(uap->path, path, sizeof(path), &path_len);


	if (strcmp("SceShellCore", p->p_comm) == 0) {
		//WriteLog(LL_Debug, "[Unmount] Unmounting %s ...\n", path);
		char* pos1 = strstr(path, "/mnt/sandbox/");
		char* pos2 = strstr(path, "/mnt/sandbox/pfsmnt/");

		if (pos1 != NULL || pos2 != NULL) {

			uap->flags = MNT_FORCE;

			char appid[10];
			memset(appid, 0, sizeof(appid));

			if (pos2 != NULL) { 
				memcpy(appid, (char*)(pos2 + 20), 9);
			} else {
				memcpy(appid, (char*)(pos1 + 13), 9);
			}

			char pfsmnt_mod[300];
			snprintf(pfsmnt_mod, 300, "/mnt/sandbox/pfsmnt/%s-mod0", appid);

			int ret = -1;

			struct thread_info_t prevInfo;
			memset(&prevInfo, 0, sizeof(prevInfo));

			oni_threadEscape(td, &prevInfo);

			uap->path = pfsmnt_mod;
			ret = _unmount(td, uap);
			WriteLog(LL_Debug, "[Unmount][%s] Unmount mod0: %s => %d\n", appid, uap->path, ret);

			ret = krmdir_t(pfsmnt_mod, td);
			WriteLog(LL_Debug, "[Unmount][%s] Delete mod0: %s => %d\n", appid, pfsmnt_mod, ret);

			oni_threadRestore(td, &prevInfo);

			uap->path = path;
			goto end;
		}
	}

	end:
	return _unmount(td, uap);
}

int overlayfs_createDirectory(struct thread* td, char* path, int mode) {
	// Create needed folder for mount
	int result = kmkdir_t(path, mode, td);
	if (result < 0)
	{
		if (result != -EEXIST) {
			WriteLog(LL_Error, "could not create %s ! (%d)", path, result);
			return result;
		} else {
			WriteLog(LL_Debug, "Folder %s already exist, try anyway ...\n", path);
		}
	}

	return result;
}

int overlayfs_onExecNewVmspace(struct image_params* imgp, struct sysentvec* sv)
{
	char* (*strstr)(const char*, const char*) = kdlsym(strstr);
	void* (*memset)(void *b, int c, size_t len) = kdlsym(memset);
	int (*snprintf)(char * restrict str, size_t size, const char * restrict format, ...) = kdlsym(snprintf);
	//int	(*fget_unlocked)(struct filedesc *fdp, int fd, cap_rights_t *needrightsp, struct file **fpp, seq_t *seqp) = kdlsym(fget_unlocked);
	
	// Get the framework
	struct overlayfs_t* fs = mira_getFramework()->overlayfs;

	int	(*exec_new_vmspace)(struct image_params *, struct sysentvec *) = hook_getFunctionAddress(fs->execNewVmspaceHook);
	
	// Call the original exec_new_vmspace
	hook_disable(fs->execNewVmspaceHook);
	int32_t ret = exec_new_vmspace(imgp, sv);
	hook_enable(fs->execNewVmspaceHook);

	WriteLog(LL_Debug, "ExecNewVmspace called !\n");

	// Check if eboot is present
	if (strstr(imgp->execpath, "eboot") == NULL)
	{
		WriteLog(LL_Debug, "%s not eboot", imgp->execpath);
		return ret;
	}

	// Get process information
	struct thread* td = imgp->proc->p_singlethread ? imgp->proc->p_singlethread : imgp->proc->p_threads.tqh_first;
	int32_t pid = td->td_proc->p_pid;

	uint64_t appid_add = 0x390;
	uint64_t proc_addr = (uint64_t)curthread->td_proc;	
	uint64_t r = proc_addr + appid_add;

	char* appid = (char*)r;

	WriteLog(LL_Info, "Proc addr: %p\n", curthread->td_proc);
	WriteLog(LL_Info, "AppID (%p): %s\n", appid, appid);

	struct thread_info_t prevInfo;
	memset(&prevInfo, 0, sizeof(prevInfo));

	oni_threadEscape(td, &prevInfo);

	WriteLog(LL_Debug, "Process Thread td %p pid %d appid: %s", td, pid, appid);

	int32_t result = -1;

	// Parse path with app data
	char app0[300];
	char pfsmnt_mod[300];
	char pfsmnt_app[300];
	char pfsmnt_union[300];
	char data_mod[300];


	snprintf(app0, 300, "/mnt/sandbox/%s_000/app0", appid);
	snprintf(pfsmnt_mod, 300, "/mnt/sandbox/pfsmnt/%s-mod0", appid);
	snprintf(pfsmnt_app, 300, "/mnt/sandbox/pfsmnt/%s-app0", appid);
	snprintf(pfsmnt_union, 300, "/mnt/sandbox/pfsmnt/%s-app0-patch0-union", appid);
	snprintf(data_mod, 300, "/user/data/%s", appid);

	result = kchmod_t(data_mod, 0777, td);
	if (result < 0) {
		if (result == -30 || result == -1) {
			WriteLog(LL_Warn, "Mod folder exist but in RO !\n");
		} else {
			WriteLog(LL_Error, "Mod folder doesn't exist (%s). Abording ... (err: %i)\n", data_mod, result);
			goto end;
		}
	}

	WriteLog(LL_Info, "Mod folder exist: %s\n", data_mod);

	// Create folder 
	overlayfs_createDirectory(td, pfsmnt_mod, 0777);

	// Mounting needed
	result = mount_nullfs(td, pfsmnt_mod, data_mod, "rw", 0);
	if (result < 0)
	{
		WriteLog(LL_Warn, "could not mount NullFS %s on %s ! (%d)", data_mod, pfsmnt_mod, result);
		goto end;
	}

	result = mount_nullfs(td, app0, pfsmnt_app, "ro", MNT_RDONLY);
	if (result < 0)
	{
		WriteLog(LL_Warn, "could not mount NullFS %s on %s ! (%d)", pfsmnt_app, app0, result);
		goto end;
	}

	// Crash here
	/*
	result = mount_unionfs(td, app0, pfsmnt_patch, MNT_RDONLY);
	if (result < 0)
	{
		WriteLog(LL_Warn, "could not mount with UnionFS1 ! (%d)", result);
		goto end;
	}
	*/

	result = mount_unionfs(td, app0, pfsmnt_mod, MNT_RDONLY);
	if (result < 0)
	{
		WriteLog(LL_Warn, "could not mount with UnionFS2 ! (%d)", result);
		goto end;
	}

	result = mount_nullfs(td, pfsmnt_union, app0, "ro", MNT_RDONLY);
	if (result < 0)
	{
		WriteLog(LL_Warn, "could not mount NullFS %s on %s ! (%d)", pfsmnt_app, app0, result);
		goto end;
	}

	WriteLog(LL_Info, "All mounted with success !");


	// Doesn't work if you nullfs a unionfs
	// Doesn't work if you want make multi union in same directory

	WriteLog(LL_Info, "OverlayFS done.");

	end:
	oni_threadRestore(td, &prevInfo);
	return ret;
}
