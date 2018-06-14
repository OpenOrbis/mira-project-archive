#include "debugger_plugin.h"
#define LOCK_PROFILING
#include <sys/param.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#include <oni/utils/memory/allocator.h>
#include <oni/messaging/message.h>
#include <oni/messaging/messagemanager.h>
#include <oni/utils/kdlsym.h>

#include <sys/kdb.h>
#include <sys/proc.h>
#include <oni/utils/sys_wrappers.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>

#include <oni/utils/logger.h>

#include <oni/init/initparams.h>
#include <oni/framework.h>

enum { Debugger_MaxMem = 0x1000 };

struct debugger_getprocs_t
{
	int32_t process_id;

	char path[1024];
	char process_name[32];

	uint64_t text_address;
	uint64_t text_size;

	uint64_t data_address;
	uint64_t data_size;

	uint64_t virtual_size;
};

struct debugger_readmem_t
{
	int32_t process_id;

	uint64_t address;
	uint64_t dataLength;
	uint8_t data[Debugger_MaxMem];
};

struct debugger_writemem_t
{
	int32_t process_id;

	uint64_t address;
	uint64_t dataLength;
	uint8_t data[Debugger_MaxMem];
};

struct debugger_setbp_t
{
	int32_t process_id;

	uint64_t address;
	uint8_t hardware;
};

struct debugger_ptrace_t
{
	int res;
	int req;
	int pid;
	uint64_t addr;
	int data;
	int setAddrToBuffer;
	char buffer[0x800];
};

struct debugger_kill_t
{
	int pid;
	int signal;
};


void debugger_getprocs_callback(struct allocation_t* ref);
void debugger_readmem_callback(struct allocation_t* ref);
void debugger_writemem_callback(struct allocation_t* ref);
void debugger_ptrace_callback(struct allocation_t* ref);
void debugger_kill_callback(struct allocation_t* ref);

// Credits: flatz
int proc_rw_mem(struct proc* p, void* ptr, size_t size, void* data, size_t* n, int write);


uint8_t debugger_load(struct debugger_plugin_t * plugin)
{
	return true;
}

uint8_t debugger_unload(struct debugger_plugin_t * plugin)
{
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
}

void debugger_readmem_callback(struct allocation_t* ref)
{
	struct message_t* message = __get(ref);
	if (!message)
		return;

	if (message->header.request != 1)
		goto cleanup;

	if (message->socket < 0)
		goto cleanup;

	if (!message->payload)
		goto cleanup;

	struct debugger_readmem_t* request = (struct debugger_readmem_t*)message->payload;
	if (request->process_id < 0)
	{
		WriteLog(LL_Error, "invalid process id.");
		goto error;
	}

	if (request->address == 0)
	{
		WriteLog(LL_Error, "Invalid address");
		goto error;
	}

	if (request->dataLength == 0)
	{
		WriteLog(LL_Error, "Invalid data length.");
		goto error;
	}

	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	struct  proc* (*pfind)(pid_t) = kdlsym(pfind);

	struct proc* process = pfind(request->process_id);
	if (process == 0)
		goto error;

	int result = proc_rw_mem(process, (void*)request->address, request->dataLength, request->data, &request->dataLength, 0);
	PROC_UNLOCK(process);

	WriteLog(LL_Debug, "proc_rw_mem returned(%d, %p, %d, %p, %d, %s) %d", process, request->address, request->dataLength, request->data, &request->dataLength, "read", result);
	if (result < 0)
		goto error;

	message->header.request = 0;
	kwrite(message->socket, request, sizeof(*request));

	// Error conditions
	if (1 == 0)
	{
	error:
		kmemset(request, 0, sizeof(*request));
		request->process_id = -1;
		kwrite(message->socket, request, sizeof(*request));
	}

cleanup:
	__dec(ref);
}

void debugger_writemem_callback(struct allocation_t* ref)
{
	struct message_t* message = __get(ref);
	if (!message)
		return;

	if (message->header.request != 1)
		goto cleanup;

	if (message->socket < 0)
		goto cleanup;

	if (!message->payload)
		goto cleanup;

	struct debugger_writemem_t* request = (struct debugger_writemem_t*)message->payload;
	if (request->process_id < 0)
		goto cleanup;

	if (request->address == 0)
		goto cleanup;

	if (request->dataLength == 0)
		goto cleanup;

	struct  proc* (*pfind)(pid_t) = kdlsym(pfind);

	struct proc* process = pfind(request->process_id);
	if (process == 0)
		goto cleanup;

	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	int result = proc_rw_mem(process, (void*)request->address, request->dataLength, request->data, &request->dataLength, 1);
	if (result < 0)
		WriteLog(LL_Error, "proc_rw_mem returned %d", result);

	// You need to unlock the process, or the kernel will assert and hang
	PROC_UNLOCK(process);

cleanup:
	__dec(ref);
}

void debugger_getprocs_callback(struct allocation_t* ref)
{
	if (!ref)
		return;

	struct message_t* message = __get(ref);
	if (!message)
		return;

	// Only handle requests
	if (message->header.request != 1)
		goto cleanup;

	int(*_sx_slock)(struct sx *sx, int opts, const char *file, int line) = kdlsym(_sx_slock);
	void(*_sx_sunlock)(struct sx *sx, const char *file, int line) = kdlsym(_sx_sunlock);
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	struct sx* allproclock = (struct sx*)kdlsym(allproc_lock);
	struct proclist* allproc = (struct proclist*)*(uint64_t*)kdlsym(allproc);

	void(*vmspace_free)(struct vmspace *) = kdlsym(vmspace_free);
	struct vmspace* (*vmspace_acquire_ref)(struct proc *) = kdlsym(vmspace_acquire_ref);
	void(*_vm_map_lock_read)(vm_map_t map, const char *file, int line) = kdlsym(_vm_map_lock_read);
	void(*_vm_map_unlock_read)(vm_map_t map, const char *file, int line) = kdlsym(_vm_map_unlock_read);

	uint64_t procCount = 0;
	struct proc* p = NULL;
	struct debugger_getprocs_t getproc = { 0 };

	sx_slock(allproclock);
	FOREACH_PROC_IN_SYSTEM(p)
	{
		PROC_LOCK(p);
		// Zero out our process information
		kmemset(&getproc, 0, sizeof(getproc));

		// Get the vm map
		struct vmspace* vm = vmspace_acquire_ref(p);
		vm_map_t map = &p->p_vmspace->vm_map;
		vm_map_lock_read(map);

		struct vm_map_entry* entry = map->header.next;

		// Copy over all of the address information
		getproc.process_id = p->p_pid;
		getproc.text_address = (uint64_t)entry->start;
		getproc.text_size = (uint64_t)entry->end - entry->start;
		getproc.data_address = (uint64_t)p->p_vmspace->vm_daddr;
		getproc.data_size = p->p_vmspace->vm_dsize;
		// Copy over the name and path
		kmemcpy(getproc.process_name, p->p_comm, sizeof(getproc.process_name));
		kmemcpy(getproc.path, p->p_elfpath, sizeof(getproc.path));
		// Write it back to the PC
		kwrite(message->socket, &getproc, sizeof(getproc));
		procCount++;

		// Free the vmmap
		vm_map_unlock_read(map);
		vmspace_free(vm);

		PROC_UNLOCK(p);
	}
	sx_sunlock(allproclock);
	// Send finalizer, because fuck this shit
	kmemset(&getproc, 0xDD, sizeof(getproc));
	kwrite(message->socket, &getproc, sizeof(getproc));

cleanup:
	__dec(ref);
}

void debugger_ptrace_callback(struct allocation_t* ref)
{
	if (!ref)
		return;

	struct message_t* message = __get(ref);
	if (!message)
		return;

	// Only handle requests
	if (message->header.request != 1)
		goto cleanup;

	if (!message->payload)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOMEM);
		goto cleanup;
	}

	// set diag auth ID flags
	curthread->td_ucred->cr_sceAuthID = 0x3800000000000007ULL;

	// make system credentials
	curthread->td_ucred->cr_sceCaps[0] = 0xFFFFFFFFFFFFFFFFULL;
	curthread->td_ucred->cr_sceCaps[1] = 0xFFFFFFFFFFFFFFFFULL;

	struct debugger_ptrace_t* ptraceRequest = (struct debugger_ptrace_t*)message->payload;


	if (ptraceRequest->setAddrToBuffer)
		ptraceRequest->addr = (uint64_t)&ptraceRequest->buffer[0];

	WriteLog(LL_Debug, "%d %d %llx %d %s", ptraceRequest->req, ptraceRequest->pid, ptraceRequest->addr, ptraceRequest->data, ptraceRequest->setAddrToBuffer ? "true" : "false");
	ptraceRequest->res = kptrace(ptraceRequest->req, ptraceRequest->pid, (caddr_t)ptraceRequest->addr, ptraceRequest->data);

	WriteLog(LL_Debug, "ptrace: %d", ptraceRequest->res);

	kwrite(message->socket, ptraceRequest, sizeof(*ptraceRequest));

cleanup:
	__dec(ref);
}

void debugger_kill_callback(struct allocation_t* ref)
{
	if (!ref)
		return;

	struct message_t* message = __get(ref);
	if (!message)
		return;

	// Only handle requests
	if (message->header.request != 1)
		goto cleanup;

	if (!message->payload)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOMEM);
		goto cleanup;
	}

	struct debugger_kill_t* killRequest = (struct debugger_kill_t*)message->payload;

	int result = kkill(killRequest->pid, killRequest->signal);
	if (result < 0)
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, result);
	else
		messagemanager_sendSuccessMessage(gFramework->messageManager, ref);

cleanup:
	__dec(ref);
}