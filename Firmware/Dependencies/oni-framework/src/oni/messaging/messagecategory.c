
#include <oni/messaging/messagecategory.h>

#include <oni/utils/kdlsym.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/ref.h>


void rpccategory_init(struct messagecategory_t* dispatcherCategory, uint8_t category)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	if (!dispatcherCategory)
		return;

	if (category >= RPCCATEGORY_MAX)
		return;

	dispatcherCategory->category = category;

	memset(dispatcherCategory->callbacks, 0, sizeof(dispatcherCategory->callbacks));
}

int32_t rpccategory_findFreeCallbackIndex(struct messagecategory_t* category)
{
	if (!category)
		return -1;

	for (uint32_t i = 0; i < RPCCATEGORY_MAX_CALLBACKS; ++i)
	{
		if (!category->callbacks[i])
			return i;
	}

	return -1;
}