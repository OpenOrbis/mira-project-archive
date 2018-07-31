#include "debugger_plugin.h"
#include <oni/framework.h>
#include <oni/utils/kdlsym.h>
#include <oni/messaging/message.h>
#include <oni/messaging/messagemanager.h>
#include <oni/utils/logger.h>

#include <oni/utils/hde/hde64.h>
#include <sys/proc.h>

#include <oni/utils/sys_wrappers.h>

enum DebuggerCmds
{
	DbgCmd_GetProcesses = 0x97077E04,
	DbgCmd_ReadMemory = 0xA0C48DE9,
	DbgCmd_WriteMemory = 0x2B3587A6,
	DbgCmd_Ptrace = 0x6DAC0B97,
	DbgCmd_Kill = 0x020C3897,
};

// Credits: flatz
int proc_rw_mem(struct proc* p, void* ptr, size_t size, void* data, size_t* n, int write);


uint8_t debugger_load(struct debugger_plugin_t * plugin)
{
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_DBG, DbgCmd_GetProcesses, debugger_getprocs_callback);
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_DBG, DbgCmd_ReadMemory, debugger_readmem_callback);
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_DBG, DbgCmd_WriteMemory, debugger_writemem_callback);
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_DBG, DbgCmd_Ptrace, debugger_ptrace_callback);
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_DBG, DbgCmd_Kill, debugger_kill_callback);

	return true;
}

uint8_t debugger_unload(struct debugger_plugin_t * plugin)
{
	messagemanager_unregisterCallback(gFramework->messageManager, RPCCAT_DBG, DbgCmd_GetProcesses, debugger_getprocs_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, RPCCAT_DBG, DbgCmd_ReadMemory, debugger_readmem_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, RPCCAT_DBG, DbgCmd_WriteMemory, debugger_writemem_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, RPCCAT_DBG, DbgCmd_Ptrace, debugger_ptrace_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, RPCCAT_DBG, DbgCmd_Kill, debugger_kill_callback);

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
}


int32_t debugger_getDisassemblyMinLength(struct debugger_plugin_t* plugin, void* address, size_t length)
{
	if (!plugin)
		return -1;

	if (!address)
		return -1;

	if (length <= 0)
		return -1;

	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

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

		PROC_LOCK(plugin->process);
		int result = proc_rw_mem(plugin->process, address, sizeof(buffer), buffer, &bytesWritten, false);
		PROC_UNLOCK(plugin->process);

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
	//struct rusage usage;
	//int procStatus = -1;

	if (!plugin)
		return false;

	if (!plugin->process)
		return false;

	// TODO: Get the current status of the proc, and if it is stopped continue it
	//kwait4(plugin->process->p_pid, &procStatus, 0, &usage);

	kkill(plugin->process->p_pid, SIGCONT);

	return true;
}

uint8_t debugger_pause(struct debugger_plugin_t* plugin)
{
	if (!plugin)
		return false;

	if (!plugin->process)
		return false;


	kkill(plugin->process->p_pid, SIGSTOP);

	return true;
}