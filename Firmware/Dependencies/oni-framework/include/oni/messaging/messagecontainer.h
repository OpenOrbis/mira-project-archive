#pragma once
#include <oni/utils/types.h>
#include "messageheader.h"

#define MESSAGECONTAINER_MAXBUFSZ	0x8000

struct messagecontainer_t
{
	uint64_t size;

	volatile uint64_t count;

	struct messageheader_t header;

	// payload
	uint8_t payload[];
};

struct messagecontainer_t* messagecontainer_allocOutgoing(enum MessageCategory category, uint32_t type, void* internalMessageData, size_t internalMessageSize);
struct messagecontainer_t* messagecontainer_allocIncoming(void* internalMessageData, size_t internalMessageSize);


void messagecontainer_acquire(struct messagecontainer_t* reference);

void messagecontainer_release(struct messagecontainer_t* reference);