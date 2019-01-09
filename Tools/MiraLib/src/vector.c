#include "vector.h"
#ifdef ONI_PLATFORM
#include <oni/utils/memory/allocator.h>
#include <oni/utils/logger.h>
#include <oni/utils/kernel.h>
#else
#include <stdio.h>
#include <stdlib.h>

#define true 1
#define false 0

#define WriteLog(x, y, ...) fprintf(stderr, y, __VA_ARGS__)
#define k_malloc malloc
#define k_free free

#endif

static uint8_t vector_increaseCapacity(Vector* vector);

uint8_t vector_initialize(Vector* vector, uint32_t elementSize)
{
	if (vector == NULL || elementSize == 0)
		return false;

	vector->size = 0;
	vector->capacity = 0;

	vector->data = NULL;
	vector->dataElementSize = elementSize;

	return true;
}

uint8_t vector_append(Vector* vector, void* value)
{
	if (vector == NULL || value == NULL)
		return false;

	uint32_t wantedIndex = vector->size + 1;

	WriteLog(LL_Debug, "index: %d cap: %d", wantedIndex, vector->capacity);

	// Validate that the next add will be in bounds, otherwise we need to increase the size
	if (wantedIndex >= vector->capacity)
	{
		WriteLog(LL_Warn, "here");

		if (!vector_increaseCapacity(vector))
			return false;
	}

	WriteLog(LL_Warn, "here");

	vector->data[wantedIndex] = value;
	vector->size++;

	WriteLog(LL_Warn, "here");

	return true;
}

static uint8_t vector_increaseCapacity(Vector* vector)
{
	if (vector == NULL)
		return false;

	uint32_t currentCapacity = vector->capacity;
	uint32_t wantedCapacity = vector->capacity + VECTOR_INCREMENT_COUNT;

	// Current size in memory of our vector
	uint64_t currentTotalSize = currentCapacity * vector->dataElementSize;
	uint64_t wantedTotalSize = wantedCapacity * vector->dataElementSize;

	WriteLog(LL_Debug,
		"currentCapacity %d wantedCapacity %d currentTotalSize 0x%llx wantedTotalSize 0x%llx.",
		currentCapacity,
		wantedCapacity,
		currentTotalSize,
		wantedTotalSize);

	// Alloicate the new list size
	uint8_t* newArray = k_malloc(wantedTotalSize);
	if (!newArray)
	{
		WriteLog(LL_Error, "could not allocate new array");
		return false;
	}
	WriteLog(LL_Warn, "here");

	(void)memset(newArray, 0, wantedTotalSize);

	if (vector->data != NULL)
	{
		WriteLog(LL_Warn, "here");
		(void)memcpy(newArray, vector->data, currentTotalSize);

		WriteLog(LL_Warn, "here");
		// Free the previous list
		k_free(vector->data);
		vector->data = NULL;
	}

	WriteLog(LL_Warn, "here");
	vector->capacity = wantedCapacity;
	vector->data = (void**)newArray;

	return true;
}

void* vector_get(Vector* vector, uint32_t index)
{
	if (vector == NULL)
		return NULL;

	WriteLog(LL_Warn, "here");

	if (index >= vector->size)
		return NULL;

	WriteLog(LL_Warn, "here");

	return vector->data[index];
}

uint8_t vector_set(Vector* vector, uint32_t index, void* value)
{
	if (vector == NULL)
		return false;

	if (index >= vector->size)
		return false;

	WriteLog(LL_Warn, "here");
	void* previousObject = vector->data[index];
	if (previousObject != NULL)
	{
		WriteLog(LL_Warn, "here");
		k_free(previousObject);
		vector->data[index] = NULL;
	}
	
	vector->data[index] = value;
	return true;
}

void vector_free(Vector* vector)
{
	if (vector == NULL)
		return;

	WriteLog(LL_Warn, "here");

	uint32_t currentSize = vector->size;
	for (uint32_t i = 0; i < currentSize; ++i)
	{
		WriteLog(LL_Warn, "here");
		void* currentObject = vector->data[i];
		if (currentObject == NULL)
			continue;

		WriteLog(LL_Warn, "here");
		k_free(currentObject);
		vector->data[i] = NULL;
	}

	WriteLog(LL_Warn, "here");
	// Free the data list
	k_free(vector->data);
	vector->data = NULL;

	// Set cap & size to 0
	vector->capacity = 0;
	vector->size = 0;
}