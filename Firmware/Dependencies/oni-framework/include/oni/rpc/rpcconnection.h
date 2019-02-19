#pragma once
#include <oni/utils/types.h>
#include <netinet/in.h>
#include <sys/param.h>
#include <sys/lock.h>
#include <sys/mutex.h>

struct rpcconnection_t;
struct rpcserver_t;
struct thread;

typedef void(*OnClientDisconnect_t)(struct rpcserver_t* server, struct rpcconnection_t* connection);

struct rpcconnection_t
{
	int32_t socket;

	struct thread* thread;

	struct sockaddr_in address;

	struct mtx lock;

	// Callback information
	struct rpcserver_t* server;

	volatile boolean_t running;

	OnClientDisconnect_t onClientDisconnect;
};

void rpcconnection_init(struct rpcconnection_t* connection);

void rpcconnection_thread(struct rpcconnection_t* connection);
