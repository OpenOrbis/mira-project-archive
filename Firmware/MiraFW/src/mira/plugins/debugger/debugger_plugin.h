#pragma once
#include <oni/plugins/plugin.h>

struct debugger_plugin_t
{
	struct plugin_t plugin;
};

uint8_t debugger_load(struct debugger_plugin_t* plugin);
uint8_t debugger_unload(struct debugger_plugin_t* plugin);

void debugger_plugin_init(struct debugger_plugin_t* plugin);