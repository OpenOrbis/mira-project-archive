#pragma once
#include <oni/utils/types.h>
#include <netinet/in.h>

#include <sys/mutex.h>

typedef void(*OnClientDisconnect_t)(struct pbserver_t* server, struct pbconnection_t* connection);

struct pbconnection_t
{
	int32_t socket;

	void* thread;

	struct sockaddr_in address;

	struct mtx lock;

	// Callback information
	struct pbserver_t* server;

	volatile boolean_t running;

	OnClientDisconnect_t onClientDisconnect;
};

void pbconnection_init(struct pbconnection_t* connection);

void pbconnection_thread(struct pbconnection_t* connection);
