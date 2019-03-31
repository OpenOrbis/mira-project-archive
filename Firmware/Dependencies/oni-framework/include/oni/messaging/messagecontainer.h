#pragma once
#include <oni/utils/types.h>
#include "messageheader.h"

#define MESSAGECONTAINER_MAXBUFSZ	0x8000

struct messagecontainer_t
{
	// Current reference count
	volatile uint64_t count;

	// Message header
	struct messageheader_t header;

	// Message Payload
	uint8_t payload[];
};

struct messagecontainer_t* messagecontainer_createMessage(enum MessageCategory category, uint32_t errorType, uint8_t isRequest, void* payload, uint32_t payloadLength);

void messagecontainer_acquire(struct messagecontainer_t* reference);

void messagecontainer_release(struct messagecontainer_t* reference);