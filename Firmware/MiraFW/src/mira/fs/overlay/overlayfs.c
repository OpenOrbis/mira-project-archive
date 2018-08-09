//#include "overlayfs.h"
//#include <sys/eventhandler.h>
//#include <sys/fcntl.h>
//#include <sys/sysproto.h>
//#include <sys/uio.h>
//#include <sys/proc.h>
//#include <sys/imgact.h>
//#include <sys/mount.h>
//#include <sys/sysent.h>
//
//#include <oni/utils/logger.h>
//#include <oni/utils/sys_wrappers.h>
//#include <oni/utils/kdlsym.h>
//#include <oni/utils/hook.h>
//#include <oni/init/initparams.h>
//#include <oni/utils/sys_wrappers.h>
//
//#include <mira/miraframework.h>
//
//#include <sys/filedesc.h>
//#include <oni/utils/cpu.h>
//
//#include <mira/utils/escape.h>
//
//static void build_iovec(struct iovec **iov, int *iovlen, const char *name, void *val, size_t len);
//
//// Taken directly from sbin/mount/getmntopts.c
//static void build_iovec(struct iovec **iov, int *iovlen, const char *name, void *val, size_t len)
//{
//	size_t (*strlen)(const char *str) = kdlsym(strlen);
//	char * (*strdup)(const char *string, struct malloc_type* type) = kdlsym(strdup);
//	void* (*realloc)(void *addr, unsigned long size, struct malloc_type *mtp, int flags) = kdlsym(realloc);
//	struct malloc_type* M_MOUNT = kdlsym(M_MOUNT);
//
//	int i;
//
//	if (*iovlen < 0)
//		return;
//
//	i = *iovlen;
//
//	*iov = realloc(*iov, sizeof **iov * (i + 2), M_MOUNT, 0);
//
//	if (*iov == NULL) {
//		*iovlen = -1;
//		return;
//	}
//
//	(*iov)[i].iov_base = strdup(name, M_MOUNT);
//	(*iov)[i].iov_len = strlen(name) + 1;
//	i++;
//	(*iov)[i].iov_base = val;
//
//	if (len == (size_t)-1) {
//		if (val != NULL)
//			len = strlen(val) + 1;
//		else
//			len = 0;
//	}
//
//	(*iov)[i].iov_len = (int)len;
//	*iovlen = ++i;
//}
//
//static int mount_nullfs(struct thread* td, char* target, char* source, unsigned int flags)
//{
//	struct iovec* iov = NULL;
//	int iovlen = 0;
//
//	WriteLog(LL_Debug, "NullFS here !");
//
//	build_iovec(&iov, &iovlen, "fstype", "nullfs", (size_t)-1);
//	build_iovec(&iov, &iovlen, "fspath", source, (size_t)-1); // Where i want to add
//	build_iovec(&iov, &iovlen, "target", target, (size_t)-1); // What i want to add
//
//	int(*nmount)(struct thread* thread, struct nmount_args*) = kdlsym(sys_nmount);
//	if (!nmount)
//		return -1;
//
//	int error;
//	struct nmount_args uap;
//
//	// clear errors
//	td->td_retval[0] = 0;
//
//	// call syscall
//	uap.iovp = iov;
//	uap.iovcnt = iovlen;
//	uap.flags = flags;
//
//	error = nmount(td, &uap);
//	if (error)
//		return -error;
//
//	// return socket
//	return td->td_retval[0];
//}
//
//static int mount_unionfs(struct thread* td, char* target, char* source, unsigned int flags)
//{
//	struct iovec* iov = NULL;
//	int iovlen = 0;
//
//	WriteLog(LL_Debug, "here");
//
//	build_iovec(&iov, &iovlen, "fstype", "unionfs", (size_t)-1);
//	build_iovec(&iov, &iovlen, "fspath", source, (size_t)-1); // Where i want to add
//	build_iovec(&iov, &iovlen, "from", target, (size_t)-1); // What i want to add
//	build_iovec(&iov, &iovlen, "ro", "", (size_t)-1);
//
//	int(*nmount)(struct thread* thread, struct nmount_args*) = kdlsym(sys_nmount);
//	if (!nmount)
//		return -1;
//
//	int error;
//	struct nmount_args uap;
//
//	// clear errors
//	td->td_retval[0] = 0;
//
//	// call syscall
//	uap.iovp = iov;
//	uap.iovcnt = iovlen;
//	uap.flags = flags;
//
//	error = nmount(td, &uap);
//	if (error)
//		return -error;
//
//	// return socket
//	return td->td_retval[0];
//}
//
//int overlayfs_onExecNewVmspace(struct image_params* imgp, struct sysentvec* sv);
//
//// TODO : Resolve 5.05 / 5.01 function
//
//
//void overlayfs_init(struct overlayfs_t* fs)
//{
//	if (!fs)
//		return;
//
//	// Install the kernel process execution
//	fs->execNewVmspaceHook = hook_create(kdlsym(exec_new_vmspace), overlayfs_onExecNewVmspace);
//	hook_enable(fs->execNewVmspaceHook);
//
//	// We are currently ignoring all proc's
//	fs->pid = -1;
//}
//
///*
//struct proc *proc_find_by_name(char* name) {
//	struct proc *p;
//
//	int (*strcmp)(const char *s1, const char *s2) = kdlsym(strcmp);
//	p = *(struct proc **)kdlsym(allproc);
//
//	do {
//		if (strcmp(p->p_comm, name) == 0) {
//			return p;
//		}
//	} while ((p = p->p_list.le_next));
//
//	return NULL;
//}
//*/
//
//int overlayfs_onExecNewVmspace(struct image_params* imgp, struct sysentvec* sv)
//{
//	char* (*strstr)(const char*, const char*) = kdlsym(strstr);
//	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
//
//	//void(*critical_enter)(void) = kdlsym(critical_enter);
//	//void(*critical_exit)(void) = kdlsym(critical_exit);
//
//	//
//	//	This is purely for updating the current drive and mount point
//	//
//	// Get the framework
//	struct overlayfs_t* fs = mira_getFramework()->overlayfs;
//
//	int	(*exec_new_vmspace)(struct image_params *, struct sysentvec *) = hook_getFunctionAddress(fs->execNewVmspaceHook);
//	
//	// Call the original exec_new_vmspace
//	hook_disable(fs->execNewVmspaceHook);
//
//	int32_t result = exec_new_vmspace(imgp, sv);
//
//	hook_enable(fs->execNewVmspaceHook);
//
//	WriteLog(LL_Debug, "ExecNewVmspace called !\n");
//
//	if (strstr(imgp->execpath, "eboot") == NULL)
//	{
//		WriteLog(LL_Debug, "%s not eboot", imgp->execpath);
//		return result;
//	}
//
//	struct thread* td = imgp->proc->p_singlethread ? imgp->proc->p_singlethread : imgp->proc->p_threads.tqh_first;
//
//	int32_t pid = td->td_proc->p_pid;
//
//	struct thread_info_t prevInfo;
//	memset(&prevInfo, 0, sizeof(prevInfo));
//
//	mira_threadEscape(td, &prevInfo);
//
//	WriteLog(LL_Debug, "Process Thread td %p pid %d", td, pid);
//
//	// Create folder called CUSA00265_MOD (Minecraft)
//	int32_t mkdirResult = kmkdir_t("/mnt/CUSA08034_MOD", 0777, td);
//	if (mkdirResult < 0)
//	{
//		if (mkdirResult != -EEXIST) {
//			WriteLog(LL_Error, "could not create folder ! (%d)", mkdirResult);
//			return result;
//		} else {
//			WriteLog(LL_Debug, "Folder already exist, try anyway ...\n");
//		}
//	}
//
//	// Mount the folder CUSA00265_MOD with /mnt/sandbox/pfsmnt/CUSA00265-app0-patch0-union (Origin) in nullfs
//	// Cause crash (because UnionFS after)
//	int mountResult = mount_nullfs(td, "/mnt/sandbox/pfsmnt/CUSA08034-app0-patch0-union", "/mnt/CUSA08034_MOD", MNT_FORCE);
//	if (mountResult < 0)
//	{
//		WriteLog(LL_Warn, "could not mount with NullFS ! (%d)", mountResult);
//		return result;
//	}
//
//	// Cause crash when you try to go in /mnt/sandbox/pfsmnt/CUSA00265-app0-patch0-union with the ftp :P
//	mountResult = mount_unionfs(td, "/mnt/usb0", "/mnt/sandbox/pfsmnt/CUSA08034-app0-patch0-union", 0);
//	if (mountResult < 0)
//	{
//		WriteLog(LL_Warn, "could not mount with UnionFS ! (%d)", mountResult);
//		return result;
//	}
//
//	WriteLog(LL_Info, "UnionFS Re-mount ! (%d)", mountResult);
//
//	// Rewrote the good old dir
//	mira_threadRestore(td, &prevInfo);
//
//	return result;
//}



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

#include <oni/utils/logger.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/hook.h>
#include <oni/init/initparams.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/cpu.h>

#include <mira/miraframework.h>
#include <oni/utils/escape.h>

static void build_iovec(struct iovec **iov, int *iovlen, const char *name, void *val, size_t len);

// Taken directly from sbin/mount/getmntopts.c
static void build_iovec(struct iovec **iov, int *iovlen, const char *name, void *val, size_t len)
{
	size_t(*strlen)(const char *str) = kdlsym(strlen);
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

static int mount_nullfs(struct thread* td, char* fspath, char* target, unsigned int flags)
{
	struct iovec* iov = NULL;
	int iovlen = 0;

	WriteLog(LL_Debug, "NullFS here !");

	build_iovec(&iov, &iovlen, "fstype", "nullfs", (size_t)-1);
	build_iovec(&iov, &iovlen, "fspath", fspath, (size_t)-1); // Where i want to add
	build_iovec(&iov, &iovlen, "target", target, (size_t)-1); // What i want to add

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
	build_iovec(&iov, &iovlen, "allow_other", "", (size_t)-1); // What i want to add

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
int overlayfs_nmountHook(struct thread* td, struct nmount_args* uap);
int overlayfs_mkdirHook(struct thread* td, struct mkdir_args* uap);

int(*_nmount)(struct thread* td, struct nmount_args* uap) = NULL;
int(*_mkdir)(struct thread* td, struct mkdir_args* uap) = NULL;

// TODO : Resolve 5.05 / 5.01 function

void overlayfs_init(struct overlayfs_t* fs)
{
	if (!fs)
		return;

	struct sysentvec* sv = kdlsym(self_orbis_sysvec);

	struct sysent* sysents = sv->sv_table;
	//void(*critical_enter)(void) = kdlsym(critical_enter);	
	//void(*critical_exit)(void) = kdlsym(critical_exit);

	// Install the kernel process execution
	fs->execNewVmspaceHook = hook_create(kdlsym(exec_new_vmspace), overlayfs_onExecNewVmspace);

	// Hook the syscall nmount for reverse
	_nmount = (void*)sysents[378].sy_call;
	_mkdir = (void*)sysents[136].sy_call;

	/*
	critical_enter();
	cpu_disable_wp();

	sysents[378].sy_call = (sy_call_t*)overlayfs_nmountHook; // AUE_NMOUNT
	sysents[136].sy_call = (sy_call_t*)overlayfs_mkdirHook; // AUE_MKDIR

	cpu_enable_wp();
	critical_exit();

	WriteLog(LL_Debug, "AUE_NMOUNT: Hook installed (Original: %p) !\n", _nmount);
	WriteLog(LL_Debug, "AUE_MKDIR: Hook installed (Original: %p) !\n", _mkdir);
	*/

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

int overlayfs_mkdirHook(struct thread* td, struct mkdir_args* uap) {
	int(*copyinstr)(const void *uaddr, void *kaddr, size_t len, size_t *done) = kdlsym(copyinstr);

	struct proc* p = td->td_proc;

	char path[300];
	size_t path_len;
	copyinstr(uap->path, path, sizeof(path), &path_len);

	WriteLog(LL_Debug, "[MKDIR] PID: %d MODE: %d PATH: %s\n", p->p_pid, uap->mode, path);

	return _mkdir(td, uap);
}

int overlayfs_nmountHook(struct thread* td, struct nmount_args* uap) {
	int(*copyin)(const void *uaddr, void *kaddr, size_t len) = kdlsym(copyin);
	struct proc* p = td->td_proc;

	WriteLog(LL_Debug, "--- NEW NMOUNT ---\n");
	WriteLog(LL_Debug, "Thread: %p PID: %d\n", td, p->p_pid);
	WriteLog(LL_Debug, "Process name: %s\n", p->p_comm);
	WriteLog(LL_Debug, "Mount flags (Without M_ROOTFS): %08llX\n", uap->flags);

	if (uap->flags & MNT_RDONLY) {
		WriteLog(LL_Debug, "Mount flags: MNT_RDONLY\n");
	}

	if (uap->flags & MNT_NOEXEC) {
		WriteLog(LL_Debug, "Mount flags: MNT_NOEXEC\n");
	}

	if (uap->flags & MNT_UNION) {
		WriteLog(LL_Debug, "Mount flags: MNT_UNION\n");
	}

	if (uap->flags & MNT_ASYNC) {
		WriteLog(LL_Debug, "Mount flags: MNT_ASYNC\n");
	}

	if (uap->flags & MNT_NOATIME) {
		WriteLog(LL_Debug, "Mount flags: MNT_NOATIME\n");
	}

	if (uap->flags & MNT_NOSYMFOLLOW) {
		WriteLog(LL_Debug, "Mount flags: MNT_NOSYMFOLLOW\n");
	}

	if (uap->flags & MNT_UPDATE) {
		WriteLog(LL_Debug, "Mount flags: MNT_UPDATE\n");
	}

	if (uap->flags & MNT_FORCE) {
		WriteLog(LL_Debug, "Mount flags: MNT_FORCE\n");
	}

	if (uap->flags & MNT_SNAPSHOT) {
		WriteLog(LL_Debug, "Mount flags: MNT_SNAPSHOT\n");
	}


	struct iovec* iov = (struct iovec*)kmalloc(uap->iovcnt * sizeof(struct iovec));
	copyin(uap->iovp, iov, uap->iovcnt * sizeof(struct iovec));

	for (int i = 0; i < uap->iovcnt; i++) {
		char* data = kmalloc(iov[i].iov_len);
		copyin(iov[i].iov_base, data, iov[i].iov_len);

		WriteLog(LL_Debug, "IOV[%i]: %s\n", i, data);

		kfree(data, iov[i].iov_len);
	}

	kfree(iov, uap->iovcnt * sizeof(struct iovec));

	WriteLog(LL_Debug, "--- END NMOUNT ---\n");

	return _nmount(td, uap);
}

int overlayfs_onExecNewVmspace(struct image_params* imgp, struct sysentvec* sv)
{
	char* (*strstr)(const char*, const char*) = kdlsym(strstr);
	void* (*memset)(void *b, int c, size_t len) = kdlsym(memset);

	//
	//	This is purely for updating the current drive and mount point
	//
	// Get the framework
	struct overlayfs_t* fs = mira_getFramework()->overlayfs;

	int(*exec_new_vmspace)(struct image_params *, struct sysentvec *) = hook_getFunctionAddress(fs->execNewVmspaceHook);

	// Call the original exec_new_vmspace
	hook_disable(fs->execNewVmspaceHook);

	int32_t result = exec_new_vmspace(imgp, sv);

	hook_enable(fs->execNewVmspaceHook);

	WriteLog(LL_Debug, "ExecNewVmspace called !\n");

	if (strstr(imgp->execpath, "eboot") == NULL)
	{
		WriteLog(LL_Debug, "%s not eboot", imgp->execpath);
		return result;
	}

	struct thread* td = imgp->proc->p_singlethread ? imgp->proc->p_singlethread : imgp->proc->p_threads.tqh_first;
	int32_t pid = td->td_proc->p_pid;

	struct thread_info_t prevInfo;
	memset(&prevInfo, 0, sizeof(prevInfo));

	oni_threadEscape(td, &prevInfo);

	WriteLog(LL_Debug, "Process Thread td %p pid %d", td, pid);

	// Create folder

	if (fs->pid == -1) {
		fs->pid = 1;
		int32_t mkdirResult = kmkdir_t("/mnt/sandbox/pfsmnt/CUSA00265-mod0", 0511, td);
		if (mkdirResult < 0)
		{
			if (mkdirResult != -EEXIST) {
				WriteLog(LL_Error, "could not create folder ! (%d)", mkdirResult);
				return result;
			}
			else {
				WriteLog(LL_Debug, "Folder already exist, try anyway ...\n");
			}
		}

		int mountResult = mount_nullfs(td, "/mnt/sandbox/pfsmnt/CUSA00265-mod0", "/mnt/sandbox/CUSA00265_000/app0", MNT_RDONLY);
		if (mountResult < 0)
		{
			WriteLog(LL_Warn, "could not mount with NullFS ! (%d)", mountResult);
			return result;
		}

		mountResult = mount_unionfs(td, "/mnt/sandbox/pfsmnt/CUSA00265-mod0", "/user/data/CUSA00265", MNT_RDONLY);
		if (mountResult < 0)
		{
			WriteLog(LL_Warn, "could not mount with UnionFS ! (%d)", mountResult);
			return result;
		}

		WriteLog(LL_Info, "UnionFS Re-mount ! (%d)", mountResult);
	}
	else {
		WriteLog(LL_Warn, "OverlayFS Already done.");

	}

	oni_threadRestore(td, &prevInfo);

	return result;
}

