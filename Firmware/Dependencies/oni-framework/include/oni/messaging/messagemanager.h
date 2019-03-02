#pragma once
#include <oni/utils/types.h>
#include <sys/param.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#define RPCDISPATCHER_MAX_CATEGORIES	32

struct messagecategory_t;
struct messagecontainer_t;
enum MessageCategory;

struct messagemanager_t
{
	struct messagecategory_t* categories[RPCDISPATCHER_MAX_CATEGORIES];

	struct mtx lock;
};

void messagemanager_init(struct messagemanager_t* manager);

// Utility functions
int32_t messagemanager_findFreeCategoryIndex(struct messagemanager_t* manager);
uint32_t messagemanager_freeCategoryCount(struct messagemanager_t* manager);
struct messagecategory_t* messagemanager_getCategory(struct messagemanager_t* manager, uint32_t categoryId);

// Registration and unregistration
int32_t messagemanager_registerCallback(struct messagemanager_t* manager, uint32_t callbackCategory, uint32_t callbackType, void* callback);
int32_t messagemanager_unregisterCallback(struct messagemanager_t* manager, int32_t callbackCategory, int32_t callbackType, void* callback);

// Sents a request
void messagemanager_sendRequest(struct messagecontainer_t* container);

// Sends an error response with no payload back to the socket
void messagemanager_sendErrorResponse(enum MessageCategory category, int32_t error);

// Sends the reply back to the socket
void messagemanager_sendResponse(struct messagecontainer_t* message);