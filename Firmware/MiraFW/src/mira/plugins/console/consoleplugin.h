#pragma once
#define LOCK_PROFILING
#include <sys/param.h>
#include <oni/plugins/plugin.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#define MAX_CONSOLES	8
#define PORT_START		5000

struct console_t
{
	int32_t ttyDescriptor;
	int32_t socketDescriptor;

	uint16_t port;

	// This is a reference
	struct consoleplugin_t* plugin;

	void* consoleThread;

	volatile uint8_t isRunning;
};

struct consoleplugin_t
{
	struct plugin_t plugin;

	// Hold references to consoles
	struct console_t* consoles[MAX_CONSOLES];

	// Lock for the console references
	struct mtx mutex;
};

// console_t
struct console_t* consoleCreateConsole(struct consoleplugin_t* plugin, int32_t tty, int32_t socket);
void consoleDestroyConsole(struct console_t* console);

// consoleplugin_t
int32_t consoleplugin_getFreeIndex(struct consoleplugin_t* plugin);

void consoleplugin_init(struct consoleplugin_t* plugin);
uint8_t consoleplugin_load(struct consoleplugin_t* plugin);
uint8_t consoleplugin_unload(struct consoleplugin_t* plugin);


void consoleplugin_onClientDisconnect(struct consoleplugin_t* plugin, struct console_t* console);