#include <oni/utils/memory/allocator.h>

#include <vm/vm.h>

#include <oni/utils/kdlsym.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/logger.h>

#include <sys/malloc.h>


void* kmalloc(size_t size)
{
	vm_map_t map = (vm_map_t)(*(uint64_t *)(kdlsym(kernel_map)));
	vm_offset_t(*kmem_alloc)(vm_map_t map, vm_size_t size) = kdlsym(kmem_alloc);

	return (void*)kmem_alloc(map, size);
}

void kfree(void* address, size_t size)
{
	vm_map_t map = (vm_map_t)(*(uint64_t *)(kdlsym(kernel_map)));
	void(*kmem_free)(void* map, void* addr, size_t size) = kdlsym(kmem_free);

	kmem_free(map, address, size);
}

void* k_malloc(size_t size)
{
	if (!size)
		size = sizeof(uint64_t);

	uint8_t* data = kmalloc(size + sizeof(uint64_t));
	if (!data)
		return NULL;

	// Set our pointer header
	(*(uint64_t*)data) = size;

	// Return the start of the requested data
	return data + sizeof(uint64_t);
}

void k_free(void* address)
{
	if (!address)
		return;

	uint8_t* data = ((uint8_t*)address) - sizeof(uint64_t);

	uint64_t size = *(uint64_t*)data;

	kfree(data, size);
}

void* kcalloc(size_t n, size_t size)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	size_t total = n * size;
	void *p = kmalloc(total);

	if (!p) return NULL;

	return memset(p, 0, total);
}

void* k_calloc(size_t n, size_t size)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	size_t total = n * size;
	void *p = k_malloc(total);

	if (!p) return NULL;

	return memset(p, 0, total);
}

void* k_realloc(void* address, size_t size)
{
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);

	if (size == 0 && address != NULL)
	{
		k_free(address);
		return NULL;
	}
	else if (address == NULL)
		return k_malloc(size);
	else if (address != NULL && size != 0)
	{
		uint8_t* data = ((uint8_t*)address) - sizeof(uint64_t);

		uint64_t oldSize = *(uint64_t*)data;

		if (size < oldSize)
			return address;
		else
		{
			void* newData = k_malloc(size);
			if (newData != NULL)
			{
				(void)memcpy(newData, address, oldSize);
				k_free(address);
			}
			return newData;
		}
	}
	
	return NULL;
}

void* krealloc(void* address, size_t size)
{
	void* (*realloc)(void *addr, unsigned long size, struct malloc_type	*type, int flags) = kdlsym(realloc);
	void* M_TEMP = kdlsym(M_TEMP);

	return realloc(address, size, M_TEMP, M_WAITOK);
}