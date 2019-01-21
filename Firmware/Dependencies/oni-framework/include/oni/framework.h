#pragma once
#include <oni/utils/types.h>

struct logger_t;
struct messagemanager_t;
struct pluginmanager_t;
struct pbserver_t;

struct framework_t
{
	char* homePath; // /user/framework
	char* configPath; // /user/framework/config.json
	char* pluginsPath; // /user/framework/plugins
	char* downloadPath; // /user/framework/download


	struct messagemanager_t* messageManager;
	struct pbserver_t* rpcServer;
	struct pluginmanager_t* pluginManager;
};

// Framework platform
extern struct framework_t* gFramework;

// Initialization parameters
extern struct initparams_t* gInitParams;

// Global logger
extern struct logger_t* gLogger;

// Base address of the kernel
extern uint8_t* gKernelBase;

// Default userland entry point
extern int oni_initializeFramework();

// Default kernelland entry point
extern void oni_kernelInitialization(void* loaderInitParams);

extern const char* gNull;