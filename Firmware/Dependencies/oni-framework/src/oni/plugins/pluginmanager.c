#include <oni/plugins/pluginmanager.h>
#include <oni/plugins/plugin.h>

#include <sys/param.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#include <oni/utils/logger.h>
#include <oni/utils/kdlsym.h>

void pluginmanager_init(struct pluginmanager_t* manager)
{
	if (!manager)
		return;

	// Zero out our plugin table
	for (uint32_t i = 0; i < ARRAYSIZE(manager->plugins); ++i)
		manager->plugins[i] = NULL;

	void(*mtx_init)(struct mtx *m, const char *name, const char *type, int opts) = kdlsym(mtx_init);
	mtx_init(&manager->lock, "pmmtx", NULL, 0);
}

int32_t pluginmanager_findFreePluginIndex(struct pluginmanager_t* manager)
{
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	if (!manager)
		return -1;

	int32_t pluginIndex = -1;

	_mtx_lock_flags(&manager->lock, 0, __FILE__, __LINE__);
	for (uint32_t i = 0; i < ARRAYSIZE(manager->plugins); ++i)
	{
		if (!manager->plugins[i])
		{
			pluginIndex = i;
			break;
		}
	}
	_mtx_unlock_flags(&manager->lock, 0, __FILE__, __LINE__);

	return pluginIndex;
}

uint32_t pluginmanager_pluginCount(struct pluginmanager_t* manager)
{
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	if (!manager)
		return 0;

	uint32_t count = 0;

	_mtx_lock_flags(&manager->lock, 0, __FILE__, __LINE__);
	for (uint32_t i = 0; i < ARRAYSIZE(manager->plugins); ++i)
	{
		if (manager->plugins[i])
			count++;
	}
	_mtx_unlock_flags(&manager->lock, 0, __FILE__, __LINE__);

	return count;
}

int32_t pluginmanager_registerPlugin(struct pluginmanager_t* manager, struct plugin_t* plugin)
{
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	WriteLog(LL_Info, "registering '%s' plugin", plugin->name);

	if (!manager)
	{
		WriteLog(LL_Error, "no manager");
		return false;
	}


	int32_t pluginIndex = pluginmanager_findFreePluginIndex(manager);
	if (pluginIndex == -1)
	{
		WriteLog(LL_Error, "no free indices");
		return false;
	}


	int32_t pluginExists = 0;

	// Verify that this plugin isn't already loaded
	_mtx_lock_flags(&manager->lock, 0, __FILE__, __LINE__);
	for (uint32_t i = 0; i < ARRAYSIZE(manager->plugins); ++i)
	{
		if (manager->plugins[i] == plugin)
		{
			pluginExists = 1;
			break;
		}
	}
	_mtx_unlock_flags(&manager->lock, 0, __FILE__, __LINE__);

	if (pluginExists)
	{
		WriteLog(LL_Error, "plugin exists");
		return false;
	}

	// Assign our plugin
	manager->plugins[pluginIndex] = plugin;

	// TODO: Make auto-loading configurable
	WriteLog(LL_Debug, "loading plugin: %s", plugin->name);

	// Bugcheck?
	uint8_t pluginLoadResult = plugin->plugin_load ? plugin->plugin_load(plugin) : false;
	if (!pluginLoadResult)
		return false;

	return true;
}

int32_t pluginmanager_unregisterPlugin(struct pluginmanager_t* manager, struct plugin_t* plugin)
{
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	_mtx_lock_flags(&manager->lock, 0, __FILE__, __LINE__);
	uint8_t value = false;
	for (uint64_t i = 0; i < ARRAYSIZE(manager->plugins); ++i)
	{
		if (manager->plugins[i] == plugin)
		{
			// Unload the plugin
			if (plugin->plugin_unload) // Bugcheck?
				plugin->plugin_unload(plugin);
			manager->plugins[i] = NULL;
			value = true;
			break;
		}
	}

	_mtx_unlock_flags(&manager->lock, 0, __FILE__, __LINE__);
	return value;
}

void pluginmanager_shutdown(struct pluginmanager_t* manager)
{
	for (uint64_t i = 0; i < ARRAYSIZE(manager->plugins); ++i)
	{
		if (manager->plugins[i] == NULL)
			continue;

		pluginmanager_unregisterPlugin(manager, manager->plugins[i]);
	}
}