#include <oni/messaging/messagemanager.h>
#include <oni/messaging/messagecategory.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/logger.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/ref.h>

#include <oni/framework.h>
#include <oni/rpc/pbserver.h>

void messagemanager_init(struct messagemanager_t* manager)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	if (!manager)
		return;

	void(*mtx_init)(struct mtx *m, const char *name, const char *type, int opts) = kdlsym(mtx_init);
	mtx_init(&manager->lock, "mmmtx", NULL, 0);

	memset(manager->categories, 0, sizeof(manager->categories));
}

int32_t messagemanager_findFreeCategoryIndex(struct messagemanager_t* manager)
{
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	if (!manager)
		return -1;

	int32_t result = -1;

	_mtx_lock_flags(&manager->lock, 0, __FILE__, __LINE__);
	for (uint32_t i = 0; i < RPCDISPATCHER_MAX_CATEGORIES; ++i)
	{
		if (!manager->categories[i])
		{
			result = i;
			break;
		}
	}
	_mtx_unlock_flags(&manager->lock, 0, __FILE__, __LINE__);

	return result;
}

uint32_t messagemanager_freeCategoryCount(struct messagemanager_t* manager)
{
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	if (!manager)
		return 0;

	uint32_t clientCount = 0;
	_mtx_lock_flags(&manager->lock,0,  __FILE__, __LINE__);
	for (uint32_t i = 0; i < RPCDISPATCHER_MAX_CATEGORIES; ++i)
	{
		if (manager->categories[i])
			clientCount++;
	}
	_mtx_unlock_flags(&manager->lock, 0, __FILE__, __LINE__);

	return clientCount;
}

struct messagecategory_t* messagemanager_getCategory(struct messagemanager_t* manager, uint32_t category)
{
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	if (!manager)
		return NULL;

	if (category >= RPCCATEGORY_MAX)
		return NULL;

	struct messagecategory_t* rpccategory = 0;

	_mtx_lock_flags(&manager->lock, 0, __FILE__, __LINE__);
	for (uint32_t i = 0; i < RPCDISPATCHER_MAX_CATEGORIES; ++i)
	{
		struct messagecategory_t* cat = manager->categories[i];

		if (!cat)
			continue;

		if (cat->category == category)
		{
			rpccategory = manager->categories[i];
			break;
		}
	}
	_mtx_unlock_flags(&manager->lock, 0, __FILE__, __LINE__);

	return rpccategory;
}

int32_t messagemanager_registerCallback(struct messagemanager_t* manager, uint32_t callbackCategory, uint32_t callbackType, void* callback)
{
	// Verify that the dispatcher and listener are valid
	if (!manager || !callback)
		return 0;

	// Verify the listener category
	if (callbackCategory >= RPCCATEGORY_MAX)
		return 0;

	struct messagecategory_t* category = messagemanager_getCategory(manager, callbackCategory);
	if (!category)
	{
		// Get a free listener index
		int32_t freeIndex = messagemanager_findFreeCategoryIndex(manager);
		if (freeIndex == -1)
			return 0;

		// Allocate a new category
		category = (struct messagecategory_t*)kmalloc(sizeof(struct messagecategory_t));
		if (!category)
			return 0;

		// Initialize the category
		rpccategory_init(category, callbackCategory);

		// Set the category in our list
		manager->categories[freeIndex] = category;
	}

	// Get the next free listener
	int32_t callbackIndex = rpccategory_findFreeCallbackIndex(category);
	if (callbackIndex == -1)
		return 0;

	// Install the listener to the category
	struct messagecategory_callback_t* categoryCallback = (struct messagecategory_callback_t*)kmalloc(sizeof(struct messagecategory_callback_t));
	if (!categoryCallback)
		return 0;

	// Set the type and callback
	categoryCallback->type = callbackType;
	categoryCallback->callback = callback;

	// Install our callback
	category->callbacks[callbackIndex] = categoryCallback;

	return 1;
}

int32_t messagemanager_unregisterCallback(struct messagemanager_t* manager, int32_t callbackCategory, int32_t callbackType, void* callback)
{
	// Verify that the dispatcher and listener are valid
	if (!manager || !callback)
		return false;

	// Verify the listener category
	if (callbackCategory >= RPCCATEGORY_MAX)
		return false;

	struct messagecategory_t* category = messagemanager_getCategory(manager, callbackCategory);
	if (!category)
		return false;

	for (uint32_t l_CallbackIndex = 0; l_CallbackIndex < ARRAYSIZE(category->callbacks); ++l_CallbackIndex)
	{
		// Get the category callback structure
		struct messagecategory_callback_t* l_Callback = category->callbacks[l_CallbackIndex];

		// Check to see if this callback is used at all
		if (!l_Callback)
			continue;

		// Check the type of the message
		if (l_Callback->type != callbackType)
			continue;

		// Check the callback
		if (l_Callback->callback != callback)
			continue;

		// Remove from the list
		category->callbacks[l_CallbackIndex] = NULL;

		// Free the memory
		kfree(l_Callback, sizeof(*l_Callback));

		l_Callback = NULL;

		return true;
	}

	return false;
}

void messagemanager_sendResponse(PbContainer* container)
/*
	messagemanager_sendResponse

	msg - Reference counted message_header_t

	This function send the message header and then the payload data
*/
{
//	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
//
//	if (!container)
//		return;
//
//	if (!container->message)
//		return;
//
//	struct pbserver_t* rpcServer = gFramework->rpcServer;
//
//	int32_t connectionSocket = pbserver_findSocketFromThread(rpcServer, curthread);
//
//	//WriteLog(LL_Debug, "connection socket found: %d", connectionSocket);
//	if (connectionSocket < 0)
//		return;
//
//	// Acquire a reference, from now on we need to goto cleanup
//	pbcontainer_acquire(container);
//
//	PbMessage* message = container->message;
//
//	// Get the size of the packed message
//	uint64_t messageSize = pb_message__get_packed_size(message);
//	uint64_t maxMessageSize = PAGE_SIZE * 2;
//	uint8_t* messageData = NULL;
//	if (messageSize >= maxMessageSize)
//	{
//		WriteLog(LL_Error, "message size (%llx) is greater than max (%llx).", messageSize, maxMessageSize);
//		goto cleanup;
//	}
//
//	// Allocate data to send
//	messageData = k_malloc(messageSize);
//	if (!messageData)
//	{
//		WriteLog(LL_Error, "could not allocate message data");
//		goto cleanup;
//	}
//	memset(messageData, 0, messageSize);
//
//	// Pack the message
//	uint32_t packedMessageSize = (uint32_t)pb_message__pack(message, messageData);
//	if(messageSize != packedMessageSize)
//	{
//		WriteLog(LL_Error, "message size (%llx) does not match packed message size (%llx).", messageSize, packedMessageSize);
//		goto cleanup;
//	}
//
//	// We need to write the 8 byte uint64_t
//	ssize_t ret = kwrite(connectionSocket, &packedMessageSize, sizeof(packedMessageSize));
//	if (ret < 0)
//	{
//		WriteLog(LL_Error, "could not write message size.");
//		goto cleanup;
//	}
//
//	// Write the actual data
//	ret = kwrite(connectionSocket, messageData, messageSize);
//	if (ret < 0)
//	{
//		WriteLog(LL_Error, "could not write message (%p) (%llx).", messageData, messageSize);
//		goto cleanup;
//	}
//
//cleanup:
//	// Free the message data
//	if (messageData)
//		k_free(messageData);
//
//	// Release the container reference
//	pbcontainer_release(container);
}

void messagemanager_sendRequest(PbContainer* container)
{
//	// Verify the message manager and message are valid
//	if (!gFramework || !gFramework->messageManager || !container)
//		return;
//
//	if (!container->message)
//		return;
//
//	pbcontainer_acquire(container);
//
//	PbMessage* header = container->message;
//
//	//int32_t connectionSocket = pbserver_findSocketFromThread(gFramework->rpcServer, curthread);
//
//	//WriteLog(LL_Debug, "connection socket found: %d", connectionSocket);
//	//if (connectionSocket < 0)
//	//	goto cleanup;
//
//	struct messagemanager_t* manager = gFramework->messageManager;
//
//	// Validate our message category
//	MessageCategory headerCategory = header->category;
//	if (headerCategory >= MESSAGE_CATEGORY__MAX)
//	{
//		WriteLog(LL_Error, "[-] invalid message category: %d max: %d", headerCategory, MESSAGE_CATEGORY__MAX);
//		goto cleanup;
//	}
//
//	struct messagecategory_t* category = messagemanager_getCategory(manager, headerCategory);
//	if (!category)
//	{
//		WriteLog(LL_Debug, "[-] could not get dispatcher category");
//		goto cleanup;
//	}
//
//	// Iterate through all of the callbacks
//	for (uint32_t l_CallbackIndex = 0; l_CallbackIndex < RPCCATEGORY_MAX_CALLBACKS; ++l_CallbackIndex)
//	{
//		// Get the category callback structure
//		struct messagecategory_callback_t* l_Callback = category->callbacks[l_CallbackIndex];
//
//		// Check to see if this callback is used at all
//		if (!l_Callback)
//			continue;
//
//		// Check the type of the message
//		if (l_Callback->type != header->type)
//			continue;
//
//		// Call the callback with the provided message
//		//WriteLog(LL_Debug, "[+] calling callback %p(%p)", l_Callback->callback, container);
//		l_Callback->callback(container);
//	}
//
//cleanup:		
//	pbcontainer_release(container);
}