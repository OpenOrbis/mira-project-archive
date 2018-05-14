#include "example_plugin.h"
#include <oni/framework.h>
#include <oni/utils/log/logger.h>

uint64_t plugin_size = sizeof(struct example_plugin_t);
struct framework_t* gFramework = NULL;
struct logger_t* gLogger = NULL;
uint8_t* gKernelBase = NULL;

void plugin_initialize(void* plugin, struct plugininit_t* arg)
{
	if (!plugin || !arg)
		return;

	struct example_plugin_t* examplePlugin = (struct example_plugin_t*)plugin;
	examplePlugin->plugin.name = "ExamplePlugin";
	examplePlugin->plugin.description = "This is the example plugin description";
	examplePlugin->plugin.plugin_load = plugin_load;
	examplePlugin->plugin.plugin_unload = plugin_unload;

	gKernelBase = arg->kernelBase;
	gFramework = arg->framework;
	gLogger = arg->logger;
}

uint8_t plugin_load(void* plugin)
{
	if (!plugin)
		return false;
	
	WriteLog(LL_Info, "Hello World from dynamically loaded plugin");

	return true;
}

uint8_t plugin_unload(void* plugin)
{
	if (!plugin)
		return false;

	return true;
}
