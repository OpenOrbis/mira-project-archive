#pragma once
#include <oni/plugins/plugin.h>
#include <oni/utils/types.h>

struct example_plugin_t
{
	struct plugin_t plugin;
};

extern uint64_t plugin_size;

void plugin_initialize(void* plugin, struct plugininit_t* arg);

// Plugin initialization prototype
uint8_t plugin_load(void* plugin);

// Plugin close prototype
uint8_t plugin_unload(void* plugin);
