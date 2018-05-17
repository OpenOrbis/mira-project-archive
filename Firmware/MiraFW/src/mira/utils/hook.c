#include "hook.h"
#include "hde/hde64.h"
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/memory/allocator.h>

#include <oni/utils/cpu.h>
#include <oni/utils/kdlsym.h>

#define HOOK_LENGTH	14

static int32_t hook_getMinHookSize(void* targetFunction)
{
	hde64s hs;

	uint32_t hookSize = HOOK_LENGTH;
	uint32_t totalLength = 0;

	
	while (totalLength < hookSize)
	{
		uint32_t length = hde64_disasm(targetFunction, &hs);
		if (hs.flags & F_ERROR)
			return -1;

		totalLength += length;
	}

	return totalLength;
}

struct hook_t* hook_create(void* targetFunction, void* functionHook)
{
	if (!targetFunction || !functionHook)
		return NULL;

	// Allocate a new hook structure to be returned
	struct hook_t* hook = (struct hook_t*)kmalloc(sizeof(struct hook_t));
	if (!hook)
		return NULL;

	// Zero out the buffer
	kmemset(hook, 0, sizeof(*hook));

	int32_t backupDataLength = hook_getMinHookSize(targetFunction);
	if (backupDataLength == -1)
	{
		kfree(hook, sizeof(*hook));
		return NULL;
	}

	// Allocate our backup bytes data
	uint8_t* backupData = (uint8_t*)kmalloc(backupDataLength);
	if (!backupData)
	{
		kfree(hook, sizeof(*hook));
		return NULL;
	}

	// Zero out and copy the beginning of the function
	kmemset(backupData, 0, backupDataLength);
	kmemcpy(backupData, targetFunction, backupDataLength);

	hook->targetAddress = targetFunction;
	hook->hookAddress = functionHook;
	hook->backupData = backupData;
	hook->backupLength = backupDataLength;
	hook->enabled = false;

	return hook;
}

void hook_enable(struct hook_t* hook)
{
	if (!hook)
		return;

	if (!hook->hookAddress)
		return;

	if (!hook->targetAddress)
		return;

	void(*critical_enter)(void) = kdlsym(critical_enter);
	void(*critical_exit)(void) = kdlsym(critical_exit);

	int8_t jumpBuffer[] = {
		0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,				// # jmp    QWORD PTR [rip+0x0]
		0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,	// # DQ: AbsoluteAddress
	}; // Shit takes 14 bytes

	uint64_t* jumpBufferAddress = (uint64_t*)(jumpBuffer + 6);

	// Assign the address
	*jumpBufferAddress = (uint64_t)hook->hookAddress;

	// Change permissions and apply the hook
	critical_enter();
	cpu_disable_wp();
	kmemcpy(hook->targetAddress, jumpBuffer, sizeof(jumpBuffer));
	cpu_enable_wp();
	critical_exit();

	hook->enabled = true;
}

void hook_disable(struct hook_t* hook)
{
	if (!hook)
		return;

	if (!hook->hookAddress)
		return;

	if (!hook->targetAddress)
		return;

	if (!hook->backupData || hook->backupLength == 0)
		return;

	void(*critical_enter)(void) = kdlsym(critical_enter);
	void(*critical_exit)(void) = kdlsym(critical_exit);

	// Change permissions and apply the hook
	critical_enter();
	cpu_disable_wp();
	kmemcpy(hook->targetAddress, hook->backupData, hook->backupLength);
	cpu_enable_wp();
	critical_exit();

	hook->enabled = false;
}

void* hook_getFunctionAddress(struct hook_t* hook)
{
	if (!hook)
		return NULL;

	return hook->targetAddress;
}