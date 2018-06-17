#include "pluginloader.h"
#include <oni/framework.h>

#include <oni/plugins/plugin.h>
#include <oni/plugins/pluginmanager.h>

#include <oni/utils/memory/allocator.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/kdlsym.h>

#include <sys/dirent.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>
#include <sys/stat.h>

#define	PLUGIN_MAXSIZE	0x200000	// 2MB hard cap

static struct plugininit_t gLoaderPluginInit;

static uint8_t pluginloader_addPluginToList(struct pluginloader_t* loader, struct loaderplugin_t* pluginEntry);

struct loaderplugin_t* pluginloader_loadPluginFromFile(struct pluginloader_t* loader, char* path);

size_t strlen(const char * str)
{
	const char *s;
	for (s = str; *s; ++s) {}
	return(s - str);
}

int strcmp(const char * s1, const char * s2)
{
	for (; *s1 == *s2; ++s1, ++s2)
		if (*s1 == 0)
			return 0;
	return *(unsigned char *)s1 < *(unsigned char *)s2 ? -1 : 1;
}

void pluginloader_init(struct pluginloader_t* loader)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);

	if (!loader)
		return;

	// Initialize all fields
	loader->pluginList = NULL;
	loader->pluginCount = 0;
	loader->pluginDirectory = NULL;

	// Get the currently configured framework plugin path
	char* frameworkPluginPath = gFramework->pluginsPath;
	if (!frameworkPluginPath)
	{
		WriteLog(LL_Error, "could not initialize pluginloader, plugin path not set");
		return;
	}

	// Calculate the path length
	// TODO: Unhack this
	size_t pluginPathLength = strlen(frameworkPluginPath);
	if (pluginPathLength == 0 || pluginPathLength > 260)
	{
		WriteLog(LL_Error, "path length is either zero, or > 260");
		return;
	}

	// Update the length
	loader->pluginDirectoryLength = pluginPathLength;

	// Allocate space for the plugin path
	char* pluginPath = (char*)kmalloc(pluginPathLength + 1);
	if (!pluginPath)
	{
		WriteLog(LL_Error, "could not allocate space for plugin path.");
		return;
	}
	memset(pluginPath, 0, pluginPathLength + 1);

	// Copy over our string
	memcpy(pluginPath, frameworkPluginPath, pluginPathLength);
	
	loader->pluginDirectory = pluginPath;

	// Assign our plugin loader params
	gLoaderPluginInit.framework = gFramework;
	gLoaderPluginInit.kernelBase = gKernelBase;
	gLoaderPluginInit.logger = gLogger;
}

uint8_t pluginloader_addPluginToList(struct pluginloader_t* loader, struct loaderplugin_t* pluginEntry)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);

	if (!loader || !pluginEntry)
		return false;

	// CHeck if we have a plugin list at all
	if (!loader->pluginList || !loader->pluginCount)
	{
		// Calculate the new size (sizeof(pointer) * however many in list)
		size_t newListSize = sizeof(pluginEntry) * 1;
		struct loaderplugin_t** newList = (struct loaderplugin_t**)kmalloc(newListSize);
		if (!newList)
		{
			WriteLog(LL_Error, "could not allocate new list");
			return false;
		}

		// Assign our new entry
		newList[0] = pluginEntry;

		// Assign our new plugin list
		loader->pluginList = newList;
		loader->pluginCount = 1;

		return true;
	}

	// Am I bat-shit insane for this?... probably
	// This screams, I need locks

	// If there exist a list already add a new entry
	struct loaderplugin_t** oldList = loader->pluginList;
	size_t oldPluginCount = loader->pluginCount;
	size_t oldListSize = sizeof(struct loaderplugin_t*) * oldPluginCount;

	size_t newPluginCount = oldPluginCount + 1;
	size_t newListSize = sizeof(struct loaderplugin_t*) * (newPluginCount);
	struct loaderplugin_t** newList = (struct loaderplugin_t**)kmalloc(newListSize);
	if (!newList)
	{
		WriteLog(LL_Error, "could not allocate new list");
		return false;
	}
	memset(newList, 0, newListSize);

	// Copy over the old list
	memcpy(newList, oldList, oldListSize);

	// Add our final entry
	newList[oldPluginCount] = pluginEntry;

	// Assign everything
	loader->pluginList = newList;
	loader->pluginCount = newPluginCount;

	return true;
}

uint32_t pluginloader_getAvailablePluginCount(struct pluginloader_t* loader)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	if (!loader)
		return 0;

	if (!loader->pluginDirectory || loader->pluginDirectoryLength == 0)
		return 0;

	int handle = kopen(loader->pluginDirectory, 0x0000 | 0x00020000, 0);
	if (handle < 0)
	{
		WriteLog(LL_Error, "could not open plugin directory %s", loader->pluginDirectory);
		return 0;
	}

	// Run through once to get the count
	uint64_t dentCount = 0;
	struct dirent* dent = 0;
	const uint32_t bufferSize = 0x8000;
	char* buffer = kmalloc(bufferSize);
	if (!buffer)
	{
		WriteLog(LL_Error, "could not allocate memory");
		kclose(handle);
		return 0;
	}

	// Zero out the buffer size
	memset(buffer, 0, bufferSize);

	// Get all of the directory entries the first time to get the count
	while (kgetdents(handle, buffer, bufferSize) > 0)
	{
		dent = (struct dirent*)buffer;

		while (dent->d_fileno)
		{
			if (dent->d_type == 0)
				break;

			// Create a new plugin entry

			dent = (struct dirent*)((uint8_t*)dent + dent->d_reclen);
		}
	}

	kclose(handle);

	return dentCount;
}

struct loaderplugin_t* pluginloader_loadPluginFromFile(struct pluginloader_t* loader, char* pluginPath)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);

	if (!loader || !pluginPath)
		return NULL;

	int fd = kopen(pluginPath, O_RDONLY, 0);
	if (fd < 0)
	{
		WriteLog(LL_Error, "could not load plugin (%s).", pluginPath);
		return NULL;
	}

	struct stat statbuf;
	memset(&statbuf, 0, sizeof(statbuf));

	// Get the file size
	int res = kfstat(fd, &statbuf);
	if (res < 0)
	{
		WriteLog(LL_Error, "could not get %d handle stat: %d", fd, res);
		kclose(fd);
		return NULL;
	}

	// Verify that our plugin is under the size limit
	size_t pluginSize = statbuf.st_size;
	if (pluginSize > PLUGIN_MAXSIZE)
	{
		WriteLog(LL_Warn, "plugin (%s) is too large, skipping.", pluginPath);
		kclose(fd);
		return NULL;
	}

	uint8_t* pluginData = (uint8_t*)kmalloc(pluginSize);
	if (!pluginData)
	{
		WriteLog(LL_Error, "could not allocate space for plugin (%s) (size: %lld)", pluginPath, pluginSize);
		kclose(fd);
		return NULL;
	}
	memset(pluginData, 0, pluginSize);

	// Read out our plugin data
	klseek(fd, 0, SEEK_SET);
	kread(fd, pluginData, pluginSize);
	kclose(fd);

	// Allocate a new entry for our list
	struct loaderplugin_t* entry = (struct loaderplugin_t*)kmalloc(sizeof(struct loaderplugin_t));
	if (!entry)
	{
		WriteLog(LL_Error, "could not allocate new entry");
		kfree(pluginData, pluginSize);
		return NULL;
	}

	entry->data = pluginData;
	entry->dataLength = pluginSize;
	entry->plugin = NULL; // do not create the plugin here
	
	size_t pathLength = strlen(pluginPath);
	if (pathLength > sizeof(entry->path))
		WriteLog(LL_Warn, "path is too long, not setting");
	else
		memcpy(entry->path, pluginPath, pathLength);

	return entry;
}

void pluginloader_loadPlugins(struct pluginloader_t* loader)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	// Verify that the loader is valid
	if (!loader)
		return;

	// Verify that the plugin directory is set
	char* pluginDirectory = loader->pluginDirectory;
	if (!pluginDirectory)
	{
		WriteLog(LL_Error, "Plugin directory not set.");
		return;
	}

	// Get the total available plugin count
	uint32_t pluginCount = pluginloader_getAvailablePluginCount(loader);
	if (pluginCount == 0)
	{
		WriteLog(LL_Info, "No plugins available to load from %s.", pluginDirectory);
		return;
	}

	int handle = kopen(pluginDirectory, 0x0000 | 0x00020000, 0);
	if (handle < 0)
	{
		WriteLog(LL_Error, "[-] could not open path %s", pluginDirectory);
		return;
	}

	struct dirent* dent = 0;
	const uint32_t bufferSize = 0x8000;
	char* buffer = kmalloc(bufferSize);
	if (!buffer)
	{
		kclose(handle);
		WriteLog(LL_Error, "could not allocate memory");
		return;
	}

	// Zero out the buffer size
	memset(buffer, 0, bufferSize);

	while (kgetdents(handle, buffer, bufferSize) > 0)
	{
		dent = (struct dirent*)buffer;

		while (dent->d_fileno)
		{
			if (dent->d_type == 0)
				break;

			// Skip over the . and .. entries
			if (!strcmp(".", dent->d_name) || !strcmp("..", dent->d_name))
			{
				dent = (struct dirent*)((uint8_t*)dent + dent->d_reclen);
				continue;
			}

			// Load the plugin from file
			struct loaderplugin_t* entry = pluginloader_loadPluginFromFile(loader, dent->d_name);
			if (!pluginloader_addPluginToList(loader, entry))
			{
				WriteLog(LL_Error, "could not add plugin to list.");
				kfree(entry, sizeof(*entry));
			}
				
			// Continue
			dent = (struct dirent*)((uint8_t*)dent + dent->d_reclen);
		}
	}

	WriteLog(LL_Debug, "closing handle");
	kclose(handle);

	pluginCount = pluginloader_getAvailablePluginCount(loader);
	for (uint32_t i = 0; i < pluginCount; ++i)
	{
		struct loaderplugin_t* availablePlugin = loader->pluginList[i];
		if (!availablePlugin)
			continue;

		struct pluginheader_t* header = (struct pluginheader_t*)availablePlugin->data;
		if (!header)
		{
			WriteLog(LL_Warn, "there is a loaderplugin_t with no data???");
			continue;
		}

		// TODO: Cast this properly
		void(* pluginInitialize)(void*, void*) = (void(*)(void*, void*))(availablePlugin->data + header->initializeOffset);
		uint64_t pluginSize = *(uint64_t*)(availablePlugin->data + header->pluginSizeOffset);

		WriteLog(LL_Info, "PluginSize: %lld, PluginInitialize: %p", pluginSize, pluginInitialize);

		if (pluginSize < sizeof(struct plugin_t))
			continue;

		// Allocate the new plugin structure
		struct plugin_t* newPlugin = (struct plugin_t*)kmalloc(pluginSize); // We allocate the plugin size which should be >= plugin_t
		if (!newPlugin)
		{
			WriteLog(LL_Error, "could not allocate new plugin space");
			continue;
		}
		memset(newPlugin, 0, pluginSize);

		// Initialize the plugin
		pluginInitialize(newPlugin, &gLoaderPluginInit);

		// This should work fine if everything went according to plan
		WriteLog(LL_Info, "Initialized (%s) plugin from file woot woot!", newPlugin->name);

		// Register the plugin with everything else
		pluginmanager_registerPlugin(gFramework->pluginManager, newPlugin);
	}
}