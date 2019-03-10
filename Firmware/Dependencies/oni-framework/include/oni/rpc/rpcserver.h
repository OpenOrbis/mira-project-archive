#pragma once
#include <oni/utils/types.h>
#include <netinet/in.h>

#include <sys/param.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#define rpcserver_MAXCLIENTS	256

struct rpcconnection_t;

struct rpcserver_t
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
	struct rpcconnection_t* connections[rpcserver_MAXCLIENTS];

	// Lock for when we modify/enumerate connections
	struct mtx connectionsLock;
};

void rpcserver_init(struct rpcserver_t* server);

void rpcserver_handleConnection(struct rpcserver_t* server, struct rpcconnection_t* connection);

int32_t rpcserver_findFreeConnectionIndex(struct rpcserver_t* server);

int32_t rpcserver_findConnectionIndex(struct rpcserver_t* server, struct rpcconnection_t* connection);

int32_t rpcserver_findSocketFromThread(struct rpcserver_t* server, struct thread* td);

void rpcserver_handleClientDisconnect(struct rpcserver_t* server, struct rpcconnection_t* connection);

uint8_t rpcserver_startup(struct rpcserver_t* server, uint16_t port);

uint8_t rpcserver_shutdown(struct rpcserver_t* server);