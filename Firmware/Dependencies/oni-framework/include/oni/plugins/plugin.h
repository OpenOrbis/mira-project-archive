#pragma once
#include <oni/utils/types.h>

// Plugin name length
#define PLUGIN_NAME_LEN		64

// Plugin description length
#define PLUGIN_DESC_LEN		256

struct plugin_bitmap_t
{
	const uint8_t* data;
	const uint32_t dataLength;
	const uint32_t width;
	const uint32_t height;
};

struct plugin_t
/*
	plugin_t

	Plugin structure that all plugins should "inherit" from
*/
{
	// Name of the plugin
	const char* name;

	// Description of the plugin
	const char* description;

	// Plugin initialization prototype
	uint8_t(*plugin_load)(void* plugin);

	// Plugin close prototype
	uint8_t(*plugin_unload)(void* plugin);
};

struct plugininit_t
/*
	plugininit_t

	This structure is used for passing "host" information to the plugins to use
*/
{
	struct framework_t* framework;
	struct logger_t* logger;
	uint8_t* kernelBase;
};