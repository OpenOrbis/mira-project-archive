#pragma once
#include <oni/utils/types.h>

struct hook_t;
struct thread;
struct proc;

#define OVERLAYFS_MAXPATH	2048

struct dmem_start_app_process_args
{
	char unknown0[0xB0];
	int32_t pid;
};

struct mount_entry_t
{
	int32_t pid;
	int32_t mountHandle;
	char* mountPoint;
	uint16_t mountPointLength;
};

struct overlayfs_t
{
	struct hook_t* execNewVmspaceHook;

	int32_t pid;

	void(*onProcessCtor)(struct overlayfs_t* fs, struct proc* proc);
	void(*onProcessDtor)(struct overlayfs_t* fs, struct proc* proc);
};

void overlayfs_init(struct overlayfs_t* fs);

void overlayfs_enable(struct overlayfs_t* fs);
void overlayfs_disable(struct overlayfs_t* fs);


//
// Helper functions
//

// This will search all usb drives for a _overlayfs file, and return this drive
char* overlayfs_findDrivePath(struct overlayfs_t* fs);

uint8_t overlayfs_fileExists(struct overlayfs_t* fs, char* path);
uint8_t overlayfs_isThreadInProc(struct overlayfs_t* fs, struct thread* td);

uint8_t overlayfs_isEnabled(struct overlayfs_t* fs);


struct map_entry_t* createMountEntry(int32_t processId, int32_t mountHandle, char* mountPoint);