#pragma once
#include <oni/utils/types.h>

#define FLAT_VECTOR_DEFAULT_CAPACITY	8
#define FLAT_VECTOR_INCREMENT_COUNT	16

struct flat_vector_t
{
	uint32_t size;

	uint32_t capacity;

	uint8_t* data;

	uint32_t dataElementSize;
};

typedef struct flat_vector_t FlatVector;

uint8_t flat_vector_initialize(FlatVector* vector, uint32_t elementSize);

// This takes the address of something, will make a clone of it internally for use
uint8_t flat_vector_append(FlatVector* vector, void* value);

void* flat_vector_get(FlatVector* vector, uint32_t index);

uint8_t flat_vector_set(FlatVector* vector, uint32_t index, void* value);

void flat_vector_free(FlatVector* vector);