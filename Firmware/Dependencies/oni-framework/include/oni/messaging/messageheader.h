#pragma once
#include <oni/utils/types.h>

enum MessageCategory
{
	MessageCategory_None = 0,
	MessageCategory_System,
	MessageCategory_Log,
	MessageCategory_Debug,
	MessageCategory_File,
	MessageCategory_Command,
	MessageCategory_Max = 14
};

#define MESSAGEHEADER_MAGIC	2

struct messageheader_t
{
	uint64_t magic : 2;
	uint64_t category : 4;
	uint64_t isRequest : 1;
	uint64_t errorType : 32;
	uint64_t payloadLength : 16;
	uint64_t padding : 9;
};