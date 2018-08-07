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

#include <sys/filedesc.h>				// filedesc
#include <oni/utils/cpu.h>

struct alt_filedesc {
	void *useless1[3];
	void *fd_rdir;
	void *fd_jdir;
};

int overlayfs_sys_open(struct thread* td, struct open_args* uap);

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

static int mount_nullfs(struct thread* td, char* target, char* source, unsigned int flags)
{
	struct iovec* iov = NULL;
	int iovlen = 0;

	WriteLog(LL_Debug, "NullFS here !");

	build_iovec(&iov, &iovlen, "fstype", "nullfs", (size_t)-1);
	build_iovec(&iov, &iovlen, "fspath", source, (size_t)-1); // Ou je veut le mettre
	build_iovec(&iov, &iovlen, "target", target, (size_t)-1); // Se que je veut mettre

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

static int mount_unionfs(struct thread* td, char* target, char* source, unsigned int flags)
{
	struct iovec* iov = NULL;
	int iovlen = 0;

	WriteLog(LL_Debug, "here");

	build_iovec(&iov, &iovlen, "fstype", "unionfs", (size_t)-1);
	build_iovec(&iov, &iovlen, "fspath", source, (size_t)-1); // Ou je veut le mettre
	build_iovec(&iov, &iovlen, "from", target, (size_t)-1); // Se que je veut mettre
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

// 5.05 Stuff
#define kdlsym_addr_exec_new_vmspace  0x0038AD10
#define kdlsym_addr_kern_mkdirat 0x00340BD0
#define kdlsym_addr_strcmp 0x001D0FD0

/*
// 5.01 Stuff
#define kdlsym_addr_dmem_start_app_process 0x002468E0
#define kdlsym_addr_exec_new_vmspace 0x0038A940
#define kdlsym_addr_kern_open 0x0072AB50
#define kdlsym_addr_kern_openat 0x0033B640
#define kdlsym_addr_kern_readv 0x00152A10
#define kdlsym_addr_kern_close  0x000C0F40
*/

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

int overlayfs_onExecNewVmspace(struct image_params* imgp, struct sysentvec* sv)
{
	char* (*strstr)(const char*, const char*) = kdlsym(strstr);
	int (*kern_mkdirat)(struct thread *td, int fd, char *path, enum uio_seg segflg, int mode) = kdlsym(kern_mkdirat);

	//void(*critical_enter)(void) = kdlsym(critical_enter);
	//void(*critical_exit)(void) = kdlsym(critical_exit);

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

	WriteLog(LL_Debug, "ExecNewVmspace called !\n");

	if (strstr(imgp->execpath, "eboot") == NULL)
	{
		WriteLog(LL_Debug, "%s not eboot", imgp->execpath);
		return result;
	}

	struct thread* td = imgp->proc->p_singlethread ? imgp->proc->p_singlethread : imgp->proc->p_threads.tqh_first;
	int32_t pid = td->td_proc->p_pid;

	struct ucred* cred = td->td_proc->p_ucred;
	struct filedesc* fd = td->td_proc->p_fd;

	// Escalate privileges
	cred->cr_uid = 0;
	cred->cr_ruid = 0;
	cred->cr_rgid = 0;
	cred->cr_groups[0] = 0;

	// Game Escape sandbox
	void* old_r_dir = fd->fd_rdir;
	cred->cr_prison = *(void**)kdlsym(prison0);
	fd->fd_rdir = fd->fd_jdir = *(void**)kdlsym(rootvnode);

	WriteLog(LL_Debug, "Process Thread td %p pid %d", td, pid);

	// Ont créer le dossier CUSA00265_MOD

	int32_t mkdirResult = kern_mkdirat(td, AT_FDCWD, "/mnt/CUSA00265_MOD", UIO_SYSSPACE, 0777);
	if (mkdirResult > 0)
	{
		if (mkdirResult != EEXIST) {
			WriteLog(LL_Error, "could not create folder ! (%d)", mkdirResult);
			return result;
		} else {
			WriteLog(LL_Debug, "Folder already exist, try anyway ...\n");
		}
	}

	// Mount the folder CUSA00265_MOD with /mnt/sandbox/pfsmnt/CUSA00265-app0-patch0-union (Origin) in nullfs
	int mountResult = mount_nullfs(td, "/mnt/sandbox/pfsmnt/CUSA00265-app0-patch0-union", "/mnt/CUSA00265_MOD", MNT_FORCE);
	if (mountResult < 0)
	{
		WriteLog(LL_Warn, "could not mount with NullFS ! (%d)", mountResult);
		return result;
	}

	// Cause crash when you try to go in /mnt/sandbox/pfsmnt/CUSA00265-app0-patch0-union with the ftp :P
	mountResult = mount_unionfs(td, "/mnt/usb0", "/mnt/sandbox/pfsmnt/CUSA00265-app0-patch0-union", 0);
	if (mountResult < 0)
	{
		WriteLog(LL_Warn, "could not mount with UnionFS ! (%d)", mountResult);
		return result;
	}

	WriteLog(LL_Info, "UnionFS Re-mount ! (%d)", mountResult);

	// Rewrote the good old dir
	fd->fd_rdir = old_r_dir;

	return result;
}
