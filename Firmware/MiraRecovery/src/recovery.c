#include <oni/utils/memory/install.h>
#include "device/device.h"

void* recovery_entry(void* args)
{
	SelfElevateAndRun((uint8_t*)0x926200000, 0x80000, recovery_init_device);

	return NULL;
}