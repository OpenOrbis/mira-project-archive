#pragma once
#include <oni/utils/types.h>
#include <netinet/in.h>

#include <sys/param.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#define PBSERVER_MAXCLIENTS	4
struct pbconnection_t;

struct pbserver_t
{
	// Server address
	struct sockaddr_in address;
	
	// Server socket
	int32_t socket;

	// Server thread
	void* thread;

	// If the server is currently running and accepting clients
	volatile boolean_t running;

	// Connections array
	struct pbconnection_t* connections[PBSERVER_MAXCLIENTS];

	// Lock for when we modify/enumerate connections
	struct mtx connectionsLock;
};

void pbserver_init(struct pbserver_t* server);

void pbserver_handleConnection(struct pbserver_t* server, struct pbconnection_t* connection);

int32_t pbserver_findFreeConnectionIndex(struct pbserver_t* server);

int32_t pbserver_findConnectionIndex(struct pbserver_t* server, struct pbconnection_t* connection);

int32_t pbserver_findSocketFromThread(struct pbserver_t* server, struct thread* td);

void pbserver_handleClientDisconnect(struct pbserver_t* server, struct pbconnection_t* connection);

uint8_t pbserver_startup(struct pbserver_t* server, uint16_t port);

uint8_t pbserver_shutdown(struct pbserver_t* server);