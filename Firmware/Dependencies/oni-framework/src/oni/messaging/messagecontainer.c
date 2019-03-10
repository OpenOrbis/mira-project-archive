#include <oni/messaging/messagecontainer.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/logger.h>

struct messagecontainer_t* messagecontainer_createMessage(enum MessageCategory category, uint32_t errorType, uint8_t isRequest, void* payload, uint32_t payloadLength)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);

	// Validate that our payload will fit
	if (payloadLength > MESSAGECONTAINER_MAXBUFSZ)
	{
		WriteLog(LL_Error, "requested size (%d) > max size (%d).", payloadLength, MESSAGECONTAINER_MAXBUFSZ);
		return NULL;
	}

	// Calculate the allocation size which is size of header + payloadLength
	uint32_t allocationSize = sizeof(struct messagecontainer_t) + payloadLength;
	struct messagecontainer_t* messageContainer = kmalloc(allocationSize);
	if (messageContainer == NULL)
	{
		WriteLog(LL_Error, "could not allocate messageContainer payloadLength: (%d).", payloadLength);
		return NULL;
	}
	(void)memset(messageContainer, 0, allocationSize);

	// Set all of the header values
	messageContainer->header.magic = MESSAGEHEADER_MAGIC;
	messageContainer->header.category = category;
	messageContainer->header.isRequest = isRequest;
	messageContainer->header.errorType = errorType;
	messageContainer->header.payloadLength = payloadLength;
	messageContainer->header.padding = 0;

	messageContainer->count = 0;
	messageContainer->size = payloadLength;

	// Copy over the payload if we have one
	if (payload != NULL && payloadLength > 0)
		(void)memcpy(messageContainer->payload, payload, payloadLength);

	// Allocate a reference so the user does not have to acquire it automatically
	messagecontainer_acquire(messageContainer);

	return messageContainer;
}

// this does not parse header etc, this only creates from payload then sends off, you will need to set header info
//struct messagecontainer_t* messagecontainer_allocIncoming(void* internalMessageData, size_t internalMessageSize)
//{
//	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
//	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);
//
//	// Verify valid size
//	if (internalMessageSize == 0)
//		return NULL;
//
//	// Allocate memory for our reference counter
//	size_t allocationSize = sizeof(struct messagecontainer_t) + internalMessageSize;
//
//	struct messagecontainer_t* reference = (struct messagecontainer_t*)kmalloc(allocationSize);
//	if (!reference)
//		return NULL;
//
//	// Zero reference
//	memset(reference, 0, allocationSize);
//
//	// Copy to ourselves
//	memcpy(reference->payload, internalMessageData, internalMessageSize);
//
//	reference->size = internalMessageSize;
//	reference->count = 0;
//
//	// Increment reference count
//	capncontainer_acquire(reference);
//
//	return reference;
//}

void messagecontainer_acquire(struct messagecontainer_t* reference)
{
	if (!reference)
		return;

	__sync_fetch_and_add(&reference->count, 1);
}

void messagecontainer_release(struct messagecontainer_t* reference)
{
	if (!reference)
		return;

	// If we subtract and the reference count is zero, free the reference and data
	if (__sync_sub_and_fetch(&reference->count, 1) == 0)
	{
		uint64_t allocationSize = sizeof(struct messagecontainer_t) + reference->size;

		// Free the entire reference header + data afterwards
		kfree(reference, allocationSize);
	}
}