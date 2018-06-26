#pragma once
#include <oni/utils/types.h>

struct hook_t;
struct thread;
struct proc;

#define OVERLAYFS_MAXPATH	4096

struct overlayfs_t
{
	// The current redirect path
	char redirectPath[OVERLAYFS_MAXPATH];

	struct hook_t* dmemStartAppProcessHook;
	struct hook_t* openHook;
	struct hook_t* closeHook;
	struct hook_t* readHook;
	struct hook_t* writeHook;

	void(*onProcessCtor)(struct overlayfs_t* fs, struct proc* proc);
	void(*onProcessDtor)(struct overlayfs_t* fs, struct proc* proc);

	struct proc* currentProc;
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