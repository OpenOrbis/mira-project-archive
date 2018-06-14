#pragma once
#include <oni/utils/types.h>

struct trapframe;

void trap_fatal_hook(struct trapframe* frame, vm_offset_t eva);

extern struct hook_t* gTrapQueen;
extern uint8_t gTrappin;