#pragma once
#include <oni/utils/types.h>
#include <sys/param.h>
#include <sys/lock.h>
#include <sys/mutex.h>

struct ref_t
{
	// Size of the data
	uint64_t size;

	// Reference count
	volatile uint64_t count;

	// Before doing anything, this lock must be held
	struct mtx lock;

	uint8_t data[];
};

struct ref_t* ref_alloc(size_t size);

struct ref_t* ref_realloc(struct ref_t* originalRef, uint64_t objectSize);

struct ref_t* ref_fromObject(void* object, uint64_t objectSize);

void* ref_getData(struct ref_t* reference);

uint64_t ref_getSize(struct ref_t* reference);

void ref_acquire(struct ref_t* reference);

void ref_release(struct ref_t* reference);

void* ref_getDataAndAcquire(struct ref_t* reference);