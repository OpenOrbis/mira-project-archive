#pragma once
#include <oni/utils/types.h>

struct thread;
struct proc;
struct ctlrunpayload_t;

struct ctlrunpayload_t
{
	// Should we execute this payload as userland or kernel proc
	uint8_t executeAsUserland;

	// Payload data start
	uint8_t* payloadStart;
	/*
		The payload data MUST have the entrypoint at offset 0
		jmp main_function
	*/

	// Payload data length
	size_t payloadSize;
};

int ctlrunpayload(struct thread* td, struct ctlrunpayload_t* payloadInfo);