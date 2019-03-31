#pragma once
#include <oni/utils/types.h>

struct orbisutils_shutdownMiraRequest_t
{
	uint8_t shutdownMira;
	uint8_t rebootConsole;
};

struct orbisutils_dumpHddKeysRequest_t
{
	uint8_t encrypted[0x60];
	uint8_t key[0x20];
};

struct orbisutils_toggleAslrRequest_t
{
	uint8_t aslrEnabled;
};