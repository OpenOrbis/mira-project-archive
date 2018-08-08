#pragma once
#include <oni/utils/types.h>

#include <sys/ucred.h>
#include <sys/filedesc.h>

struct thread_info_t
{
	struct ucred cred;
	struct filedesc desc;
};

void mira_threadEscape(struct thread* td, struct thread_info_t* outThreadInfo);
void mira_threadRestore(struct thread* td, struct thread_info_t* threadInfo);