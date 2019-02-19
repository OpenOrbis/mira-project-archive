#pragma once
#include <oni/utils/types.h>

#define RPCCATEGORY_MAX_CALLBACKS		256
#define RPCCATEGORY_MAX	16

struct ref_t;
struct messagecategory_callback_t;

struct messagecontainer_t;

struct messagecategory_t
{
	uint32_t category;
	struct messagecategory_callback_t* callbacks[RPCCATEGORY_MAX_CALLBACKS];
};

struct messagecategory_callback_t
{
	uint32_t type;
	void(*callback)(struct messagecontainer_t* container);
};

void rpccategory_init(struct messagecategory_t* dispatcherCategory, uint8_t category);
int32_t rpccategory_findFreeCallbackIndex(struct messagecategory_t* category);