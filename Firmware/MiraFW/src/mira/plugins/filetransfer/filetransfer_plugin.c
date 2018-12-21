#include "filetransfer_plugin.h"
#include <oni/framework.h>
#include <oni/utils/kdlsym.h>
#include <oni/messaging/messagemanager.h>
#include <oni/messaging/pbcontainer.h>
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

#include <string.h>

#include <protobuf-c/mirabuiltin.pb-c.h>

#include "fileexplorer.pb-c.h"

#ifndef MIN
#define MIN ( x, y ) ( (x) < (y) ? : (x) : (y) )
#endif

void filetransfer_open_callback(PbContainer* reference);
void filetransfer_close_callback(PbContainer* reference);
void filetransfer_read_callback(PbContainer* reference);
void filetransfer_readfile_callback(PbContainer* reference);
void filetransfer_write_callback(PbContainer* reference);
void filetransfer_writefile_callback(PbContainer* reference);
void filetransfer_getdents_callback(PbContainer* reference);
void filetransfer_delete_callback(PbContainer* reference);
void filetransfer_stat_callback(PbContainer* reference);
void filetransfer_mkdir_callback(PbContainer* reference);
void filetransfer_rmdir_callback(PbContainer* reference);
void filetransfer_unlink_callback(PbContainer* reference);

void filetransfer_echo_callback(PbContainer* container);

extern struct logger_t* gLogger;

void filetransfer_plugin_init(struct filetransfer_plugin_t* plugin)
{
	if (!plugin)
		return;

	plugin->plugin.name = "FileTransfer";
	plugin->plugin.description = "File transfer plugin using a custom standalone protocol";

	plugin->plugin.plugin_load = (uint8_t(*)(void*)) filetransfer_load;
	plugin->plugin.plugin_unload = (uint8_t(*)(void*)) filetransfer_unload;
}

uint8_t filetransfer_load(struct filetransfer_plugin_t* plugin)
{
	// Register all of the callbacks
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Open, filetransfer_open_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Close, filetransfer_close_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__GetDents, filetransfer_read_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Read, filetransfer_readfile_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Write, filetransfer_write_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Unlink, filetransfer_writefile_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Move, filetransfer_getdents_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Stat, filetransfer_delete_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__MkDir, filetransfer_stat_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__RmDir, filetransfer_mkdir_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Echo, filetransfer_echo_callback);
	
	return true;
}

uint8_t filetransfer_unload(struct filetransfer_plugin_t* plugin)
{
	// Unregister all of the callbacks
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Open, filetransfer_open_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Close, filetransfer_close_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__GetDents, filetransfer_read_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Read, filetransfer_readfile_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Write, filetransfer_write_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Unlink, filetransfer_writefile_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Move, filetransfer_getdents_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Stat, filetransfer_delete_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__MkDir, filetransfer_stat_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__RmDir, filetransfer_mkdir_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Echo, filetransfer_echo_callback);

	return true;
}

void filetransfer_echo_callback(PbContainer* container)
{
	if (!container || !container->message)
		return;

	// Hold the incoming request
	EchoRequest* request = NULL;

	// Acquire a reference before we touch anything in this structure
	pbcontainer_acquire(container);

	// Get the outer message that is already parsed
	PbMessage* outerMessage = container->message;
	if (!outerMessage)
		goto cleanup;

	// Get the inner (OpenRequest) message data and size
	size_t innerDataSize = outerMessage->data.len;
	uint8_t* innerData = outerMessage->data.data;
	if (!innerData || innerDataSize == 0)
		goto cleanup;

	// Unpack the inner message
	request = echo_request__unpack(NULL, innerDataSize, innerData);
	if (!request)
		goto cleanup;

	WriteLog(LL_Warn, "echo: %s", request->message);

	EchoResponse response = ECHO_RESPONSE__INIT;
	response.error = 0;

	// Calculate the packed size
	size_t responseSize = echo_response__get_packed_size(&response);

	// Allocate the response data
	uint8_t* responseData = k_malloc(responseSize);
	if (!responseData)
	{
		WriteLog(LL_Error, "could not allocate response data");
		goto cleanup;
	}
	(void)memset(responseData, 0, responseSize);

	// Pack the data into the newly allocated buffer
	size_t responsePackedSize = echo_response__pack(&response, responseData);
	if (responseSize != responsePackedSize)
	{
		WriteLog(LL_Error, "responseSize (%llx) != responsePackedSize (%llx)", responseSize, responsePackedSize);
		k_free(responseData);
		goto cleanup;
	}

	// Create a pbcontainer for ref counting
	PbContainer* responseContainer = pbcontainer_create2(outerMessage->category, outerMessage->type, responseData, responseSize);
	if (!responseContainer)
	{
		WriteLog(LL_Error, "could not allocate response container");
		k_free(responseData);
		goto cleanup;
	}

	// Send the response back to PC
	messagemanager_sendResponse(responseContainer);

	// Release (which should destroy the response container)
	pbcontainer_release(responseContainer);
cleanup:
	if (request)
		echo_request__free_unpacked(request, NULL);

	pbcontainer_release(container);
}

void filetransfer_open_callback(PbContainer* reference)
{
	if (!reference || !reference->message)
		return;

	// Hold the incoming request
	OpenRequest* request = NULL;

	// Acquire a reference before we touch anything in this structure
	pbcontainer_acquire(reference);

	// Get the outer message that is already parsed
	PbMessage* outerMessage = reference->message;
	if (!outerMessage)
		goto cleanup;

	// Get the inner (OpenRequest) message data and size
	size_t innerDataSize = outerMessage->data.len;
	uint8_t* innerData = outerMessage->data.data;
	if (!innerData || innerDataSize == 0)
		goto cleanup;

	// Unpack the inner message
	request = open_request__unpack(NULL, innerDataSize, innerData);
	if (!request)
		goto cleanup;

	// Do the needful
	struct thread_info_t threadInfo;
	oni_threadEscape(curthread, &threadInfo);
	int32_t result = kopen(request->path, request->flags, request->mode);
	oni_threadRestore(curthread, &threadInfo);

	// Create the response
	OpenResponse response = OPEN_RESPONSE__INIT;
	if (result < 0)
	{
		// Handle error case
		response.error = result;
	}
	else
	{
		// No error, return handle
		response.handle = result;
		response.error = 0;
	}

	// Calculate the packed size
	size_t responseSize = open_response__get_packed_size(&response);

	// Allocate the response data
	uint8_t* responseData = k_malloc(responseSize);
	if (!responseData)
	{
		WriteLog(LL_Error, "could not allocate response data");
		goto cleanup;
	}
	(void)memset(responseData, 0, responseSize);

	// Pack the data into the newly allocated buffer
	size_t responsePackedSize = open_response__pack(&response, responseData);
	if (responseSize != responsePackedSize)
	{
		WriteLog(LL_Error, "responseSize (%llx) != responsePackedSize (%llx)", responseSize, responsePackedSize);
		k_free(responseData);
		goto cleanup;
	}

	// Create a pbcontainer for ref counting
	PbContainer* responseContainer = pbcontainer_create2(outerMessage->category, outerMessage->type, responseData, responseSize);
	if (!responseContainer)
	{
		WriteLog(LL_Error, "could not allocate response container");
		k_free(responseData);
		goto cleanup;
	}

	// Send the response back to PC
	messagemanager_sendResponse(responseContainer);

	// Release (which should destroy the response container)
	pbcontainer_release(responseContainer);

cleanup:
	if (request)
		open_request__free_unpacked(request, NULL);

	pbcontainer_release(reference);
}

void filetransfer_close_callback(PbContainer* reference){ }

void filetransfer_read_callback(PbContainer* reference){ }

void filetransfer_readfile_callback(PbContainer* reference){ }

void filetransfer_write_callback(PbContainer* reference){ }

void filetransfer_writefile_callback(PbContainer* reference){ }

void filetransfer_getdents_callback(PbContainer* reference){ }

void filetransfer_delete_callback(PbContainer* reference){ }

void filetransfer_stat_callback(PbContainer* reference){ }

void filetransfer_mkdir_callback(PbContainer* reference){ }

void filetransfer_rmdir_callback(PbContainer* reference){ }

void filetransfer_unlink_callback(PbContainer* reference){ }