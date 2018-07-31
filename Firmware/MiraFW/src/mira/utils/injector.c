#include "injector.h"
#include <oni/utils/kdlsym.h>
#include <oni/utils/logger.h>

#include <oni/utils/kernel.h>
#include <oni/utils/sys_wrappers.h>

#include <sys/sysproto.h>
#include <sys/proc.h>
#include <sys/mman.h>
#include <sys/ptrace.h>

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
	return (caddr_t)td->td_retval[0];
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