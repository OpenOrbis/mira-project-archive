#pragma once
#include <oni/utils/types.h>

struct task_struct;
struct proc;
struct proc_vm_map_entry;

struct proc_vm_map_entry
{
	char name[32];
	vm_offset_t start;
	vm_offset_t end;
	vm_offset_t offset;
	uint16_t prot;
};

/*
	kernelRdmsr

	TODO: Description
*/
uint64_t kernelRdmsr(int Register);
int proc_rw_mem_pid(int pid, void* ptr, size_t size, void* data, size_t* n, int write);
int proc_rw_mem(struct proc* p, void* ptr, size_t size, void* data, size_t* n, int write);

struct proc* proc_find_by_name(const char* name);
int proc_get_vm_map(struct proc* p, struct proc_vm_map_entry** entries, size_t* num_entries);


void	*memcpy(void * __restrict, const void * __restrict, size_t);
void	*memmove(void *, const void *, size_t);
void	*memset(void *, int, size_t);
int	 memcmp(const void *, const void *, size_t);

size_t strlen(const char *str);
int strcmp(const char *str1, const char *str2);