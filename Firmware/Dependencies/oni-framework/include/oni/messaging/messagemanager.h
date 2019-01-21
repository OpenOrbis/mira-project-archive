#pragma once
#include <oni/utils/types.h>
#include <sys/param.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#define RPCDISPATCHER_MAX_CATEGORIES	32

struct messagecategory_t;
struct pbcontainer_t;
typedef struct pbcontainer_t PbContainer;

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
void messagemanager_sendRequest(PbContainer* container);

// Sends the reply back to the socket
void messagemanager_sendResponse(PbContainer* container);