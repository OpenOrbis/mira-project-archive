#pragma once
#include <oni/utils/types.h>

#include <oni/utils/log/logger.h>

struct pluginheader_t
{
	uint64_t initializeOffset;
	uint64_t pluginSizeOffset;
};

struct loaderplugin_t
{
	char path[260];

	uint8_t* data;
	uint32_t dataLength;

	void* plugin;
};

/*
	pluginloader_t

	This will eventually be merged into the pluginmanager_t class
*/
struct pluginloader_t
{
	// Plugin directory path
	char* pluginDirectory;
	uint32_t pluginDirectoryLength;

	struct loaderplugin_t** pluginList;
	uint32_t pluginCount;
};

void pluginloader_init(struct pluginloader_t* loader);

uint32_t pluginloader_getAvailablePluginCount(struct pluginloader_t* loader);
struct loaderplugin_t* pluginloader_loadPluginFromFile(struct pluginloader_t* loader, char* pluginPath);
void pluginloader_loadPlugins(struct pluginloader_t* loader);

