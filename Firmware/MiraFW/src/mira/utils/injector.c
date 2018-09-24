#include "injector.h"
#include <oni/utils/kdlsym.h>
#include <oni/utils/logger.h>

#include <oni/utils/kernel.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/escape.h>

#include <sys/sysproto.h>
#include <sys/proc.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/unistd.h>
#include <sys/sysent.h>

#include <machine/reg.h>

void* injector_allocateMemory(int32_t pid, uint32_t size)
{
	struct  proc* (*pfind)(pid_t) = kdlsym(pfind);
	int(*sys_mmap)(struct thread*, struct mmap_args*) = kdlsym(sys_mmap);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	// Get the proc structure from pid, this will return with the lock held
	struct proc* process = pfind(pid);
	if (!process)
		return NULL;

	int error;
	struct mmap_args uap;

	// Get the process main thread
	struct thread *td = process->p_singlethread != NULL ? process->p_singlethread : process->p_threads.tqh_first;
	
	PROC_UNLOCK(process);

	// clear errors
	td->td_retval[0] = 0;

	// call syscall
	uap.addr = NULL;
	uap.len = size;
	uap.prot = PROT_READ | PROT_WRITE | PROT_EXEC;
	uap.flags = MAP_ANON | MAP_PREFAULT_READ;
	uap.fd = -1;
	uap.pos = 0;
	error = sys_mmap(td, &uap);
	if (error)
		return (caddr_t)(int64_t)-error;

	// return
	return (void*)td->td_retval[0];
}

uint8_t injector_createUserProcess(uint8_t* moduleData, uint32_t moduleSize)
{
	struct vmspace* (*vmspace_alloc)(vm_offset_t min, vm_offset_t max) = kdlsym(vmspace_alloc);
	void(*pmap_activate)(struct thread *td) = kdlsym(pmap_activate);
	struct sysentvec* sv = kdlsym(self_orbis_sysvec);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	struct  proc* (*pfind)(pid_t) = kdlsym(pfind);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);


	WriteLog(LL_Debug, "finding syscore");

	// Get syscore process
	struct proc* syscoreProc = proc_find_by_name("SceSysCore.elf");
	if (!syscoreProc)
	{
		WriteLog(LL_Error, "could not find syscore");
		return false;
	}

	WriteLog(LL_Debug, "syscore process: %p", syscoreProc);

	// Get the main thread
	struct thread* td = syscoreProc->p_singlethread ? syscoreProc->p_singlethread : syscoreProc->p_threads.tqh_first;
	if (!td)
	{
		WriteLog(LL_Error, "could not get syscore thread");
		return false;
	}

	WriteLog(LL_Debug, "syscore main thread: %p", td);

	// Fork syscore into it's own process
	pid_t newPid = krfork_t(RFPROC | RFCFDG, td);
	WriteLog(LL_Debug, "forked syscore from pid (%d) to pid (%d)", syscoreProc->p_pid, newPid);
	if (newPid <= 0)
	{
		WriteLog(LL_Error, "could not fork syscore (%d).", newPid);
		return false;
	}

	// Freeze the newly created process
	kkill(newPid, SIGSTOP);

	// Get our new proc's structure, the process is returned LOCKED
	struct proc* newProc = pfind(newPid);
	if (!newProc)
	{
		kkill(newPid, SIGTERM);
		return false;
	}

	WriteLog(LL_Debug, "new forked process: %p", newProc);

	struct thread* newThread = newProc->p_singlethread ? newProc->p_singlethread : newProc->p_threads.tqh_first;
	if (!newThread)
	{
		PROC_UNLOCK(newProc);
		WriteLog(LL_Error, "getting new proc's thread failed\n");
		kkill(newPid, SIGTERM);
		return false;
	}

	WriteLog(LL_Debug, "new forked main thread: %p", newThread);
	// Create a new vmspace for our process
	vm_offset_t sv_minuser = MAX(sv->sv_minuser, PAGE_SIZE);
	struct vmspace* vmspace = vmspace_alloc(sv_minuser, sv->sv_maxuser);
	if (!vmspace)
	{
		PROC_UNLOCK(newProc);
		WriteLog(LL_Error, "vmspace_alloc failed\n");
		kkill(newPid, SIGTERM);
		return false;
	}

	WriteLog(LL_Debug, "allocated new vmspace: %p", vmspace);

	// Assign our new vmspace to our process
	newProc->p_vmspace = vmspace;
	pmap_activate(newThread);

	PROC_UNLOCK(newProc);

	uint8_t* procModuleData = injector_allocateMemory(newPid, moduleSize);
	if (!procModuleData)
	{
		WriteLog(LL_Error, "could not allocate memory inside new process.");
		PROC_UNLOCK(newProc);
		kkill(newPid, SIGTERM);
		return false;
	}

	WriteLog(LL_Debug, "proc module data: %p", procModuleData);

	// Write our module
	size_t bytesWritten = moduleSize;
	int ret = proc_rw_mem(newProc, procModuleData, moduleSize, moduleData, &bytesWritten, true);
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not write module (%d).", ret);
		kkill(newPid, SIGTERM);
		return false;
	}

	WriteLog(LL_Debug, "writing module (%p : %llx) data to %p in new proc returned (%d).", moduleData, moduleSize, ret);

	struct thread_info_t threadInfo;
	memset(&threadInfo, 0, sizeof(threadInfo));

	oni_threadEscape(curthread, &threadInfo);

	// Attach to the process
	ret = kptrace(PT_ATTACH, newPid, NULL, 0);
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not trace process (%d).", ret);
		kkill(newPid, SIGTERM);
		return false;
	}

	WriteLog(LL_Debug, "attaching to new process returned (%d).", ret);
	// Create a new credential
	struct thread* iterThread = NULL;
	FOREACH_THREAD_IN_PROC(newProc, iterThread)
	{
		if (!iterThread)
			continue;

		ksetuid_t(0, iterThread);

		WriteLog(LL_Debug, "creating new credential for thread: %p");

		oni_threadEscape(iterThread, NULL);
	}

	// TODO: Add ELF parsing

	struct reg registers;
	memset(&registers, 0, sizeof(registers));

	// TODO: Get and save registers
	ret = kptrace(PT_GETREGS, newPid, (caddr_t)&registers, 0);
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not get registers (%d).", ret);
		kkill(newPid, SIGTERM);
		return false;
	}

	WriteLog(LL_Debug, "saved registers, rip: %p", registers.r_rip);

	// Set the new instruction pointer, currently assuming that the first instruction is jmp <entrypoint>
	registers.r_rip = (register_t)procModuleData;

	// Set the registers
	ret = kptrace(PT_SETREGS, newPid, (caddr_t)&registers, 0);
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not set registers (%d).", ret);
		kkill(newPid, SIGTERM);
		return false;
	}

	WriteLog(LL_Debug, "writing new rip returned (%d)", ret);

	// Resume the process
	kkill(newPid, SIGCONT);


	return true;
}

uint8_t injector_injectModule(int32_t pid, uint8_t* moduleData, uint32_t moduleSize, uint8_t newThread)
{
	//void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	//void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);
	struct  proc* (*pfind)(pid_t) = kdlsym(pfind);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	struct proc* syscoreProc = proc_find_by_name("SceSysCore.elf");
	if (!syscoreProc)
	{
		WriteLog(LL_Debug, "syscoreproc: %p", syscoreProc);
		return false;
	}

	// Set the pid
	pid = syscoreProc->p_pid;

	if (!moduleData || moduleSize == 0)
		return false;

	if (pid <= 0)
		return false;

	// First allocate memory using mmap inside of the target process
	size_t processMemorySize = (PAGE_SIZE - (moduleSize % PAGE_SIZE));

	WriteLog(LL_Debug, "attempting to allocate %llx for %llx", processMemorySize, moduleSize);

	uint8_t* processMemory = injector_allocateMemory(pid, processMemorySize);
	if (!processMemory)
		return false;

	WriteLog(LL_Debug, "allocated process memory at %p", processMemory);

	size_t stackMemorySize = 0x80000;
	uint8_t* stackMemory = injector_allocateMemory(pid, stackMemorySize);
	if (!stackMemory)
		return false;

	WriteLog(LL_Debug, "allocated stack memory at %p", stackMemory);


	// TODO: Write the module data into the new target process space
	size_t bytesWritten = 0;

	// Write the module data
	int32_t result = proc_rw_mem_pid(pid, processMemory, moduleSize, moduleData, &bytesWritten, true);
	if (result < 0)
		return false;

	WriteLog(LL_Debug, "returned: %d, wrote %lld bytes of module %p to %p", result,  bytesWritten, moduleData, processMemory);

	// Pause the target process
	result = kkill(pid, SIGSTOP);
	if (result != 0)
	{
		WriteLog(LL_Error, "could not stop process for module injection");
		return false;
	}

	// Get the process for our target process
	struct proc* targetProc = pfind(pid);
	if (!targetProc)
	{
		WriteLog(LL_Error, "dsadas");
		return false;
	}

	// Acquire the main thread
	struct thread* targetMainThread = TAILQ_FIRST(&targetProc->p_threads);

	PROC_UNLOCK(targetProc);

	

	// Create a new thread using the main threads process
	int(*create_thread)(struct thread * td, uint64_t ctx, void(*start_func)(void *), void *arg, char *stack_base, uint64_t stack_size, char *tls_base, long *child_tid, long *parent_tid, uint64_t flags, uint64_t rtp) = kdlsym(kern_thr_create);

	long threadId = 0;
	result = create_thread(targetMainThread, 0, (void(*)(void*))processMemory, NULL, (char*)stackMemory, stackMemorySize, NULL, NULL, NULL, 0, 0);
	if (result < 0)
	{
		WriteLog(LL_Error, "could not create new thread in target process (%d).", result);
		return false;
	}

	// Resume the process
	kkill(pid, SIGCONT);

	WriteLog(LL_Debug, "thread created id: %p (%d)", threadId, result);

	return true;
}