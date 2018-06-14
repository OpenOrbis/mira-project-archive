#include <oni/utils/kdlsym.h>
#include <oni/utils/logger.h>

#include <sys/proc.h>
#include <sys/filedesc.h>

#if ONI_PLATFORM==ONI_PLATFORM_ORBIS_BSD_501
#define kdlsym_addr_dmem_start_app_process 0x002468E0
#define kdlsym_addr_jailed 0x003ABC10
#elif ONI_PLATFORM==ONI_PLATFORM_ORBIS_BSD_505
#define kdlsym_addr_jailed 0x003ABFE0
#define kdlsym_addr_dmem_start_app_process 0x002469F0
#endif

int sys_jailbreak(struct thread* td, register void* unused)
{
	if (!td)
		return -1;

	struct prison** prison0 = kdlsym(prison0);
	struct vnode** rootvnode = kdlsym(rootvnode);


	struct ucred* cred = td->td_ucred;

	struct proc* process = td->td_proc;

	// Root ourselves
	cred->cr_uid = 0;
	cred->cr_ruid = 0;
	cred->cr_rgid = 0;
	cred->cr_groups[0] = 0;

	// Escape the prison
	cred->cr_prison = prison0[0];

	// Set us to diagnostics id
	cred->cr_sceAuthID = 0x3800000000000007ULL;

	// Maximum privs
	cred->cr_sceCaps[0] = 0xffffffffffffffff;
	cred->cr_sceCaps[1] = 0xffffffffffffffff;

	// Get the file descriptor
	struct filedesc* fileDescriptor = process->p_fd;
	if (!fileDescriptor)
	{
		WriteLog(LL_Error, "could not get proc file descriptor");
		return -1;
	}

	// Set our root nodes
	fileDescriptor->fd_rdir = rootvnode[0];
	fileDescriptor->fd_jdir = rootvnode[0];

	WriteLog(LL_Info, "un-sandboxed and un-jailed pid: %d", process->p_pid);

	return 0;
}