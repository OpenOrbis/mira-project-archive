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

#include <mira/miraframework.h>
#include <mira/utils/escape.h>
#include <mira/utils/escape.h>

typedef uint32_t seq_t;
#define	CAP_RIGHTS_VERSION_00	0
#define	CAP_RIGHTS_VERSION	CAP_RIGHTS_VERSION_00

struct cap_rights {
	uint64_t	cr_rights[CAP_RIGHTS_VERSION + 2];
};

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

static int unmount(char* path, int flags) {	
	int(*sys_unmount)(struct thread* thread, struct unmount_args*) = kdlsym(sys_unmount);
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
int overlayfs_nmountHook(struct thread* td, struct nmount_args* uap);
int overlayfs_mkdirHook(struct thread* td, struct mkdir_args* uap);

int(*_nmount)(struct thread* td, struct nmount_args* uap) = NULL;	
int (*_mkdir)(struct thread* td, struct mkdir_args* uap) = NULL;

// TODO : Resolve 5.05 / 5.01 function

void overlayfs_init(struct overlayfs_t* fs)
{
	if (!fs)
		return;

	struct sysent* sysents = kdlsym(kern_sysents); 
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

int overlayfs_mkdirHook(struct thread* td, struct mkdir_args* uap) {
	int (*copyinstr)(const void *uaddr, void *kaddr, size_t len, size_t *done) = kdlsym(copyinstr);

	struct proc* p = td->td_proc;

	char path[300];
	size_t path_len;
	copyinstr(uap->path, path, sizeof(path), &path_len);

	WriteLog(LL_Debug, "[MKDIR] PID: %d MODE: %d PATH: %s\n", p->p_pid, uap->mode, path);

	return _mkdir(td, uap);
}

int overlayfs_nmountHook(struct thread* td, struct nmount_args* uap) {
	int (*copyin)(const void *uaddr, void *kaddr, size_t len) = kdlsym(copyin);
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
	int (*sprintf)(char * restrict str, const char * restrict format, ...) = kdlsym(sprintf);
	//int	(*fget_unlocked)(struct filedesc *fdp, int fd, cap_rights_t *needrightsp, struct file **fpp, seq_t *seqp) = kdlsym(fget_unlocked);
	
	// Get the framework
	struct overlayfs_t* fs = mira_getFramework()->overlayfs;

	int	(*exec_new_vmspace)(struct image_params *, struct sysentvec *) = hook_getFunctionAddress(fs->execNewVmspaceHook);
	
	// Call the original exec_new_vmspace
	hook_disable(fs->execNewVmspaceHook);
	int32_t result = exec_new_vmspace(imgp, sv);
	hook_enable(fs->execNewVmspaceHook);

	WriteLog(LL_Debug, "ExecNewVmspace called !\n");

	// Check if eboot is present
	if (strstr(imgp->execpath, "eboot") == NULL)
	{
		WriteLog(LL_Debug, "%s not eboot", imgp->execpath);
		return result;
	}

	// Get process information
	struct thread* td = imgp->proc->p_singlethread ? imgp->proc->p_singlethread : imgp->proc->p_threads.tqh_first;
	int32_t pid = td->td_proc->p_pid;
	char* appid = td->td_proc + 0x390;

	struct thread_info_t prevInfo;
	memset(&prevInfo, 0, sizeof(prevInfo));

	mira_threadEscape(td, &prevInfo);

	WriteLog(LL_Debug, "Process Thread td %p pid %d appid: %s", td, pid, appid);

	int32_t mountResult, mkdirResult = -1;

	// Get SceShellCore process for handle data
	struct proc* shellcore = proc_find_by_name("SceShellCore");
	struct thread* shellcore_td = shellcore->p_threads.tqh_first;

	// Parse path with app data
	char app0[255];
	char sandbox_mod[255];
	char pfsmnt_mod[255];
	char pfsmnt_app[255];
	char pfsmnt_patch[255];
	char pfsmnt_union[255];
	char data_mod[255];

	sprintf(app0, "/mnt/sandbox/%s_000/app0", appid);
	sprintf(pfsmnt_mod, "/mnt/sandbox/pfsmnt/%s-mod0", appid);
	sprintf(pfsmnt_app, "/mnt/sandbox/pfsmnt/%s-app0", appid);
	sprintf(pfsmnt_patch, "/mnt/sandbox/pfsmnt/%s-patch0", appid);
	sprintf(pfsmnt_union, "/mnt/sandbox/pfsmnt/%s-app0-patch0-union", appid);
	sprintf(data_mod, "/user/data/%s", appid);

	// Unmount new created folder
	mountResult = unmount(app0, 0);
	if (mountResult < 0) {
		WriteLog(LL_Warn, "could not unmount %s (%d)", app0, mountResult);
	}

	mountResult = unmount(pfsmnt_union, 0);
	if (mountResult < 0) {
		WriteLog(LL_Warn, "could not unmount %s (%d)", app0, mountResult);
	}

	// Create folder if not exist
	overlayfs_createDirectory(shellcore_td, pfsmnt_mod, 0777);

	// Mounting needed
	mountResult = mount_nullfs(shellcore_td, pfsmnt_mod, data_mod, "rw", 0);
	if (mountResult < 0)
	{
		WriteLog(LL_Warn, "could not mount NullFS %s on %s ! (%d)", data_mod, pfsmnt_mod, mountResult);
		goto end;
	}

	mountResult = mount_nullfs(shellcore_td, app0, pfsmnt_app, "ro", MNT_RDONLY);
	if (mountResult < 0)
	{
		WriteLog(LL_Warn, "could not mount NullFS %s on %s ! (%d)", pfsmnt_app, app0, mountResult);
		goto end;
	}

	mountResult = mount_unionfs(shellcore_td, app0, pfsmnt_mod, MNT_RDONLY);
	if (mountResult < 0)
	{
		WriteLog(LL_Warn, "could not mount with UnionFS1 ! (%d)", mountResult);
		goto end;
	}

	mountResult = mount_nullfs(shellcore_td, pfsmnt_union, app0, "ro", MNT_RDONLY);
	if (mountResult < 0)
	{
		WriteLog(LL_Warn, "could not mount NullFS %s on %s ! (%d)", pfsmnt_app, app0, mountResult);
		goto end;
	}


	// Doesn't work if you nullfs a unionfs
	// Doesn't work if you want make multi union in same directory

	/* Doesn't work for now (fget_err return random error ?!)
	struct file *fp;

	int fd = kopen_t("/mnt/sandbox/CUSA07184_MOD", O_RDONLY | O_DIRECTORY, 0, td);
	if (fd < 0) {
		WriteLog(LL_Error, "Error with open ! (%d)", fd);
		return result;
	}

	cap_rights_t right;
	memset(&right, 0, sizeof(cap_rights_t));

	int fget_err = fget_unlocked(td->td_proc->p_fd, fd, &right, &fp, NULL); // Sa crash ici :/
	if (fget_err != 0) {
		WriteLog(LL_Error, "Error with fget_unlocked ! (/!\\ Try anyway) (%i)", fget_err);
	}
	*/

	//struct filedesc* fd_ptr = td->td_proc->p_fd;
	//fd_ptr->fd_rdir = fd_ptr->fd_jdir = fp->f_vnode;

	// fp->f_vnode
	WriteLog(LL_Info, "Custom folder created and set ! (%d)", mountResult);

	end:
	mira_threadRestore(td, &prevInfo);
	return result;
}
