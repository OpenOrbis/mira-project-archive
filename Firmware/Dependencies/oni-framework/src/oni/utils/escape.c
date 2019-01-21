#include <oni/utils/escape.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/memory/allocator.h>

#include <sys/proc.h>

#define AUTHID_DIAGNOSTICS	0x3800000000000007ULL

#define SCECAPS_MAX			0xFFFFFFFFFFFFFFFFULL

void oni_threadEscape(struct thread* td, struct thread_info_t* outThreadInfo)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);

	if (!td)
		return;

	if (outThreadInfo)
	{
		// Zero out the previous thread information
		memset(outThreadInfo, 0, sizeof(*outThreadInfo));

		// Ensure that the cred sizes are the same (ideally they should be the same type)
		if (sizeof(outThreadInfo->cred) != sizeof(*td->td_ucred))
			return;

		// Copy the ucred structure
		memcpy(&outThreadInfo->cred, td->td_ucred, sizeof(outThreadInfo->cred));

		// Ensure that the filedesc sizes are the same (ideally they should be the same type)
		if (sizeof(outThreadInfo->desc) != sizeof(*td->td_proc->p_fd))
			return;

		memcpy(&outThreadInfo->desc, td->td_proc->p_fd, sizeof(outThreadInfo->desc));
	}

	// Create new cred if we don't already have one
	ksetuid(0);

	// Set our group id to root
	td->td_ucred->cr_rgid = 0;
	td->td_ucred->cr_svgid = 0;

	// Set our user id to root
	td->td_ucred->cr_uid = 0;
	td->td_ucred->cr_ruid = 0;

	// Root group
	td->td_ucred->cr_groups[0] = 0;

	// No prison
	td->td_ucred->cr_prison = *(void**)kdlsym(prison0);

	// Get the file descriptor
	struct filedesc* fd = td->td_proc->p_fd;

	// Set our file descriptor, and jail file descriptor to the root vnode '/'
	fd->fd_rdir = fd->fd_jdir = *(void**)kdlsym(rootvnode);

	// set diag auth ID flags
	td->td_ucred->cr_sceAuthID = AUTHID_DIAGNOSTICS;

	// make system credentials
	td->td_ucred->cr_sceCaps[0] = SCECAPS_MAX;
	td->td_ucred->cr_sceCaps[1] = SCECAPS_MAX;
}

void oni_threadRestore(struct thread* td, struct thread_info_t* threadInfo)
{
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);

	if (!td || !threadInfo)
		return;

	if (!td->td_ucred)
		return;

	// Copy the cred back over
	memcpy(td->td_ucred, &threadInfo->cred, sizeof(*td->td_ucred));

	// Get the file descriptor
	struct filedesc* fd = td->td_proc->p_fd;

	if (!fd)
		return;

	memcpy(fd, &threadInfo->desc, sizeof(*fd));
}