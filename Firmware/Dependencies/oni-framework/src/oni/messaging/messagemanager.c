#include <oni/messaging/messagemanager.h>
#include <oni/messaging/messagecategory.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/logger.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/ref.h>

#include <oni/framework.h>
#include <oni/rpc/rpcserver.h>

#include <oni/messaging/messagecontainer.h>

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

void messagemanager_sendErrorResponse(enum MessageCategory category, int32_t error)
{
	struct messagecontainer_t* responseContainer = messagecontainer_createMessage(category, (uint32_t)error, false, NULL, 0);
	if (responseContainer == NULL)
	{
		WriteLog(LL_Error, "could not allocate response");
		return;
	}

	messagemanager_sendResponse(responseContainer);

	messagecontainer_release(responseContainer);
}

void messagemanager_sendResponse(struct messagecontainer_t* container)
{
	//void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	if (!container)
		return;

	if (gFramework == NULL || gFramework->rpcServer == NULL)
		return;

	int32_t connectionSocket = rpcserver_findSocketFromThread(gFramework->rpcServer, curthread);

	//WriteLog(LL_Debug, "connection socket found: %d", connectionSocket);
	if (connectionSocket < 0)
		return;

	// Acquire a reference, from now on we need to goto cleanup
	messagecontainer_acquire(container);

#ifdef _DEBUG
	if (sizeof(&container->header) != sizeof(uint64_t))
		WriteLog(LL_Debug, "sizeof(header): %x sizeof(uint64): %x, fix yer code", sizeof(&container->header), sizeof(uint64_t));
#endif

	// Write the header
	int32_t ret = kwrite(connectionSocket, &container->header, sizeof(container->header));
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not write message (%p) (%llx).", &container->header, sizeof(container->header));
		goto cleanup;
	}

	// Write the payload
	ret = kwrite(connectionSocket, container->payload, container->header.payloadLength);
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not write message (%p) (%llx).", container->payload, container->header.payloadLength);
		goto cleanup;
	}

cleanup:
	// Release the container reference
	messagecontainer_release(container);
}

void messagemanager_sendRequest(struct messagecontainer_t* container)
{
	// Verify the message manager and message are valid
	if (!gFramework || !gFramework->messageManager || !container)
		return;

	//WriteLog(LL_Debug, "container: %p", container);

	messagecontainer_acquire(container);

	//WriteLog(LL_Debug, "here");

	struct messagemanager_t* manager = gFramework->messageManager;
	//WriteLog(LL_Debug, "manager: %p", manager);

	// Validate our message category
	enum MessageCategory headerCategory = container->header.category;
	if (headerCategory < 0 || headerCategory >= MessageCategory_Max)
	{
		WriteLog(LL_Error, "[-] invalid message category: %d max: %d", headerCategory, MessageCategory_Max);
		goto cleanup;
	}

	//WriteLog(LL_Debug, "headerCategory: %d", headerCategory);

	struct messagecategory_t* category = messagemanager_getCategory(manager, headerCategory);
	if (!category)
	{
		WriteLog(LL_Debug, "[-] could not get dispatcher category");
		goto cleanup;
	}

	//WriteLog(LL_Debug, "category: %p", category);

	// Iterate through all of the callbacks
	for (uint32_t l_CallbackIndex = 0; l_CallbackIndex < RPCCATEGORY_MAX_CALLBACKS; ++l_CallbackIndex)
	{
		// Get the category callback structure
		struct messagecategory_callback_t* l_Callback = category->callbacks[l_CallbackIndex];

		// Check to see if this callback is used at all
		if (!l_Callback)
			continue;

		// Check the type of the message
		if (l_Callback->type != container->header.errorType)
			continue;

		if (l_Callback->callback == NULL)
			continue;

		// Call the callback with the provided message
		//WriteLog(LL_Debug, "[+] calling callback %p(%p)", l_Callback->callback, container);
		l_Callback->callback(container);
	}

cleanup:		
	messagecontainer_release(container);
}