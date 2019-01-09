#include "flat_vector.h"
#include <oni/utils/memory/allocator.h>
#include <oni/utils/logger.h>
#include <oni/utils/kernel.h>

static uint8_t flat_vector_increaseCapacity(FlatVector* vector);

uint8_t flat_vector_initialize(FlatVector* vector, uint32_t elementSize)
{
	if (vector == NULL || elementSize == 0)
		return false;

	vector->size = 0;
	vector->capacity = 0;

	vector->data = NULL;
	vector->dataElementSize = elementSize;

	return true;
}

uint8_t flat_vector_append(FlatVector* vector, void* value)
{
	if (vector == NULL || value == NULL)
		return false;

	WriteLog(LL_Debug, "size: %d cap: %d", vector->size, vector->capacity);

	// Validate that the next add will be in bounds, otherwise we need to increase the size
	if (vector->size >= vector->capacity)
	{
		WriteLog(LL_Warn, "here");

		if (!flat_vector_increaseCapacity(vector))
			return false;
	}

	WriteLog(LL_Warn, "here");

	void* vectorObject = vector->data + (vector->dataElementSize * vector->size);
	(void)memcpy(vectorObject, value, vector->dataElementSize);

	vector->size++;

	WriteLog(LL_Warn, "here");

	return true;
}

static uint8_t flat_vector_increaseCapacity(FlatVector* vector)
{
	if (vector == NULL)
		return false;

	uint32_t currentCapacity = vector->capacity;
	uint32_t wantedCapacity = vector->capacity + FLAT_VECTOR_INCREMENT_COUNT;

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
	vector->data = newArray;

	return true;
}

void* flat_vector_get(FlatVector* vector, uint32_t index)
{
	if (vector == NULL)
		return NULL;

	if (vector->data == NULL)
		return NULL;

	WriteLog(LL_Warn, "here");

	if (index >= vector->size)
		return NULL;

	WriteLog(LL_Warn, "here");

	return vector->data + (vector->dataElementSize * index);
}

uint8_t flat_vector_set(FlatVector* vector, uint32_t index, void* value)
{
	if (vector == NULL)
		return false;

	if (vector->data == NULL)
		return false;

	if (index >= vector->size)
		return false;

	WriteLog(LL_Warn, "here");

	void* vectorObject = vector->data + (vector->dataElementSize * index);
	(void)memset(vectorObject, 0, vector->dataElementSize);
	(void)memcpy(vectorObject, value, vector->dataElementSize);

	return true;
}

void flat_vector_free(FlatVector* vector)
{
	if (vector == NULL)
		return;

	WriteLog(LL_Warn, "here");

	// Free the data list
	if (vector->data)
	{
		k_free(vector->data);
		vector->data = NULL;
	}

	// Set cap & size to 0
	vector->capacity = 0;
	vector->size = 0;

	vector->dataElementSize = 0;
}