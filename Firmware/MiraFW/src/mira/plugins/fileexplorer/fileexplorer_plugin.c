#include "fileexplorer_plugin.h"
#include <oni/framework.h>
#include <oni/utils/kdlsym.h>
#include <oni/messaging/messagemanager.h>
#include <oni/utils/sys_wrappers.h>
#include <sys/dirent.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <sys/fcntl.h>
#include <oni/utils/ref.h>
#include <oni/utils/logger.h>
#include <oni/utils/memory/allocator.h>
#include <oni/rpc/pbserver.h>
#include <oni/utils/escape.h>

#include <mira/utils/flat_vector.h>
#include <mira/utils/vector.h>

#include <string.h>

#ifndef MIN
#define MIN ( x, y ) ( (x) < (y) ? : (x) : (y) )
#endif

void fileexplorer_open_callback(PbContainer* reference);
void fileexplorer_close_callback(PbContainer* reference);
void fileexplorer_read_callback(PbContainer* reference);
void fileexplorer_write_callback(PbContainer* reference);
void fileexplorer_getdents_callback(PbContainer* reference);
void fileexplorer_stat_callback(PbContainer* reference);
void fileexplorer_mkdir_callback(PbContainer* reference);
void fileexplorer_rmdir_callback(PbContainer* reference);
void fileexplorer_unlink_callback(PbContainer* reference);

void fileexplorer_echo_callback(PbContainer* container);

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
	
	return true;
}

uint8_t fileexplorer_unload(struct fileexplorer_plugin_t* plugin)
{
	// Unregister all of the callbacks

	return true;
}

void fileexplorer_echo_callback(PbContainer* container)
{

}

void fileexplorer_open_callback(PbContainer* container)
{

}

void fileexplorer_close_callback(PbContainer* container)
{ 

}

void fileexplorer_read_callback(PbContainer* container)
{

}

void fileexplorer_write_callback(PbContainer* container)
{

}


void fileexplorer_getdents_callback(PbContainer* container)
{ 

}

void fileexplorer_stat_callback(PbContainer* container)
{ 
}

void fileexplorer_mkdir_callback(PbContainer* container)
{ 
}

void fileexplorer_rmdir_callback(PbContainer* container)
{ 
}

void fileexplorer_unlink_callback(PbContainer* container)
{
}