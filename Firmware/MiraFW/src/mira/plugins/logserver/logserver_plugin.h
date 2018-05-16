#pragma once
#include <oni/plugins/plugin.h>
#include <netinet/in.h>

#define LOGSERVER_MAXCLIENTS	8
#define LOGSERVER_BUFFERSIZE	0x1000

struct logserver_plugin_t;

typedef void(*onLogClientDisconnect_t)(struct logserver_plugin_t*, struct logserver_client_t*);

struct logserver_client_t
{
	int32_t socket;

	struct thread* thread;

	// Buffer
	uint8_t buffer[LOGSERVER_BUFFERSIZE];

	// Address of the client
	struct sockaddr_in address;
	
	onLogClientDisconnect_t disconnect;

	struct loginserver_plugin_t* server;
};

struct logserver_plugin_t
{
	struct plugin_t plugin;

	struct sockaddr_in address;
	int32_t socket;
	uint16_t port;

	int isRunning;
	struct thread* thread;

};

void logserver_init(struct logserver_plugin_t* plugin);

void logserver_serverThread(void* data);