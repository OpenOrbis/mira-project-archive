#include "fileexplorer_plugin.h"
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

void fileexplorer_open_callback(PbContainer* reference);
void fileexplorer_close_callback(PbContainer* reference);
void fileexplorer_read_callback(PbContainer* reference);
void fileexplorer_readfile_callback(PbContainer* reference);
void fileexplorer_write_callback(PbContainer* reference);
void fileexplorer_writefile_callback(PbContainer* reference);
void fileexplorer_getdents_callback(PbContainer* reference);
void fileexplorer_delete_callback(PbContainer* reference);
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
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Open, fileexplorer_open_callback);
	//messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Close, fileexplorer_close_callback);
	//messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__GetDents, fileexplorer_read_callback);
	//messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Read, fileexplorer_readfile_callback);
	//messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Write, fileexplorer_write_callback);
	//messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Unlink, fileexplorer_writefile_callback);
	//messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Move, fileexplorer_getdents_callback);
	//messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Stat, fileexplorer_delete_callback);
	//messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__MkDir, fileexplorer_stat_callback);
	//messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__RmDir, fileexplorer_mkdir_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Echo, fileexplorer_echo_callback);
	
	return true;
}

uint8_t fileexplorer_unload(struct fileexplorer_plugin_t* plugin)
{
	// Unregister all of the callbacks
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Open, fileexplorer_open_callback);
	//messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Close, fileexplorer_close_callback);
	//messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__GetDents, fileexplorer_read_callback);
	//messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Read, fileexplorer_readfile_callback);
	//messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Write, fileexplorer_write_callback);
	//messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Unlink, fileexplorer_writefile_callback);
	//messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Move, fileexplorer_getdents_callback);
	//messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Stat, fileexplorer_delete_callback);
	//messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__MkDir, fileexplorer_stat_callback);
	//messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__RmDir, fileexplorer_mkdir_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Echo, fileexplorer_echo_callback);

	return true;
}

#define UNPACK_REQUEST(requestType, lowerRequestType, name, container) \
	requestType* name = NULL; \
	pbcontainer_acquire(container); \
	{ \
		PbMessage* outerMessage = container->message;	\
		if (!outerMessage)	\
		{WriteLog(LL_Error, "could not get outer message");	\
			goto cleanup; }	size_t innerDataSize = outerMessage->data.len; \
		uint8_t* innerData = outerMessage->data.data; \
		if (!innerData || innerDataSize == 0) \
		{ WriteLog(LL_Error, "innerData (%p) or innerDataSize (%llx) is invalid", innerData, innerDataSize); \
			goto cleanup; \
		} name = lowerRequestType##__unpack(NULL, innerDataSize, innerData); \
	}

#define CLEANUP_REQUEST(lowerRequestType, name, container) \
	cleanup: \
		if (name) \
			lowerRequestType##__free_unpacked(name, NULL); \
		pbcontainer_release(container);

#define PACK_RESPONSE(response, lowerResponseType, requestContainer, name) \
	PbContainer* name = NULL; \
	{	\
		size_t responseSize = lowerResponseType##__get_packed_size(response); \
		if (responseSize == 0) \
		{ WriteLog(LL_Error, "responseSize == 0"); \
			goto cleanup; \
		} uint8_t* responseData = k_malloc(responseSize); \
		if (!responseData) \
		{ WriteLog(LL_Error, "could not allocate response data"); \
			goto cleanup; \
		} (void)memset(responseData, 0, responseSize); \
		size_t responsePackedSize = lowerResponseType##__pack(response, responseData); \
		if (responseSize != responsePackedSize) \
		{ WriteLog(LL_Error, "responseSize (%llx) != responsePackedSize (%llx)", responseSize, responsePackedSize); \
			k_free(responseData); \
			goto cleanup; \
		} name = pbcontainer_create2(container->message->category, container->message->type, responseData, responseSize); \
		if (!name) \
		{ WriteLog(LL_Error, "could not allocate response container"); \
			k_free(responseData); \
			goto cleanup; } \
	} 


void fileexplorer_echo_callback(PbContainer* container)
{
	if (!container || !container->message)
		return;

	UNPACK_REQUEST(EchoRequest, echo_request, request, container);

	WriteLog(LL_Warn, "echo: %s", request->message);

	// Update our error code
	IntValue* error = k_malloc(sizeof(*error));
	if (!error)
	{
		WriteLog(LL_Error, "could not allocate error code.");
		goto cleanup;
	}
	(void)memset(error, 0, sizeof(*error));
	error->value = ERRORS__EOK;

	EchoResponse* response = k_malloc(sizeof(EchoResponse));
	if (!response)
	{
		WriteLog(LL_Error, "could not allocate response");
		goto cleanup;
	}
	(void)memset(response, 0, sizeof(*response));

	response->error = error;

	
	PACK_RESPONSE(response, echo_response, container, responseContainer);

	// Send the response back to PC
	messagemanager_sendResponse(responseContainer);

	// Release (which should destroy the response container)
	pbcontainer_release(responseContainer);

	CLEANUP_REQUEST(echo_request, request, container);
}

void fileexplorer_open_callback(PbContainer* reference)
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

void fileexplorer_close_callback(PbContainer* reference){ }

void fileexplorer_read_callback(PbContainer* reference){ }

void fileexplorer_readfile_callback(PbContainer* reference){ }

void fileexplorer_write_callback(PbContainer* reference){ }

void fileexplorer_writefile_callback(PbContainer* reference){ }

void fileexplorer_getdents_callback(PbContainer* reference){ }

void fileexplorer_delete_callback(PbContainer* reference){ }

void fileexplorer_stat_callback(PbContainer* reference){ }

void fileexplorer_mkdir_callback(PbContainer* reference){ }

void fileexplorer_rmdir_callback(PbContainer* reference){ }

void fileexplorer_unlink_callback(PbContainer* reference){ }