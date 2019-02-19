#include <oni/messaging/messagecontainer.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/kdlsym.h>

struct messagecontainer_t* messagecontainer_allocOutgoing(enum MessageCategory category, uint32_t errorType, void* internalMessageData, size_t internalMessageSize)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);

	// Verify valid size
	if (internalMessageSize == 0 || internalMessageSize > MESSAGECONTAINER_MAXBUFSZ)
		return NULL;

	// Allocate memory for our reference counter
	size_t allocationSize = sizeof(struct messagecontainer_t) + internalMessageSize;

	struct messagecontainer_t* reference = (struct messagecontainer_t*)kmalloc(allocationSize);
	if (!reference)
		return NULL;

	// Zero reference
	(void)memset(reference, 0, allocationSize);

	reference->header.magic = MESSAGEHEADER_MAGIC;
	reference->header.category = category;
	reference->header.errorType = errorType;
	reference->header.isRequest = false;
	reference->header.payloadLength = internalMessageSize;

	reference->size = internalMessageSize;
	reference->count = 0;

	// Copy the payload to "ourself"
	(void)memcpy(reference->payload, internalMessageData, internalMessageSize);

	// Increment reference count
	messagecontainer_acquire(reference);

	return reference;
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