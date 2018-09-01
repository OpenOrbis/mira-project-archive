#pragma once
#include <oni/plugins/plugin.h>

struct ref_t;

struct appinstallerplugin_t
{
	struct plugin_t plugin;

	int(*sceAppInstUtilInitialize)(void);
	int(*sceAppInstUtilAppInstallPkg)(const char* file_path, int reserved);
	int(*sceAppInstUtilGetTitleIdFromPkg)(const char* pkg_path, char* title_id, int* is_app);
	int(*sceAppInstUtilAppPrepareOverwritePkg)(const char* pkg_path);
	int(*sceAppInstUtilGetPrimaryAppSlot)(const char* title_id, unsigned int* slot);
};

void appinstallerplugin_init(struct appinstallerplugin_t* plugin);

uint8_t appinstaller_load(struct appinstallerplugin_t * plugin);
uint8_t appinstaller_unload(struct appinstallerplugin_t * plugin);

//
//	Utility functions
//	Credits: flatz
//
int ffs_allocblocks(int fd, unsigned long size, unsigned int flags, unsigned int alignment);
int gsched_set_slot_prio(int fd, unsigned int slot, unsigned int prio, unsigned int* status);

//
//	Callbacks
//
void appinstaller_installPkg(struct ref_t* reference);
