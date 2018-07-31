#pragma once
#include <oni/utils/types.h>

uint8_t injector_injectModule(int32_t pid, uint8_t* moduleData, uint32_t moduleSize, uint8_t newThread);