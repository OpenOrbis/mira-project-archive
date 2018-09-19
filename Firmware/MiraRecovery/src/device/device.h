#pragma once
#include <oni/utils/types.h>
#include <sys/conf.h>

// Function Prototype for the device driver
d_open_t recovery_open;
d_close_t recovery_close;
d_read_t recovery_read;
d_write_t recovery_write;
d_ioctl_t recovery_ioctl;

struct miraframework_t;

enum recoveryctl
{
	// Read actions
	CtlDumpFlash,
	CtlDumpSram,
	CtlDumpBios,

	// Write actions
	CtlWriteFlash,
	CtlWriteSram,
	CtlWriteBios,

	// Oni actions
	CtlLoadFrameworkFromSocket,
	CtlLoadFrameworkFromFile,
	CtlLoadFrameworkFromMemory,

	CtlUnloadFramework,

	//
	CtlRunPayload,

	CtlEnableTrap,
};

void recovery_init_device();
void recovery_uninit_device();

// Initialize our down and dirty payload
extern uint8_t* gMiraPayload;
extern size_t gMiraPayloadSize;

extern struct miraframework_t* gMiraFramework;
