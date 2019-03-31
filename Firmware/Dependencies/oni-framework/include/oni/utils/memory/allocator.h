#pragma once
#include <sys/param.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#include <oni/utils/types.h>

/*
kmalloc

malloc(3) wrapper to decrease arguments
*/
void* kmalloc(size_t size);

/*
kfree

Frees the memory at `address` with size `size`
*/
void kfree(void* address, size_t size);

void* kcalloc(size_t n, size_t size);

void* krealloc(void* address, size_t size);

void* k_malloc(size_t size);

void k_free(void* address);

void* k_calloc(size_t n, size_t size);

void* k_realloc(void* address, size_t size);