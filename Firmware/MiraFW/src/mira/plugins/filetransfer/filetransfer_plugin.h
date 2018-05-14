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

uint8_t filetransfer_load(struct filetransfer_plugin_t* plugin);
uint8_t filetransfer_unload(struct filetransfer_plugin_t* plugin);

void filetransfer_plugin_init(struct filetransfer_plugin_t* plugin);