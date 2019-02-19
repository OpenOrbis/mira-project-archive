#include "fileexplorer_plugin.h"
#include <oni/framework.h>
#include <oni/utils/kdlsym.h>
#include <oni/messaging/messagemanager.h>
#include <oni/messaging/messagecontainer.h>

#include <oni/utils/sys_wrappers.h>
#include <sys/dirent.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <sys/fcntl.h>
#include <oni/utils/ref.h>
#include <oni/utils/logger.h>
#include <oni/utils/memory/allocator.h>
#include <oni/rpc/rpcserver.h>
#include <oni/utils/escape.h>

#include <mira/utils/flat_vector.h>
#include <mira/utils/vector.h>
#include <mira/miraframework.h>

#include <string.h>

#include "fileexplorer_messages.h"

#ifndef MIN
#define MIN ( x, y ) ( (x) < (y) ? : (x) : (y) )
#endif


void fileexplorer_open_callback(struct messagecontainer_t* reference);
void fileexplorer_close_callback(struct messagecontainer_t* reference);
void fileexplorer_read_callback(struct messagecontainer_t* reference);
void fileexplorer_write_callback(struct messagecontainer_t* reference);
void fileexplorer_getdents_callback(struct messagecontainer_t* reference);
void fileexplorer_stat_callback(struct messagecontainer_t* reference);
void fileexplorer_mkdir_callback(struct messagecontainer_t* reference);
void fileexplorer_rmdir_callback(struct messagecontainer_t* reference);
void fileexplorer_unlink_callback(struct messagecontainer_t* reference);

void fileexplorer_echo_callback(struct messagecontainer_t* container);

extern struct logger_t* gLogger;

void fileexplorer_plugin_init(struct fileexplorer_plugin_t* plugin)
{
	if (!plugin)
		return;

	plugin->plugin.name = "FileTransfer";
	plugin->plugin.description = "File transfer plugin using a custom standalone protocol";

	plugin->plugin.plugin_load = (uint8_t(*)(void*)) fileexplorer_load;
	plugin->plugin.plugin_unload = (uint8_t(*)(void*)) fileexplorer_unload;
}

uint8_t fileexplorer_load(struct fileexplorer_plugin_t* plugin)
{
	// Register all of the callbacks
	messagemanager_registerCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_Echo, fileexplorer_echo_callback);
	
	return true;
}

uint8_t fileexplorer_unload(struct fileexplorer_plugin_t* plugin)
{
	// Unregister all of the callbacks
	messagemanager_unregisterCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_Echo, fileexplorer_echo_callback);
	
	return true;
}

void fileexplorer_echo_callback(struct messagecontainer_t* container)
{
	if (container == NULL)
		return;

	WriteLog(LL_Debug, "container: %p", container);

	messagecontainer_acquire(container);
	
	if (container->size < sizeof(struct fileexplorer_echoRequest_t))
	{
		WriteLog(LL_Error, "malformed message");
		goto cleanup;
	}

	struct fileexplorer_echoRequest_t* request = (struct fileexplorer_echoRequest_t*)container->payload;
	if (request->length >= container->size)
	{
		WriteLog(LL_Error, "malformed length");
		goto cleanup;
	}

	WriteLog(LL_Info, "echo: (%d) %s", request->length, request->message);
	
cleanup:
	messagecontainer_release(container);
}

void fileexplorer_open_callback(struct messagecontainer_t* container)
{

}

void fileexplorer_close_callback(struct messagecontainer_t* container)
{ 

}

void fileexplorer_read_callback(struct messagecontainer_t* container)
{

}

void fileexplorer_write_callback(struct messagecontainer_t* container)
{

}


void fileexplorer_getdents_callback(struct messagecontainer_t* container)
{ 

}

void fileexplorer_stat_callback(struct messagecontainer_t* container)
{ 
}

void fileexplorer_mkdir_callback(struct messagecontainer_t* container)
{ 
}

void fileexplorer_rmdir_callback(struct messagecontainer_t* container)
{ 
}

void fileexplorer_unlink_callback(struct messagecontainer_t* container)
{
}