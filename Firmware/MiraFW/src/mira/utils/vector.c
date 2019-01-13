#include "vector.h"
#include <oni/utils/memory/allocator.h>
#include <oni/utils/logger.h>
#include <oni/utils/kernel.h>

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

	uint32_t wantedIndex = vector->size;

	// Validate that the next add will be in bounds, otherwise we need to increase the size
	if (wantedIndex >= vector->capacity)
	{
		if (!vector_increaseCapacity(vector))
			return false;
	}

	if (vector->data == NULL)
		return false;

	vector->data[wantedIndex] = value;
	vector->size++;

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

	(void)memset(newArray, 0, wantedTotalSize);

	if (vector->data != NULL)
	{
		(void)memcpy(newArray, vector->data, currentTotalSize);

		// Free the previous list
		k_free(vector->data);
		vector->data = NULL;
	}

	vector->capacity = wantedCapacity;
	vector->data = (void**)newArray;

	return true;
}

void* vector_get(Vector* vector, uint32_t index)
{
	if (vector == NULL)
		return NULL;

	if (index >= vector->size)
		return NULL;

	return vector->data[index];
}

uint8_t vector_set(Vector* vector, uint32_t index, void* value)
{
	if (vector == NULL)
		return false;

	if (index >= vector->size)
		return false;

	void* previousObject = vector->data[index];
	if (previousObject != NULL)
	{
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

	uint32_t currentSize = vector->size;
	for (uint32_t i = 0; i < currentSize; ++i)
	{
		void* currentObject = vector->data[i];
		if (currentObject == NULL)
			continue;

		k_free(currentObject);
		vector->data[i] = NULL;
	}

	// Free the data list
	k_free(vector->data);
	vector->data = NULL;

	// Set cap & size to 0
	vector->capacity = 0;
	vector->size = 0;
}