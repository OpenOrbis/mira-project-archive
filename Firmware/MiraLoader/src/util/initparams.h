#pragma once
#include <sys/types.h>

struct proc;

struct initparams_t
{
	// Payload base address, can be allocated dynamically
	uint64_t payloadBase;

	// Payload size
	uint64_t payloadSize;

	// Kernel process handle
	struct proc* process;

	// Entrypoint
	// Userland should set this to NULL
	void* entrypoint;
};
