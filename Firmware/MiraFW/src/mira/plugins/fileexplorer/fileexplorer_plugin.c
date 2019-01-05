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

#define PB_DECODE(containerObject, capitalType, lowerType, variableName) \
	pbcontainer_acquire(containerObject); \
	capitalType* variableName = NULL; { \
	PbMessage* outerMessage = containerObject->message; \
	if (!outerMessage) { \
		WriteLog(LL_Error, "could not get the outer message"); \
		goto cleanup;	} \
	size_t innerDataSize = outerMessage->payload.len; \
	uint8_t* innerData = outerMessage->payload.data; \
	if (!innerData || innerDataSize == 0) { \
		WriteLog(LL_Error, "could not get the inner data"); \
		goto cleanup; } variableName = lowerType ## __unpack(NULL, innerDataSize, innerData); }

#define PB_ENCODE(fromContainer, message, messageCategory, messageCommand, lowerType, outputContainerName) \
	PbContainer* outputContainerName = NULL; \
	{ size_t packedSize = lowerType ## __get_packed_size(&message); \
		uint8_t* responseData = k_malloc(packedSize); \
		if (responseData == NULL) \
		{ WriteLog(LL_Error, "could not allocate response data"); \
			goto cleanup; \
		} (void) lowerType ## __pack(&message, responseData); \
		PbMessage* responseMessage = k_malloc(sizeof(PbMessage)); \
		if (responseMessage == NULL) \
		{ WriteLog(LL_Error, "could not allocate response message"); \
			goto cleanup; \
		} static PbMessage pb_message_init = PB_MESSAGE__INIT; \
		*responseMessage = pb_message_init; \
		responseMessage->category = messageCategory; \
		responseMessage->type = messageCommand; \
		responseMessage->payload.data = responseData; \
		responseMessage->payload.len = packedSize; \
		outputContainerName = pbcontainer_create(responseMessage, true); \
		if (outputContainerName == NULL) \
		{	k_free(responseData); \
			responseData = NULL; \
			k_free(responseMessage); \
			responseMessage = NULL; \
			WriteLog(LL_Error, "could not allocate response container"); \
			goto cleanup; \
		}	}

#define PB_RELEASE(containerObject) \
	cleanup: \
		pbcontainer_release(containerObject);

void fileexplorer_echo_callback(PbContainer* container)
{
	if (container == NULL || container->message == NULL)
		return;

	PB_DECODE(container, EchoRequest, echo_request, request);
	if (!request)
	{
		WriteLog(LL_Error, "could not decode incoming request.");
		goto cleanup;
	}

	// Start our custom code
	WriteLog(LL_Warn, "echo: %s", request->message);
	// End our custom code

	// Initialize a response
	EchoResponse response = ECHO_RESPONSE__INIT;
	response.error = ERRORS__EOK;

	PB_ENCODE(container, response, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Echo, echo_response, responseContainer);

	// Send the response back and release this pbcontainer which should free
	messagemanager_sendResponse(responseContainer);
	pbcontainer_release(responseContainer);

	// release the container reference
	PB_RELEASE(container);
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
	size_t innerDataSize = outerMessage->payload.len;
	uint8_t* innerData = outerMessage->payload.data;
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
	PbContainer* responseContainer = pbcontainer_createNew(outerMessage->category, outerMessage->type, responseData, responseSize);
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