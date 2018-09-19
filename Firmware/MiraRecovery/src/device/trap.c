#include "trap.h"
#include <sys/param.h>
#include <sys/proc.h>
#include <machine/frame.h>

#include <oni/utils/logger.h>
#include <oni/utils/hook.h>

struct hook_t* gTrapQueen = NULL;
uint8_t gTrappin = FALSE;

void trap_fatal_hook(struct trapframe* frame, vm_offset_t eva)
{
	if (!frame)
		return;

	void(*trap_fatal)(struct trapframe*, vm_offset_t) = hook_getFunctionAddress(gTrapQueen);

	void* stackPointer = ((uint8_t*)frame) - 0xA8;
	
	struct proc* process = curproc;
	struct thread* thread = curthread;
	//void* initproc = NULL;

	if (!process || !thread)
		return;

	WriteLog(LL_Error, "kernel process crashed: (%s:%s):RIP: %p, RSP:%p, EVA: %p", process->p_elfpath, thread->td_name, frame->tf_rip, stackPointer, eva);

	//// Kill the process
	//sx_xlock(&proctree_lock);
	//PROC_LOCK(process);
	//proc_reparent(process, initproc);
	//PROC_UNLOCK(process);
	//sx_xunlock(&proctree_lock);

	//wakeup(process);

	//exit1(thread, 0);

	// Check if the original trap is enabled
	if (gTrappin)
	{
		hook_disable(gTrapQueen);
		trap_fatal(frame, eva);
		hook_disable(gTrapQueen);
	}
}