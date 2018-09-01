#include "debugger_plugin.h"
#define LOCK_PROFILING
#include <oni/framework.h>

#include <oni/messaging/message.h>
#include <oni/messaging/messagemanager.h>

#include <oni/utils/logger.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/ref.h>
#include <oni/utils/kernel.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>

#include <sys/proc.h>



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

struct debugger_getthreads_t
{
	int pid;
};

void debugger_readmem_callback(struct ref_t* reference)
{
	void* (*_memset)(void *s, int c, size_t n) = kdlsym(memset);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	struct  proc* (*pfind)(pid_t) = kdlsym(pfind);

	struct message_t* message = ref_getIncrement(reference);
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
		_memset(request, 0, sizeof(*request));
		request->process_id = -1;
		kwrite(message->socket, request, sizeof(*request));
	}

cleanup:
	ref_release(reference);
}

void debugger_writemem_callback(struct ref_t* reference)
{
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	struct message_t* message = ref_getIncrement(reference);
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

	int result = proc_rw_mem(process, (void*)request->address, request->dataLength, request->data, &request->dataLength, 1);
	if (result < 0)
		WriteLog(LL_Error, "proc_rw_mem returned %d", result);

	// You need to unlock the process, or the kernel will assert and hang
	PROC_UNLOCK(process);

cleanup:
	ref_release(reference);
}

void debugger_getprocs_callback(struct ref_t* reference)
{
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
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	if (!reference)
		return;

	struct message_t* message = ref_getIncrement(reference);
	if (!message)
		return;

	// Only handle requests
	if (message->header.request != 1)
		goto cleanup;

	uint64_t procCount = 0;
	struct proc* p = NULL;
	struct debugger_getprocs_t getproc;
	memset(&getproc, 0, sizeof(getproc));

	sx_slock(allproclock);
	FOREACH_PROC_IN_SYSTEM(p)
	{
		PROC_LOCK(p);
		// Zero out our process information
		memset(&getproc, 0, sizeof(getproc));

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
		memcpy(getproc.process_name, p->p_comm, sizeof(getproc.process_name));
		memcpy(getproc.path, p->p_elfpath, sizeof(getproc.path));
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
	memset(&getproc, 0xDD, sizeof(getproc));
	kwrite(message->socket, &getproc, sizeof(getproc));

cleanup:
	ref_release(reference);
}

void debugger_ptrace_callback(struct ref_t* reference)
{
	if (!reference)
		return;

	struct message_t* message = ref_getIncrement(reference);
	if (!message)
		return;

	// Only handle requests
	if (message->header.request != 1)
		goto cleanup;

	if (!message->payload)
	{
		messagemanager_sendResponse(reference, -ENOMEM);
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
	ref_release(reference);
}

void debugger_kill_callback(struct ref_t* reference)
{
	if (!reference)
		return;

	struct message_t* message = ref_getIncrement(reference);
	if (!message)
		return;

	// Only handle requests
	if (message->header.request != 1)
		goto cleanup;

	if (!message->payload)
	{
		messagemanager_sendResponse(reference, -ENOMEM);
		goto cleanup;
	}

	struct debugger_kill_t* killRequest = (struct debugger_kill_t*)message->payload;

	int result = kkill(killRequest->pid, killRequest->signal);

	messagemanager_sendResponse(reference, result);

cleanup:
	ref_release(reference);
}