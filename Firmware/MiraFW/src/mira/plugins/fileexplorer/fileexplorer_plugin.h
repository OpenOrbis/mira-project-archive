#pragma once
#include <oni/plugins/plugin.h>

struct fileexplorer_plugin_t
{
	struct plugin_t plugin;
};

uint8_t fileexplorer_load(struct fileexplorer_plugin_t* plugin);
uint8_t fileexplorer_unload(struct fileexplorer_plugin_t* plugin);

void fileexplorer_plugin_init(struct fileexplorer_plugin_t* plugin);