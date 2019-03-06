#pragma once
#include <oni/plugins/plugin.h>

enum OrbisUtilsCommands
{
	OrbisUtils_GetHddKeys = 0x0411E4B9,
	OrbisUtils_ShutdownMira = 0x7FDF47D5,
	OrbisUtils_SetAslr = 0x554BB3F1,
};

struct orbisutils_plugin_t
{
	struct plugin_t plugin;
};

void orbisutils_init(struct orbisutils_plugin_t* plugin);