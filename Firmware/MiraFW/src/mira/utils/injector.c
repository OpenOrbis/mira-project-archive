#include "injector.h"
#include <oni/utils/kdlsym.h>
#include <oni/utils/logger.h>

#include <oni/utils/kernel.h>
#include <oni/utils/sys_wrappers.h>

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


	// Get syscore process
	struct proc* syscoreProc = proc_find_by_name("SceSysCore");
	if (!syscoreProc)
	{
		WriteLog(LL_Error, "could not find syscore");
		return false;
	}

	// Get the main thread
	struct thread* td = syscoreProc->p_singlethread ? syscoreProc->p_singlethread : syscoreProc->p_threads.tqh_first;
	if (!td)
	{
		WriteLog(LL_Error, "could not get syscore thread");
		return false;
	}

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

	struct thread* newThread = newProc->p_singlethread ? newProc->p_singlethread : newProc->p_threads.tqh_first;
	if (!newThread)
	{
		PROC_UNLOCK(newProc);
		WriteLog(LL_Error, "getting new proc's thread failed\n");
		kkill(newPid, SIGTERM);
		return false;
	}

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

	// Write our module
	size_t bytesWritten = moduleSize;
	int ret = proc_rw_mem(newProc, procModuleData, moduleSize, moduleData, &bytesWritten, true);
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not write module (%d).", ret);
		kkill(newPid, SIGTERM);
		return false;
	}

	// Attach to the process
	ret = kptrace(PT_ATTACH, newPid, NULL, 0);
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not trace process (%d).", ret);
		kkill(newPid, SIGTERM);
		return false;
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

	// Resume the process
	kkill(newPid, SIGCONT);

	return true;
}

uint8_t injector_injectModule(int32_t pid, uint8_t* moduleData, uint32_t moduleSize, uint8_t newThread)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);

	if (!moduleData || moduleSize == 0)
		return false;

	if (pid <= 0)
		return false;

	// First allocate memory using mmap inside of the target process
	uint8_t* processMemory = injector_allocateMemory(pid, moduleSize);
	if (!processMemory)
		return false;

	WriteLog(LL_Debug, "allocated process memory at %p", processMemory);

	// TODO: Write the module data into the new target process space
	size_t bytesWritten = 0;

	// Write the module data
	int32_t result = proc_rw_mem_pid(pid, processMemory, moduleSize, moduleData, &bytesWritten, true);
	if (result < 0)
		return false;

	WriteLog(LL_Debug, "returned: %d, wrote %lld bytes of module %p to %p", result,  bytesWritten, moduleData, processMemory);

	// TODO: Set module permissions to RWX

	// TODO: Trace the application and pause it
	result = kkill(pid, SIGSTOP);
	if (result != 0)
	{
		WriteLog(LL_Error, "could not stop process for module injection");
		return false;
	}

	// I'm not 100% sure that this will always succeed if you are injecting multiple modules
	result = kptrace(PT_ATTACH, pid, NULL, 0);
	WriteLog(LL_Debug, "ptrace attach returned %d", result);

	// Hold the previous registers
	struct reg registers;
	struct fpreg fpRegisters;
	memset(&registers, 0, sizeof(registers));
	memset(&fpRegisters, 0, sizeof(fpRegisters));

	// TODO: Get and save registers
	result = kptrace(PT_GETREGS, pid, (caddr_t)&registers, 0);
	if (result < 0)
	{
		WriteLog(LL_Error, "could not get registers");
		return false;
	}

	result = kptrace(PT_GETFPREGS, pid, (caddr_t)&fpRegisters, 0);
	if (result < 0)
	{
		WriteLog(LL_Error, "could not save floating point registers");
		return false;
	}

	// TODO: CHANGE THIS
	// Get the entry point which is the entry point (64 bit int at the beginning of the module data)
	void* entryPoint = processMemory + (*(uint64_t*)moduleData);

	// Create a copy of the existing registers
	struct reg hijackedRegs;
	memset(&hijackedRegs, 0, sizeof(hijackedRegs));
	memcpy(&hijackedRegs, &registers, sizeof(hijackedRegs));

	// Set the new instruction pointer
	hijackedRegs.r_rip = (register_t)entryPoint;

	// Set the registers
	result = kptrace(PT_SETREGS, pid, (caddr_t)&hijackedRegs, 0);
	if (result < 0)
	{
		WriteLog(LL_Error, "could not set registers");
		return false;
	}

	// TODO: Set registers, and call our create thread stub

	// TODO: Set registers back, and continue execution

	// NOTE: Leave the process traced, or bad things will happen

	return true;
}