#pragma once
#ifdef ONI_PLATFORM
#include <oni/utils/types.h>
#else
#include <stdint.h>
#endif

#define VECTOR_DEFAULT_CAPACITY	8
#define VECTOR_INCREMENT_COUNT	16

struct vector_t
{
	// Curent count of elements in our array
	uint32_t size;

	// Total capcaity of our array
	uint32_t capacity;

	// Pointer to an array of our objects
	void** data;

	uint32_t dataElementSize;
	// The sizeof(T) for data
};

typedef struct vector_t Vector;

uint8_t vector_initialize(Vector* vector, uint32_t elementSize);

// ASSUMPTION: value must be allocated with k_malloc
uint8_t vector_append(Vector* vector, void* value);

void* vector_get(Vector* vector, uint32_t index);
uint8_t vector_set(Vector* vector, uint32_t index, void* value);

void vector_free(Vector* vector);
