#include "debugger_plugin.h"

#include <sys/lock.h>
#include <sys/sx.h>
#include <sys/proc.h>
#include <sys/mman.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>

#include <oni/utils/logger.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/ref.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/cpu.h>
#include <oni/utils/kernel.h>
#include <oni/utils/memory/allocator.h>

uint8_t debugger_isAddressMapped(struct debugger_plugin_t* plugin, void* address)
{
	if (!plugin)
		return false;

	if (!address)
		return false;

	for (size_t i = 0; i < ARRAYSIZE(plugin->segments); ++i)
	{
		struct segment_t* segment = &plugin->segments[i];

		// Skip NULL segments
		if (!segment->address)
			continue;

		// If the address is before the segment start address skip
		if (address < segment->address)
			continue;

		// If the address is beyond the segment size
		if ((uint64_t)address > ((uint64_t)segment->address + segment->size))
			continue;

		// We are in the middle of a mapped section congrats.
		return true;
	}

	// :(
	return false;
}

int32_t debugger_findFreeBreakpointIndex(struct debugger_plugin_t* plugin)
{
	if (!plugin)
		return -1;

	for (size_t i = 0; i < ARRAYSIZE(plugin->breakpoints); ++i)
	{
		struct breakpoint_t* breakpoint = &plugin->breakpoints[i];

		// If this is a free address, return
		if (!breakpoint->address)
			return i;
	}

	return -1;
}

void* debugger_getTextAddress(struct debugger_plugin_t* plugin)
{
	if (!plugin)
		return NULL;

	for (size_t i = 0; i < ARRAYSIZE(plugin->segments); ++i)
	{
		struct segment_t* segment = &plugin->segments[i];
		
		// Skip empty segments
		if (!segment->address)
			continue;

		// Check for EXEC | READ perms (99.99% .text)
		if (segment->protection & (PROT_EXEC | PROT_READ))
			return segment->address;
	}

	return NULL;
}

int32_t debugger_addBreakpoint(struct debugger_plugin_t* plugin, void* address, uint8_t size, uint8_t hardware)
{
	if (!plugin)
		return -1;

	if (!address)
		return -1;

	// The only sizes that can be passed in are 1, 2, 4 bytes
	switch (size)
	{
	case 1:
	case 2:
	case 4:
		break;
	default:
		return -1;
	}
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	void(*critical_enter)(void) = kdlsym(critical_enter);
	void(*critical_exit)(void) = kdlsym(critical_exit);
	struct  proc* (*pfind)(pid_t) = kdlsym(pfind);

	struct proc* proc = pfind(plugin->pid);
	if (!proc)
		return -1;

	_mtx_lock_flags(&plugin->lock, 0, __FILE__, __LINE__);

	int32_t index = -1;
	if (hardware)
	{
		WriteLog(LL_Info, "hardware breakpoints aren't supported yet.");
		index = -1;
		goto cleanup;
	}

	// Check that this address is mapped
	if (!debugger_isAddressMapped(plugin, address))
	{
		WriteLog(LL_Error, "address %p is not mapped", address);
		index = -1;
		goto cleanup;
	}

	uint8_t* textAddress = debugger_getTextAddress(plugin);
	if (!textAddress)
	{
		WriteLog(LL_Error, "could not get text address");
		index = -1;
		goto cleanup;
	}

	size_t breakpointOffset = (uint8_t*)address - textAddress;
	int32_t backupLength = debugger_getDisassemblyMinLength(plugin, address, size);
	if (backupLength <= 0)
	{
		WriteLog(LL_Debug, "could not get disassembly length %p %d", address, size);
		index = -1;
		goto cleanup;
	}

	// Find a free index
	index = debugger_findFreeBreakpointIndex(plugin);
	if (index < 0)
	{
		WriteLog(LL_Debug, "no free breakpoints");
		index = -1;
		goto cleanup;
	}

	// Get our breakpoint address
	struct breakpoint_t* breakpoint = &plugin->breakpoints[index];

	// Zero out any nasties that may not have been reset
	memset(breakpoint, 0, sizeof(*breakpoint));

	// Backup these bytes
	breakpoint->backup = kmalloc(backupLength);
	if (!breakpoint->backup)
	{
		WriteLog(LL_Error, "could not allocate backup bytes");
		index = -1;
		goto cleanup;
	}
	memset(breakpoint->backup, 0, backupLength);

	// Copy the memory
	if ((uint64_t)address & 0x8000000000000000ULL) // handle kernel address
		memcpy(breakpoint->backup, address, backupLength);
	else
	{
		size_t bytesRead = 0;
		// handle userland address
		int result = proc_rw_mem(proc, address, backupLength, breakpoint->backup, &bytesRead, false);

		// Handle errors
		if (result < 0)
		{
			kfree(breakpoint->backup, backupLength);
			breakpoint->backup = NULL;
			WriteLog(LL_Error, "could not read userland memory %p (%d)", address, result);
			index = -1;
			goto cleanup;
		}
	}

	// Set the address
	breakpoint->address = address;
	breakpoint->offset = breakpointOffset;
	breakpoint->backupLength = backupLength;
	breakpoint->size = size;

	// Write the final breakpoint
	if ((uint64_t)address & 0x8000000000000000ULL) // handle kernel address
	{
		WriteLog(LL_Debug, "software breakpointing kernel address %p", address);
		critical_enter();
		cpu_disable_wp();
		memset(address, 0xCC, 1);
		cpu_enable_wp();
		critical_exit();
	}
	else // handle software bp
	{
		WriteLog(LL_Debug, "software breakpointing userland address %p", address);
		uint8_t bp[] = { 0xCC };
		size_t bytesWritten = 0;

		int result = proc_rw_mem(proc, address, 1, bp, &bytesWritten, true);

		if (result < 0)
		{
			kfree(breakpoint->backup, backupLength);
			breakpoint->backup = NULL;
			memset(breakpoint, 0, sizeof(*breakpoint));
			WriteLog(LL_Error, "could not write userland breakpoint %p", address);
			index = -1;
			goto cleanup;
		}
	}

cleanup:
	_mtx_unlock_flags(&plugin->lock, 0, __FILE__, __LINE__);
	PROC_UNLOCK(proc);
	return index;
}

uint8_t debugger_removeBreakpoint(struct debugger_plugin_t* plugin, void* address)
{
	if (!plugin)
		return false;

	if (!address)
		return false;

	// Ensure we have a valid process
	if (plugin->pid < 0)
		return false;

	void(*critical_enter)(void) = kdlsym(critical_enter);
	void(*critical_exit)(void) = kdlsym(critical_exit);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	struct  proc* (*pfind)(pid_t) = kdlsym(pfind);

	struct proc* proc = pfind(plugin->pid);
	if (!proc)
		return false;

	_mtx_lock_flags(&plugin->lock, 0, __FILE__, __LINE__);

	// Iterate all of our breakpoints
	int32_t result = false;
	for (size_t i = 0; ARRAYSIZE(plugin->breakpoints); ++i)
	{
		struct breakpoint_t* breakpoint = &plugin->breakpoints[i];

		// Skip empty addresses
		if (!breakpoint->address)
			continue;

		// Skip breakpoints that aren't this one we want to remove
		if (breakpoint->address != address)
			continue;

		// Handle removing brekapoints

		if (breakpoint->hardware) // handle hardware breakpoints
		{
			// TODO: Remove the hardware breakpoint
			WriteLog(LL_Error, "hardware breakpoints not yet supported");
			result = false;
			break;
		}
		else // handle software breakpoints
		{
			// Check if the address is still mapped
			if (debugger_isAddressMapped(plugin, breakpoint->address))
			{
				// If we are mapped, set the bytes back
				if (!breakpoint->backup)
					WriteLog(LL_Warn, "could not reset bytes of software breakpoint %p", breakpoint->address);
				else
				{
					// Handle kernel restore
					if ((uint64_t)address & 0x8000000000000000ULL)
					{
						critical_enter();
						cpu_disable_wp();
						memcpy(address, breakpoint->backup, breakpoint->backupLength);
						cpu_enable_wp();
						critical_exit();
					}
					else // Handle userland restore
					{
						// Attempt to write the original bytes back
						//proc_rw_mem(struct proc* p, void* ptr, size_t size, void* data, size_t* n, int write);
						size_t bytesWritten = 0;
						int result = proc_rw_mem(proc, breakpoint->address, breakpoint->backupLength, breakpoint->backup, &bytesWritten, true);
						if (!result)
							WriteLog(LL_Warn, "could not write original bytes back (%d) %p", result, breakpoint->address);
					}
				}
			}
			else // Let the user know some shit hit the fan
				WriteLog(LL_Warn, "not writing original bytes back, old address %p is not mapped", breakpoint->address);

			// Zero out the entry
			memset(breakpoint, 0, sizeof(*breakpoint));

			WriteLog(LL_Info, "breakpoint at %p removed", address);
			result = true;
			break;
		}
	}

	_mtx_unlock_flags(&plugin->lock, 0, __FILE__, __LINE__);

	PROC_UNLOCK(proc);

	if (!result)
		WriteLog(LL_Info, "couldn't find bp info for address %p", address);

	return result;
}
void debugger_updateSegments(struct debugger_plugin_t* plugin)
{
	if (!plugin)
		return;

	if (plugin->pid < 0)
		return;

	void(*_vm_map_lock_read)(vm_map_t map, const char *file, int line) = kdlsym(_vm_map_lock_read);
	void(*_vm_map_unlock_read)(vm_map_t map, const char *file, int line) = kdlsym(_vm_map_unlock_read);
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	void(*vmspace_free)(struct vmspace *) = kdlsym(vmspace_free);
	struct vmspace* (*vmspace_acquire_ref)(struct proc *) = kdlsym(vmspace_acquire_ref);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	struct  proc* (*pfind)(pid_t) = kdlsym(pfind);

	struct proc* proc = pfind(plugin->pid);
	if (!proc)
		return;


	_mtx_lock_flags(&plugin->lock, MTX_QUIET, __FILE__, __LINE__);

	// Keep track of our current segment index
	uint32_t segmentIndex = 0;

	// Zero out all of our current segments
	memset(plugin->segments, 0, sizeof(plugin->segments));

	// Get the vm map
	struct vmspace* vm = vmspace_acquire_ref(proc);
	vm_map_t map = &proc->p_vmspace->vm_map;
	vm_map_lock_read(map);

	struct vm_map_entry* entry = NULL;
	while ((entry = map->header.next) != NULL)
	{
		struct segment_t* segment = &plugin->segments[segmentIndex];
		segment->address = (void*)entry->start;
		segment->size = entry->end - entry->start;
		segment->protection = entry->protection;
		segmentIndex++;
	}

	// Free the vmmap
	vm_map_unlock_read(map);
	vmspace_free(vm);

	PROC_UNLOCK(proc);

	_mtx_unlock_flags(&plugin->lock, MTX_QUIET, __FILE__, __LINE__);

	WriteLog(LL_Info, "updated segments");
}

void debugger_updateBreakpoints(struct debugger_plugin_t* plugin)
{
	if (!plugin)
		return;

	//int(*_sx_slock)(struct sx *sx, int opts, const char *file, int line) = kdlsym(_sx_slock);
	//void(*_sx_sunlock)(struct sx *sx, const char *file, int line) = kdlsym(_sx_sunlock);
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);

	_mtx_lock_flags(&plugin->lock, MTX_QUIET, __FILE__, __LINE__);

	for (size_t i = 0; i < ARRAYSIZE(plugin->breakpoints); ++i)
	{
		struct breakpoint_t* breakpoint = &plugin->breakpoints[i];

		// Skip NULL breakpoints
		if (!breakpoint->address)
			continue;

		// Check to see if the address is currently mapped
		if (!debugger_isAddressMapped(plugin, breakpoint->address))
		{
			// Hold a temporary breakpoint
			struct breakpoint_t oldBreakpoint;

			// Zero the new breakpoint data
			memset(&oldBreakpoint, 0, sizeof(oldBreakpoint));

			// Copy the previous data over
			memcpy(&oldBreakpoint, breakpoint, sizeof(*breakpoint));

			// Remove the previous breakpoint
			debugger_removeBreakpoint(plugin, oldBreakpoint.address);

			uint8_t* newAddress = debugger_getTextAddress(plugin);
			if (!newAddress)
			{
				WriteLog(LL_Error, "could not get debugger text.");

				// Remove this entry
				memset(breakpoint, 0, sizeof(*breakpoint));
				continue;
			}

			newAddress += oldBreakpoint.offset;

			int32_t index = debugger_addBreakpoint(plugin, newAddress, oldBreakpoint.size, oldBreakpoint.hardware);
			if (index < 0)
			{
				WriteLog(LL_Error, "could not set breakpoint");
				// Remove this entry
				memset(breakpoint, 0, sizeof(*breakpoint));
				continue;
			}

			WriteLog(LL_Info, "breakpoint renewed %p", newAddress);
		}
	}

	_mtx_unlock_flags(&plugin->lock, MTX_QUIET, __FILE__, __LINE__);
}

uint8_t debugger_clearAllBreakpoints(struct debugger_plugin_t* plugin)
{
	if (!plugin)
		return false;

	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	_mtx_lock_flags(&plugin->lock, MTX_QUIET, __FILE__, __LINE__);

	for (size_t i = 0; i < ARRAYSIZE(plugin->breakpoints); ++i)
	{
		struct breakpoint_t* breakpoint = &plugin->breakpoints[i];

		// Skip NULL breakpoints
		if (!breakpoint->address)
			continue;

		// Remove each of the breakpoints
		if (!debugger_removeBreakpoint(plugin, breakpoint->address))
			WriteLog(LL_Warn, "could not remove breakpoint for address %p", breakpoint->address);
	}

	// Zero out any leftover buffer stuff
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	memset(plugin->breakpoints, 0, sizeof(plugin->breakpoints));

	_mtx_unlock_flags(&plugin->lock, MTX_QUIET, __FILE__, __LINE__);

	return true;
}