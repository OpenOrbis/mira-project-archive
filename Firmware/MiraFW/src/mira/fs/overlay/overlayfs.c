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
#include <sys/stat.h>

#include <oni/utils/logger.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/hook.h>
#include <oni/init/initparams.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/ref.h>
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
int kunmount_t(char* path, int flags, struct thread* td) 
{	
	struct sysentvec* sv = kdlsym(self_orbis_sysvec);
	struct sysent* sysents = sv->sv_table;
	int(*sys_unmount)(struct thread* thread, struct unmount_args*) = (void*)sysents[22].sy_call;
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

// Mount nullfs filesystem
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

// Mount unionfs filesystem
static int mount_unionfs(struct thread* td, char* fspath, char* from, char* protection, unsigned int flags)
{
	struct iovec* iov = NULL;
	int iovlen = 0;

	WriteLog(LL_Debug, "UnionFS here !");

	build_iovec(&iov, &iovlen, "fstype", "unionfs", (size_t)-1);
	build_iovec(&iov, &iovlen, "fspath", fspath, (size_t)-1); // Where i want to add
	build_iovec(&iov, &iovlen, "from", from, (size_t)-1); // What i want to add
	build_iovec(&iov, &iovlen, "allow_other", "", (size_t)-1);
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

int overlayfs_onExecNewVmspace(struct image_params* imgp, struct sysentvec* sv);
int overlayfs_rmdirHook(struct thread* td, struct rmdir_args* uap);

int (*_rmdir)(struct thread* td, struct rmdir_args* uap) = NULL;

void overlayfs_init(struct overlayfs_t* fs)
{
	if (!fs)
		return;
	struct sysentvec* sv = kdlsym(self_orbis_sysvec);
	struct sysent* sysents = sv->sv_table;
	void(*critical_enter)(void) = kdlsym(critical_enter);	
	void(*critical_exit)(void) = kdlsym(critical_exit);

	// Install the kernel process execution
	fs->execNewVmspaceHook = hook_create(kdlsym(exec_new_vmspace), overlayfs_onExecNewVmspace);
	
	// Hook the syscall rmdir
	_rmdir = (void*)sysents[137].sy_call;

	critical_enter();	
	cpu_disable_wp();

	sysents[137].sy_call = (sy_call_t*)overlayfs_rmdirHook;

	cpu_enable_wp();	
	critical_exit();

	WriteLog(LL_Debug, "AUE_RMDIR: Hook installed (Original: %p) !\n", _rmdir);

	hook_enable(fs->execNewVmspaceHook);

	// We are currently ignoring all proc's
	fs->pid = -1;
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

int overlayfs_rmdirHook(struct thread* td, struct rmdir_args* uap) 
{
	int (*copyinstr)(const void *uaddr, void *kaddr, size_t len, size_t *done) = kdlsym(copyinstr);
	int (*strcmp)(const char *str1, const char* str2) = kdlsym(strcmp);
	char* (*strstr)(const char *haystack, const char *needle) = kdlsym(strstr);
	void* (*memset)(void *b, int c, size_t len) = kdlsym(memset);
	void* (*memcpy)(void *restrict dst, const void *restrict src, size_t n) = kdlsym(memcpy);
	int (*snprintf)(char * restrict str, size_t size, const char * restrict format, ...) = kdlsym(snprintf);

	struct proc* p = td->td_proc;

	char path[300];
	size_t path_len;
	copyinstr(uap->path, path, sizeof(path), &path_len);

	if (strcmp("SceShellCore", p->p_comm) == 0)
	{
		char* pos1 = strstr(path, "/mnt/sandbox/");
		char* pos2 = strstr(path, "/mnt/sandbox/pfsmnt/");

		if (pos1 != NULL || pos2 != NULL) {

			// Get the AppID
			char appid[10];
			memset(appid, 0, sizeof(appid));

			if (pos2 != NULL) { 
				memcpy(appid, (char*)(pos2 + 20), 9);
			} else {
				memcpy(appid, (char*)(pos1 + 13), 9);
			}

			// Generate needed path
			char app_folder[300];
			char app0[300];
			char pfsmnt_mod[300];

			snprintf(app0, 300, "/mnt/sandbox/%s_000/app0", appid);
			snprintf(pfsmnt_mod, 300, "/mnt/sandbox/pfsmnt/%s-mod0", appid);
			snprintf(app_folder, 300, "/mnt/sandbox/%s_000", appid);

			int ret = -1;
			if (strstr(app_folder, path) == 0) {
				struct thread_info_t prevInfo;
				memset(&prevInfo, 0, sizeof(prevInfo));

				oni_threadEscape(td, &prevInfo);

				struct stat buff;
				if (kstat_t(pfsmnt_mod, &buff, td) == 0) {
					// Last step of folder suppresion

					// Unmount all folder still append in app0
					ret = 0;
					while (ret == 0) {
						ret = kunmount_t(app0, MNT_FORCE, td);
						WriteLog(LL_Debug, "[Unmount][%s] Unmount app0: %s => %d\n", appid, app0, ret);
					}

					// Unmount mod0 folder
					ret = kunmount_t(pfsmnt_mod, MNT_FORCE, td);
					WriteLog(LL_Debug, "[Unmount][%s] Unmount mod0: %s => %d\n", appid, pfsmnt_mod, ret);

					// Delete mod0 folder
					uap->path = pfsmnt_mod;
					ret = _rmdir(td, uap);
					WriteLog(LL_Debug, "[Rmdir][%s] Rmdir mod0: %s => %d\n", appid, uap->path, ret);
				}

				oni_threadRestore(td, &prevInfo);

				uap->path = path;
				return _rmdir(td, uap);
			}
		}
	}

	return _rmdir(td, uap);
}

int overlayfs_onExecNewVmspace(struct image_params* imgp, struct sysentvec* sv)
{
	char* (*strstr)(const char*, const char*) = kdlsym(strstr);
	void* (*memset)(void *b, int c, size_t len) = kdlsym(memset);
	int (*snprintf)(char * restrict str, size_t size, const char * restrict format, ...) = kdlsym(snprintf);
	
	// Get the framework
	struct overlayfs_t* fs = mira_getFramework()->overlayfs;

	int	(*exec_new_vmspace)(struct image_params *, struct sysentvec *) = hook_getFunctionAddress(fs->execNewVmspaceHook);
	// Call the original exec_new_vmspace
	hook_disable(fs->execNewVmspaceHook);
	int32_t ret = exec_new_vmspace(imgp, sv);
	hook_enable(fs->execNewVmspaceHook);

	// Check if eboot is present
	if (strstr(imgp->execpath, "eboot") == NULL)
	{
		WriteLog(LL_Debug, "%s not eboot", imgp->execpath);
		goto end;
	}

	// Get process information
	struct thread* td = imgp->proc->p_singlethread ? imgp->proc->p_singlethread : imgp->proc->p_threads.tqh_first;
	int32_t pid = td->td_proc->p_pid;

	// Get AppID
	uint64_t appid_add = 0x390;
	uint64_t proc_addr = (uint64_t)curthread->td_proc;	
	uint64_t r = proc_addr + appid_add;

	char* appid = (char*)r;

	WriteLog(LL_Info, "Proc addr: %p", curthread->td_proc);
	WriteLog(LL_Info, "AppID (%p): %s", appid, appid);

	// Go outside the sandbox !
	struct thread_info_t prevInfo;
	memset(&prevInfo, 0, sizeof(prevInfo));

	oni_threadEscape(td, &prevInfo);

	WriteLog(LL_Debug, "Process Thread td %p pid %d appid: %s", td, pid, appid);

	int32_t result = -1;

	// Get all needed path from appid
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

	// Check if mod folder exist
	struct stat data_stat;
	if (kstat_t(data_mod, &data_stat, td) < 0) {
		WriteLog(LL_Error, "Mod folder doesn't exist (%s). Abording ... (err: %i)", data_mod, result);
		goto end_root;
	}

	WriteLog(LL_Info, "Mod folder exist: %s", data_mod);

	// Create folder 
	overlayfs_createDirectory(td, pfsmnt_mod, 0511);

	// Doesn't work if you nullfs a unionfs
	// Doesn't work if you want make multi union in same directory

	// Unmount union
	result = kunmount_t(pfsmnt_union, MNT_FORCE, td);
	WriteLog(LL_Debug, "[%s] Unmount union: %s => %d", appid, pfsmnt_union, result);

	/*
	//Kernel panic (Interesting message before: [KERNEL] dmem_start_app_process pid: 68, map=0xffffe06b30b10258, app_maps_count[0], 0 -> 1)
	// Unmount all folder mounted in app0
	result = 0;
	while (result == 0) {
		result = kunmount_t(app0, MNT_FORCE, td);
		WriteLog(LL_Debug, "[%s] Unmount app0: %s => %d", appid, app0, result);
	}

	// Delete app0 folder
	result = krmdir_t(app0, td);
	WriteLog(LL_Debug, "[%s] Delete app0: %s => %d", appid, app0, result);

	// Re-create app0 folder
	overlayfs_createDirectory(td, app0, 0511);
	*/

	// Add app0 (?!)
	result = mount_nullfs(td, app0, pfsmnt_app, "ro", MNT_RDONLY);
	if (result < 0)
	{
		WriteLog(LL_Warn, "could not mount NullFS %s on %s ! (%d)", pfsmnt_app, app0, result);
		goto end_root;
	}

	// Mount mod0 in pfsmnt (from /user/data)
	result = mount_nullfs(td, pfsmnt_mod, data_mod, "ro", MNT_RDONLY);
	if (result < 0)
	{
		WriteLog(LL_Warn, "could not mount NullFS %s on %s ! (%d)", data_mod, pfsmnt_mod, result);
		goto end_root;
	}

	// Crash append here
	/*
	result = mount_unionfs(td, app0, pfsmnt_patch, "ro", MNT_RDONLY);
	if (result < 0)
	{
		WriteLog(LL_Warn, "could not mount with UnionFS1 ! (%d)", result);
		goto end_root;
	}
	*/

	// Union app0 and mod0
	result = mount_unionfs(td, app0, pfsmnt_mod, "ro", MNT_RDONLY);
	if (result < 0)
	{
		WriteLog(LL_Warn, "could not mount with UnionFS2 ! (%d)", result);
		goto end_root;
	}

	// Mount union with app0
	result = mount_nullfs(td, pfsmnt_union, app0, "ro", MNT_RDONLY);
	if (result < 0)
	{
		WriteLog(LL_Warn, "could not mount NullFS %s on %s ! (%d)", pfsmnt_app, app0, result);
		goto end_root;
	}

	WriteLog(LL_Info, "All mounted with success !");
	WriteLog(LL_Info, "OverlayFS done.");

end_root:
	oni_threadRestore(td, &prevInfo);

end:
	WriteLog(LL_Debug, "ExecNewVmspace called !");

	return ret;
}
