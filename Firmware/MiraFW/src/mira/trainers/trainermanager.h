#pragma once
#include <oni/utils/types.h>

struct trainerblock_t
{
	uint64_t trainerSize;
	void(*trainerEntry)();
};

struct trainermanager_t
{

};

uint8_t trainermanager_injectTrainer(struct trainerblock_t* trainerBlock);