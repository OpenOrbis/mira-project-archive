#pragma once
#include <oni/plugins/plugin.h>

struct filetransfer_plugin_t
{
	struct plugin_t plugin;
};

struct filetransfer_transfer_t
{
	uint64_t position;
	uint64_t length;
};

int32_t filetransfer_pluginInit(void* args);
int32_t filetransfer_pluginClose();

void filetransfer_plugin_init(struct filetransfer_plugin_t* plugin);