#pragma once
#include <oni/plugins/plugin.h>

struct orbisutils_plugin_t
{
	struct plugin_t plugin;
};

void orbisutils_init(struct orbisutils_plugin_t* plugin);