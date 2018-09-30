#pragma once
#include <oni/utils/types.h>

struct pbserver_t
{
	int32_t listenSocket;

	int32_t connectionSocket;

	boolean_t reuse;

	void* thread;
};

void pbserver_init(struct pbserver_t* server);