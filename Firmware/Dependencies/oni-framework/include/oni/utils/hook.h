#pragma once
#include <oni/utils/types.h>

struct hook_t
{
	// Address of the target function
	void* targetAddress;

	// Address of the hook
	void* hookAddress;

	// If this hook is enabled
	uint8_t enabled;

	// The backup data length for the jmp overwrite
	uint32_t backupLength;
	uint8_t* backupData;
};

struct hook_t* hook_create(void* targetFunction, void* functionHook);

void hook_enable(struct hook_t* hook);
void hook_disable(struct hook_t* hook);

void* hook_getFunctionAddress(struct hook_t* hook);