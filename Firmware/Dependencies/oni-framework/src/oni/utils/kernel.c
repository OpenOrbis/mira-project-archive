#include <oni/utils/types.h>



#include <oni/utils/kernel.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/memory/allocator.h>

#include <sys/lock.h>
#include <sys/mutex.h>

#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/param.h>
#include <sys/uio.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>


#define  VM_PROT_GPU_READ ((vm_prot_t)0x10)
#define  VM_PROT_GPU_WRITE ((vm_prot_t)0x20)

#define PROT_CPU_READ 0x1
#define PROT_CPU_WRITE 0x2
#define PROT_CPU_EXEC 0x4
#define PROT_GPU_READ 0x10
#define PROT_GPU_WRITE 0x20

int proc_rw_mem_pid(int pid, void* ptr, size_t size, void* data, size_t* n, int write)
{
	struct  proc* (*pfind)(pid_t) = kdlsym(pfind);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	struct proc* process = pfind(pid);
	if (!process)
		return -1;

	int result = proc_rw_mem(process, ptr, size, data, n, write);

	_mtx_unlock_flags(&process->p_mtx, 0, __FILE__, __LINE__);

	return result;
}

int proc_rw_mem(struct proc* p, void* ptr, size_t size, void* data, size_t* n, int write)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	int(*proc_rwmem)(struct proc *p, struct uio *uio) = kdlsym(proc_rwmem);

	struct thread* td = curthread;
	struct iovec iov;
	struct uio uio;
	int ret;

	if (!p) {
		ret = EINVAL;
		goto error;
	}

	if (size == 0) {
		if (n)
			*n = 0;
		ret = 0;
		goto error;
	}

	memset(&iov, 0, sizeof(iov));
	iov.iov_base = (caddr_t)data;
	iov.iov_len = size;

	memset(&uio, 0, sizeof(uio));
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	uio.uio_offset = (off_t)ptr;
	uio.uio_resid = (ssize_t)size;
	uio.uio_segflg = UIO_SYSSPACE;
	uio.uio_rw = write ? UIO_WRITE : UIO_READ;
	uio.uio_td = td;

	
	ret = proc_rwmem(p, &uio);
	if (n)
		*n = (size_t)((ssize_t)size - uio.uio_resid);

error:
	return ret;
}

// Credits: flatz
int proc_get_vm_map(struct proc* p, struct proc_vm_map_entry** entries, size_t* num_entries) 
{
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	struct vmspace* (*vmspace_acquire_ref)(struct proc *) = kdlsym(vmspace_acquire_ref);
	void(*vmspace_free)(struct vmspace *) = kdlsym(vmspace_free);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void(*wakeup)(void*) = kdlsym(wakeup);
	void(*_vm_map_lock_read)(vm_map_t map, const char *file, int line) = kdlsym(_vm_map_lock_read);
	void(*_vm_map_unlock_read)(vm_map_t map, const char *file, int line) = kdlsym(_vm_map_unlock_read);
	void(*faultin)(struct proc *p) = kdlsym(faultin);

	struct vmspace* vm;
	struct proc_vm_map_entry* info = NULL;
	vm_map_t map;
	vm_map_entry_t entry;
	size_t n, i;
	int ret;

	if (!p) {
		ret = EINVAL;
		goto error;
	}
	if (!entries) {
		ret = EINVAL;
		goto error;
	}
	if (!num_entries) {
		ret = EINVAL;
		goto error;
	}

	PROC_LOCK(p);
	if (p->p_flag & P_WEXIT) {
		PROC_UNLOCK(p);
		ret = ESRCH;
		goto error;
	}
	_PHOLD(p);
	PROC_UNLOCK(p);

	vm = vmspace_acquire_ref(p);
	if (!vm) {
		PRELE(p);
		ret = ESRCH;
		goto error;
	}
	map = &vm->vm_map;

	vm_map_lock_read(map);
	for (entry = map->header.next, n = 0; entry != &map->header; entry = entry->next) {
		if (entry->eflags & MAP_ENTRY_IS_SUB_MAP)
			continue;
		++n;
	}
	if (n == 0)
		goto done;
	size_t allocSize = n * sizeof(*info);
	info = (struct proc_vm_map_entry*)kmalloc(allocSize);
	if (!info) {
		vm_map_unlock_read(map);
		vmspace_free(vm);

		PRELE(p);

		ret = ENOMEM;
		goto error;
	}
	memset(info, 0, n * sizeof(*info));
	for (entry = map->header.next, i = 0; entry != &map->header; entry = entry->next) {
		if (entry->eflags & MAP_ENTRY_IS_SUB_MAP)
			continue;

		info[i].start = entry->start;
		info[i].end = entry->end;
		info[i].offset = entry->offset;

		info[i].prot = 0;
		if (entry->protection & VM_PROT_READ)
			info[i].prot |= PROT_CPU_READ;
		if (entry->protection & VM_PROT_WRITE)
			info[i].prot |= PROT_CPU_WRITE;
		if (entry->protection & VM_PROT_EXECUTE)
			info[i].prot |= PROT_CPU_EXEC;
		if (entry->protection & VM_PROT_GPU_READ)
			info[i].prot |= PROT_GPU_READ;
		if (entry->protection & VM_PROT_GPU_WRITE)
			info[i].prot |= PROT_GPU_WRITE;

		++i;
	}

done:
	vm_map_unlock_read(map);
	vmspace_free(vm);

	PRELE(p);

	*num_entries = n;
	*entries = info;

	info = NULL;
	ret = 0;

error:
	if (info)
		kfree(info, allocSize);

	return ret;
}

// Credits: flatz
struct proc* proc_find_by_name(const char* name) 
{
	int(*_sx_slock)(struct sx *sx, int opts, const char *file, int line) = kdlsym(_sx_slock);
	void(*_sx_sunlock)(struct sx *sx, const char *file, int line) = kdlsym(_sx_sunlock);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);

	struct sx* allproclock = (struct sx*)kdlsym(allproc_lock);
	struct proclist* allproc = (struct proclist*)*(uint64_t*)kdlsym(allproc);

	int(*strcmp)(const char *str1, const char* str2) = kdlsym(strcmp);

	struct proc* p;

	if (!name)
		return NULL;

	_sx_slock(allproclock, 0, __FILE__, __LINE__);

	FOREACH_PROC_IN_SYSTEM(p) {
		PROC_LOCK(p);


		if (strcmp(p->p_comm, name) == 0) {
			PROC_UNLOCK(p);
			goto done;
		}

		PROC_UNLOCK(p);
	}

	p = NULL;

done:
	_sx_sunlock(allproclock, __FILE__, __LINE__);

	return p;
}
