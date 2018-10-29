#pragma once
#include <sys/types.h>

int64_t sys_dynlib_load_prx(char* prxPath, int* outModuleId);
int64_t sys_dynlib_unload_prx(int64_t prxID);
int64_t sys_dynlib_dlsym(int64_t moduleHandle, const char* functionName, void *destFuncOffset);