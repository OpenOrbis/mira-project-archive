#pragma once
#include <oni/plugins/plugin.h>
#include <netinet/in.h>

struct gdbstub_plugin_t
{
	struct plugin_t plugin;

	// Server address
	struct sockaddr_in address;

	// thread
	void* thread;

	uint8_t running;

	int32_t socket;
	uint16_t port;
};

uint8_t gdbstub_load(struct gdbstub_plugin_t* plugin);
uint8_t gdbstub_unload(struct gdbstub_plugin_t* plugin);

void gdbstub_plugin_init(struct gdbstub_plugin_t* plugin);