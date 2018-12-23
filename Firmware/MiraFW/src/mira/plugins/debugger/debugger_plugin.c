#include "debugger_plugin.h"
#include <oni/framework.h>
#include <oni/utils/kdlsym.h>
#include <oni/messaging/messagemanager.h>
#include <oni/utils/logger.h>
#include <oni/utils/hook.h>
#include <oni/utils/kernel.h>

#include <oni/utils/hde/hde64.h>
#include <sys/proc.h>
#include <sys/ptrace.h>

#include <oni/utils/sys_wrappers.h>

#include <oni/init/initparams.h>

#include <mira/miraframework.h>

#include <protobuf-c/mirabuiltin.pb-c.h>


enum DebuggerCmds
{
	DbgCmd_GetProcesses = 0x97077E04,
	DbgCmd_ReadMemory = 0xA0C48DE9,
	DbgCmd_WriteMemory = 0x2B3587A6,
	DbgCmd_Ptrace = 0x6DAC0B97,
	DbgCmd_Kill = 0x020C3897,
	DbgCmd_GetThreads = 0x0,
};

// Credits: flatz
int proc_rw_mem(struct proc* p, void* ptr, size_t size, void* data, size_t* n, int write);

static struct hook_t* gTrapFatalHook = NULL;

uint8_t debugger_load(struct debugger_plugin_t * plugin)
{
	if (!plugin)
		return false;

	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__DEBUG, DbgCmd_GetProcesses, debugger_getprocs_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__DEBUG, DbgCmd_ReadMemory, debugger_readmem_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__DEBUG, DbgCmd_WriteMemory, debugger_writemem_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__DEBUG, DbgCmd_Ptrace, debugger_ptrace_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__DEBUG, DbgCmd_Kill, debugger_kill_callback);

	//hook_enable(plugin->trapFatalHook);

	return true;
}

uint8_t debugger_unload(struct debugger_plugin_t * plugin)
{
	if (!plugin)
		return false;

	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__DEBUG, DbgCmd_GetProcesses, debugger_getprocs_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__DEBUG, DbgCmd_ReadMemory, debugger_readmem_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__DEBUG, DbgCmd_WriteMemory, debugger_writemem_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__DEBUG, DbgCmd_Ptrace, debugger_ptrace_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__DEBUG, DbgCmd_Kill, debugger_kill_callback);

	hook_disable(plugin->trapFatalHook);

	return true;
}

void debugger_plugin_init(struct debugger_plugin_t* plugin)
{
	if (!plugin)
		return;

	plugin->plugin.name = "Debugger";
	plugin->plugin.description = "Kernel mode debugger";

	plugin->plugin.plugin_load = (uint8_t(*)(void*)) debugger_load;
	plugin->plugin.plugin_unload = (uint8_t(*)(void*)) debugger_unload;

	// Create the lock
	void(*mtx_init)(struct mtx *m, const char *name, const char *type, int opts) = kdlsym(mtx_init);
	mtx_init(&plugin->lock, "miradbg", NULL, 0);

	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	// Zero out the breakpoints list
	memset(plugin->breakpoints, 0, sizeof(plugin->breakpoints));

	// Zero out the threads list
	memset(plugin->threads, 0, sizeof(plugin->threads));

	// Zero out the registers list
	memset(&plugin->registers, 0, sizeof(plugin->registers));
	memset(&plugin->floatingRegisters, 0, sizeof(plugin->floatingRegisters));
	memset(&plugin->debugRegisters, 0, sizeof(plugin->debugRegisters));

	plugin->trapFatalHook = hook_create(kdlsym(trap_fatal), debugger_onTrapFatal);
	gTrapFatalHook = plugin->trapFatalHook;
}

void debugger_onTrapFatal(struct trapframe* frame, vm_offset_t eva)
{
	if (!frame)
		goto hang_thread;

	if (!curthread)
	{
		WriteLog(LL_Error, "could not get thread context");
		goto hang_thread;
	}

	void* rsp = ((uint8_t*)frame - 0xA8);
	/*
		pop rbp;
		leave;
		ret;
	*/

	
	char* dash = "-----------------------";

	WriteLog(LL_Info, "kernel panic detected");
	
	// Print extra information in case that Oni/Mira itself crashes
	if (gInitParams && curthread->td_proc == gInitParams->process)
	{
		WriteLog(LL_Info, dash);
		WriteLog(LL_Info, "mira base: %p size: %p", gInitParams->payloadBase, gInitParams->payloadSize);
		WriteLog(LL_Info, "mira proc: %p entrypoint: %p", gInitParams->process);

		if (gFramework)
			WriteLog(LL_Info, "mira messageManager: %p pluginManager: %p rpcServer: %p", gFramework->messageManager, gFramework->pluginManager, gFramework->rpcServer);
	}

	WriteLog(LL_Info, dash);
	WriteLog(LL_Info, "gKernelBase: %p", gKernelBase);
	WriteLog(LL_Info, "OffsetFromMiraBase: %p", 0);
	WriteLog(LL_Info, "OffsetFromKernelBase: %p", 0);

	WriteLog(LL_Info, "thread: %p proc: %p pid: %d path: %s", curthread, curthread->td_proc, curthread->td_proc->p_pid, curthread->td_proc->p_elfpath);
	WriteLog(LL_Info, "eva: %p", eva);
	WriteLog(LL_Info, "rdi: %p", frame->tf_rdi);
	WriteLog(LL_Info, "rsi: %p", frame->tf_rsi);
	WriteLog(LL_Info, "rdx: %p", frame->tf_rdx);
	WriteLog(LL_Info, "rcx: %p", frame->tf_rcx);
	WriteLog(LL_Info, "r8: %p", frame->tf_r8);
	WriteLog(LL_Info, "r9: %p", frame->tf_r9);
	WriteLog(LL_Info, "rax: %p", frame->tf_rax);
	WriteLog(LL_Info, "rbx: %p", frame->tf_rbx);
	WriteLog(LL_Info, "rbp: %p", frame->tf_rbp);
	WriteLog(LL_Info, "r10: %p", frame->tf_r10);
	WriteLog(LL_Info, "r11: %p", frame->tf_r11);
	WriteLog(LL_Info, "r12: %p", frame->tf_r12);
	WriteLog(LL_Info, "r13: %p", frame->tf_r13);
	WriteLog(LL_Info, "r14: %p", frame->tf_r14);
	WriteLog(LL_Info, "r15: %p", frame->tf_r15);
	WriteLog(LL_Info, "trapno: %u", frame->tf_trapno);
	WriteLog(LL_Info, "fs: %u", frame->tf_fs);
	WriteLog(LL_Info, "gs: %u", frame->tf_gs);
	WriteLog(LL_Info, "addr: %p", frame->tf_addr);
	WriteLog(LL_Info, "flags: %u", frame->tf_flags);
	WriteLog(LL_Info, "es: %u", frame->tf_es);
	WriteLog(LL_Info, "ds: %u", frame->tf_ds);
	WriteLog(LL_Info, "err: %p", frame->tf_err);
	WriteLog(LL_Info, "rip: %p", frame->tf_rip);
	WriteLog(LL_Info, "cs: %p", frame->tf_cs);
	WriteLog(LL_Info, "rflags: %p", frame->tf_rflags);
	WriteLog(LL_Info, "rsp adjusted: %p rsp: %p", rsp, frame->tf_rsp);
	WriteLog(LL_Info, "err: %p", frame->tf_err);
	WriteLog(LL_Info, dash);

	// If the kernel itself crashes, we don't want this to be debuggable, otherwise the entire console hangs
	if (curthread->td_proc->p_pid == 0)
	{
		// See if we have a trap fatal reference, if not we just hang and let the console die
		if (!gTrapFatalHook)
			goto hang_thread;

		// Get the original hook
		void(*onTrapFatal)(struct trapframe* frame, vm_offset_t eva) = hook_getFunctionAddress(gTrapFatalHook);

		// Disable the hook
		hook_disable(gTrapFatalHook);

		// Call original sce trap fatal
		onTrapFatal(frame, eva);
		return;
	}

	// Intentionally hang the thread
hang_thread:
	for (;;)
		__asm__("nop");

	// Allow the debugger to be placed here manually and continue exceution
	__asm__("pop %rbp;leave;ret;");
}


int32_t debugger_getDisassemblyMinLength(struct debugger_plugin_t* plugin, void* address, size_t length)
{
	if (!plugin)
		return -1;

	if (!address)
		return -1;

	if (length <= 0)
		return -1;

	//void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	struct  proc* (*pfind)(pid_t) = kdlsym(pfind);

	hde64s hs;

	uint32_t hookSize = length;
	uint32_t totalLength = 0;

	// Check if this is a kernel address
	if ((uint64_t)address & 0x8000000000000000ULL)
	{
		while (totalLength < hookSize)
		{
			uint32_t length = hde64_disasm(address, &hs);
			if (hs.flags & F_ERROR)
				return -1;

			totalLength += length;
		}

		return totalLength;
	}
	else // userland address
	{
		size_t bytesWritten = 0;
		uint8_t buffer[64];
		memset(buffer, 0, sizeof(buffer));

		struct proc* process = pfind(plugin->pid);

		int result = proc_rw_mem(process, address, sizeof(buffer), buffer, &bytesWritten, false);
		
		PROC_UNLOCK(process);

		if (!result)
		{
			WriteLog(LL_Warn, "could not read process memory", result, address);
			return -1;
		}

		while (totalLength < hookSize)
		{
			// This is bad, fix this later
			if (totalLength > sizeof(buffer))
				return -1;

			uint32_t length = hde64_disasm(buffer + totalLength, &hs);
			if (hs.flags & F_ERROR)
				return -1;

			totalLength += length;
		}

		return totalLength;
	}
}

uint8_t debugger_continue(struct debugger_plugin_t* plugin)
{
	if (!plugin)
		return false;

	if (plugin->pid < 0)
		return false;

	int32_t ret = kkill(plugin->pid, SIGCONT);

	return ret == 0;
}

uint8_t debugger_pause(struct debugger_plugin_t* plugin)
{
	if (!plugin)
		return false;

	if (plugin->pid < 0)
		return false;

	// returns -1 on error
	int32_t ret = kkill(plugin->pid, SIGSTOP);

	return ret == 0;
}

uint8_t debugger_attach(struct debugger_plugin_t* plugin, int32_t pid)
{
	if (!plugin)
		return false;

	// Check that a valid pid was passed in
	if (pid < 0)
		return false;

	// Deny re-attaching to the same pid
	if (plugin->pid == pid)
		return false;

	int32_t ret = kptrace(PT_ATTACH, pid, NULL, 0);
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not attach to pid %d (%d).", pid, ret);
		return false;
	}

	// Assign our pid
	plugin->pid = pid;
	return true;
}


uint8_t debugger_detach(struct debugger_plugin_t* plugin)
{
	void(*proc_reparent)(struct proc* child, struct proc* parent) = kdlsym(proc_reparent);
	struct  proc* (*pfind)(pid_t) = kdlsym(pfind);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	struct proc* proc0 = (struct proc*)kdlsym(proc0);
	//struct proc* sysCoreProc = NULL;

	if (!plugin)
		return false;

	if (plugin->pid < 0)
		return false;

	
	struct proc* proc = pfind(plugin->pid);
	if (proc)
	{
		struct ucred* cred = proc->p_ucred;
		
		// If we have a proc, then we have to check what kind of process it was
		switch (cred->cr_sceAuthID)
		{
			// SceShellCore
		case 0x3800000000000010:
			break;
		}
	}

	int result = -1;
	struct ucred* cred = proc ? proc->p_ucred : NULL;
	if (!cred)
		goto cleanup;

	// TODO: Fix
	// FIXME: Properly detach and reparent
	result = kptrace(PT_DETACH, plugin->pid, NULL, 0);
	if (result < 0)
		WriteLog(LL_Warn, "could not ptrace detach (%d).", result);


	
	// Check if this was shellcore
	if (cred->cr_sceAuthID == 0x3800000000000010)
	{
		// Reparent ShellCore to proc0
		proc_reparent(proc, proc0);
		result = true;
	}	
	else if (true)
	{
		// Check if this is a userland process, if so
		// reparent to shellcore or syscore?
		result = true;
	}
	else if (false)
	{
		// Check if this is a kernel process, uh idk?
		result = true;
	}
	
	plugin->pid = -1;

cleanup:
	PROC_UNLOCK(proc);
	return result;
}

void debugger_updateThreads(struct debugger_plugin_t* plugin)
{
	if (!plugin)
		return;

	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	struct  proc* (*pfind)(pid_t) = kdlsym(pfind);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);

	struct proc* lockedProc = pfind(plugin->pid);
	if (!lockedProc)
		return;

	_mtx_lock_flags(&plugin->lock, MTX_QUIET, __FILE__, __LINE__);

	// Zero out our threads
	memset(plugin->threads, 0, sizeof(plugin->threads));

	// Hold our current index
	uint32_t threadIndex = 0;

	struct thread* thread = NULL;
	FOREACH_THREAD_IN_PROC(lockedProc, thread)
	{
		if (threadIndex >= ARRAYSIZE(plugin->threads))
		{
			WriteLog(LL_Error, "too many threads, expand maximum thread count");
			break;
		}

		struct thread_t* threadObject = &plugin->threads[threadIndex];
		threadObject->address = thread;
		threadObject->errorno = thread->td_errno;
		threadObject->stack = (void*)thread->td_kstack;
		threadObject->stackSizeInPage = thread->td_kstack_pages;
		
		if (sizeof(threadObject->frame) != sizeof(thread->td_frame))
			WriteLog(LL_Warn, "buffer overrun possible, stack frames are different sizes");

		// Copy over the frame
		memcpy(&threadObject->frame, thread->td_frame, sizeof(thread->td_frame));

		threadObject->cpuId = thread->td_oncpu;

		struct ucred* credentials = thread->td_ucred;
		if (credentials)
		{
			threadObject->effectiveUserId = (int32_t)credentials->cr_uid;
			threadObject->realUserId = (int32_t)credentials->cr_ruid;
			threadObject->savedUserId = (int32_t)credentials->cr_svuid;

			threadObject->realGroupId = (int32_t)credentials->cr_rgid;
			threadObject->savedGroupId = (int32_t)credentials->cr_svgid;

			threadObject->prison = credentials->cr_prison;
			threadObject->authId = credentials->cr_sceAuthID;
		}
		else
		{
			// If there is no cred, set all -1's
			threadObject->effectiveUserId = -1;
			threadObject->realUserId = -1;
			threadObject->savedUserId = -1;

			threadObject->realGroupId = -1;
			threadObject->savedGroupId = -1;

			threadObject->prison = NULL;
			threadObject->authId = 0x0BADF00D0BADF00D;
		}

		threadObject->debuggerFlags = thread->td_dbgflags;
		threadObject->debuggerChildPid = thread->td_dbg_forked;
		threadObject->currentSignalMask = 0x0BADF00D0BADF00D; /**(uint32_t*)&thread->td_sigmask;*/

		threadIndex++;
	}

	PROC_UNLOCK(lockedProc);

	_mtx_unlock_flags(&plugin->lock, MTX_QUIET, __FILE__, __LINE__);

	WriteLog(LL_Info, "updated threads");
}

void debugger_update(struct debugger_plugin_t* plugin)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	if (!plugin)
		return;

	//struct proc* process = pfind(plugin->pid);
	//if (process == NULL)
	//{
	//	debugger_detach(plugin);
	//}
	//PROC_UNLOCK(process);
	// TODO: Check the current state before invoking the update functions
	// TODO: A process will need to be in the attached + paused state in order for this to work

	debugger_updateSegments(plugin);
	debugger_updateThreads(plugin);
	debugger_updateBreakpoints(plugin);

	memset(&plugin->registers, 0, sizeof(plugin->registers));
	memset(&plugin->debugRegisters, 0, sizeof(plugin->debugRegisters));
	memset(&plugin->floatingRegisters, 0, sizeof(plugin->floatingRegisters));

	int32_t res = kptrace(PT_GETREGS, plugin->pid, (caddr_t)&plugin->registers, 0);
	if (res < 0)
	{
		WriteLog(LL_Error, "could not get registers (%d).", res);
		return;
	}

	res = kptrace(PT_GETFPREGS, plugin->pid, (caddr_t)&plugin->floatingRegisters, 0);
	if (res < 0)
	{
		WriteLog(LL_Error, "could not get floating point registers (%d).", res);
		return;
	}

	res = kptrace(PT_GETDBREGS, plugin->pid, (caddr_t)&plugin->debugRegisters, 0);
	if (res < 0)
	{
		WriteLog(LL_Error, "could not get debug registers (%d).", res);
		return;
	}

	WriteLog(LL_Debug, "debugger updated!");
}