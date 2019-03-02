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
	messagemanager_registerCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_Open, fileexplorer_open_callback);
	messagemanager_registerCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_Close, fileexplorer_close_callback);
	messagemanager_registerCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_Read, fileexplorer_read_callback);
	messagemanager_registerCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_Write, fileexplorer_write_callback);
	messagemanager_registerCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_GetDents, fileexplorer_getdents_callback);
	messagemanager_registerCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_Stat, fileexplorer_stat_callback);
	messagemanager_registerCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_MkDir, fileexplorer_mkdir_callback);
	messagemanager_registerCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_RmDir, fileexplorer_rmdir_callback);
	messagemanager_registerCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_Unlink, fileexplorer_unlink_callback);

	return true;
}

uint8_t fileexplorer_unload(struct fileexplorer_plugin_t* plugin)
{
	// Unregister all of the callbacks
	messagemanager_unregisterCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_Echo, fileexplorer_echo_callback);
	messagemanager_unregisterCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_Open, fileexplorer_open_callback);
	messagemanager_unregisterCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_Close, fileexplorer_close_callback);
	messagemanager_unregisterCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_Read, fileexplorer_read_callback);
	messagemanager_unregisterCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_Write, fileexplorer_write_callback);
	messagemanager_unregisterCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_GetDents, fileexplorer_getdents_callback);
	messagemanager_unregisterCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_Stat, fileexplorer_stat_callback);
	messagemanager_unregisterCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_MkDir, fileexplorer_mkdir_callback);
	messagemanager_unregisterCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_RmDir, fileexplorer_rmdir_callback);
	messagemanager_unregisterCallback(mira_getFramework()->framework.messageManager, MessageCategory_File, FileExplorer_Unlink, fileexplorer_unlink_callback);

	return true;
}

void fileexplorer_echo_callback(struct messagecontainer_t* container)
{
	if (container == NULL)
		return;

	messagecontainer_acquire(container);
	
	if (container->size < sizeof(struct fileexplorer_echoRequest_t))
	{
		WriteLog(LL_Error, "malformed message");
		messagemanager_sendErrorResponse(MessageCategory_File, -ENOBUFS);
		goto cleanup;
	}

	struct fileexplorer_echoRequest_t* request = (struct fileexplorer_echoRequest_t*)container->payload;
	if (request->length >= container->size)
	{
		WriteLog(LL_Error, "malformed length");
		messagemanager_sendErrorResponse(MessageCategory_File, -ENOBUFS);
		goto cleanup;
	}

	WriteLog(LL_Info, "echo: (%d) %s", request->length, request->message);
	
	// Send successful result back
	messagemanager_sendErrorResponse(MessageCategory_File, 0);

cleanup:
	messagecontainer_release(container);
}

void fileexplorer_open_callback(struct messagecontainer_t* container)
{
	if (container == NULL)
		return;

	messagecontainer_acquire(container);

	uint16_t payloadLength = container->header.payloadLength;

	struct fileexplorer_openRequest_t* request = (struct fileexplorer_openRequest_t*)container->payload;
	if (request->pathLength > payloadLength || request->pathLength == 0)
	{
		WriteLog(LL_Error, "path is out of bounds (%d) > (%d)", request->pathLength, container->size);
		messagemanager_sendErrorResponse(MessageCategory_File, -ENAMETOOLONG);
		goto cleanup;
	}

	WriteLog(LL_Debug, "open: (%s) (%d) (%d)", request->path, request->flags, request->mode);

	struct thread_info_t threadInfo;
	oni_threadEscape(curthread, &threadInfo);
	int32_t result = kopen(request->path, request->flags, request->mode);
	oni_threadRestore(curthread, &threadInfo);

	struct messagecontainer_t* responseContainer = messagecontainer_createMessage(MessageCategory_File, (uint32_t)result, false, NULL, 0);
	if (responseContainer == NULL)
	{
		WriteLog(LL_Error, "could not allocate response");
		messagemanager_sendErrorResponse(MessageCategory_File, -ENOMEM);
		goto cleanup;
	}

	messagemanager_sendResponse(responseContainer);
	messagecontainer_release(responseContainer);

cleanup:
	messagecontainer_release(container);
}


void fileexplorer_close_callback(struct messagecontainer_t* container)
{ 
	if (container == NULL)
		return;

	messagecontainer_acquire(container);

	uint16_t payloadLength = container->header.payloadLength;
	if (payloadLength < sizeof(struct fileexplorer_closeRequest_t))
	{
		WriteLog(LL_Error, "not enough payload (%d) wanted (%d).", payloadLength, sizeof(struct fileexplorer_closeRequest_t));
		goto cleanup;
	}

	struct fileexplorer_closeRequest_t* request = (struct fileexplorer_closeRequest_t*)container->payload;

	struct thread_info_t threadInfo;
	oni_threadEscape(curthread, &threadInfo);
	kclose(request->handle);
	oni_threadRestore(curthread, &threadInfo);

	// Send a success response back
	messagemanager_sendErrorResponse(MessageCategory_File, 0);

cleanup:
	messagecontainer_release(container);
}

void fileexplorer_read_callback(struct messagecontainer_t* container)
{
	if (container == NULL)
		return;

	messagecontainer_acquire(container);

	uint16_t payloadLength = container->header.payloadLength;
	uint32_t expectedRequestSize = sizeof(struct fileexplorer_readRequest_t);
	if (payloadLength < expectedRequestSize)
	{
		WriteLog(LL_Error, "not enough payload (%d) wanted (%d).", payloadLength, expectedRequestSize);
		messagemanager_sendErrorResponse(MessageCategory_File, -ENOBUFS);
		goto cleanup;
	}

	struct fileexplorer_readRequest_t* request = (struct fileexplorer_readRequest_t*)container->payload;
	if (request->handle < 0)
	{
		WriteLog(LL_Error, "invalid handle (%d).", request->handle);
		messagemanager_sendErrorResponse(MessageCategory_File, request->handle);
		goto cleanup;
	}

	uint32_t maxRequestSize = (MESSAGECONTAINER_MAXBUFSZ - sizeof(struct fileexplorer_readResponse_t));
	if (request->count > maxRequestSize)
	{
		WriteLog(LL_Error, "request count was too large (%d) max (%d).", request->count, maxRequestSize);
		messagemanager_sendErrorResponse(MessageCategory_File, -EFBIG);
		goto cleanup;
	}

	// Sometimes PS4 doesn't like large buffers, so we allocate this one
	uint8_t* readBuffer = (uint8_t*)k_malloc(request->count);
	if (readBuffer == NULL)
	{
		WriteLog(LL_Error, "could not allocate buffer");
		messagemanager_sendErrorResponse(MessageCategory_File, -ENOMEM);
		goto cleanup;
	}
	(void)memset(readBuffer, 0, request->count);

	// Read out the data into our buffer
	int32_t result = kread(request->handle, readBuffer, request->count);
	if (result < 0)
	{
		WriteLog(LL_Error, "could not read into the buffer (%d).", result);

		// Free the buffer to prevent leaks
		if (readBuffer)
			k_free(readBuffer);
		readBuffer = NULL;

		messagemanager_sendErrorResponse(MessageCategory_File, result);
		goto cleanup;
	}

	// Send the response back, this creates a copy of the data
	struct messagecontainer_t* responseContainer = messagecontainer_createMessage(MessageCategory_File, (uint32_t)result, false, readBuffer, request->count);
	if (responseContainer == NULL)
	{
		WriteLog(LL_Error, "could not allocate response");

		// Free the buffer to prevent leaks
		if (readBuffer)
			k_free(readBuffer);
		readBuffer = NULL;

		messagemanager_sendErrorResponse(MessageCategory_File, -ENOMEM);
		goto cleanup;
	}

	messagemanager_sendResponse(responseContainer);
	messagecontainer_release(responseContainer);

	// Free the buffer to prevent leaks
	if (readBuffer)
		k_free(readBuffer);
	readBuffer = NULL;

cleanup:
	messagecontainer_release(container);
}

void fileexplorer_write_callback(struct messagecontainer_t* container)
{
	if (container == NULL)
		return;

	messagecontainer_acquire(container);

	uint16_t payloadLength = container->header.payloadLength;
	uint32_t expectedRequestSize = sizeof(struct fileexplorer_writeRequest_t);
	if (payloadLength < expectedRequestSize)
	{
		WriteLog(LL_Error, "not enough payload (%d) wanted (%d).", payloadLength, expectedRequestSize);
		messagemanager_sendErrorResponse(MessageCategory_File, -ENOBUFS);
		goto cleanup;
	}

	struct fileexplorer_writeRequest_t* request = (struct fileexplorer_writeRequest_t*)container->payload;
	if (request->handle < 0)
	{
		WriteLog(LL_Error, "invalid handle (%d).", request->handle);
		messagemanager_sendErrorResponse(MessageCategory_File, request->handle);
		goto cleanup;
	}

	if (request->count > MESSAGECONTAINER_MAXBUFSZ)
	{
		WriteLog(LL_Error, "request too large (%d) max (%d).", request->count, MESSAGECONTAINER_MAXBUFSZ);
		messagemanager_sendErrorResponse(MessageCategory_File, -EFBIG);
		goto cleanup;
	}

	// Read out the data into our buffer
	int32_t result = kwrite(request->handle, request->data, request->count);
	if (result < 0)
	{
		WriteLog(LL_Error, "could not read into the buffer (%d).", result);
		messagemanager_sendErrorResponse(MessageCategory_File, result);
		goto cleanup;
	}

	// Send the response back, this creates a copy of the data
	messagemanager_sendErrorResponse(MessageCategory_File, result);

cleanup:
	messagecontainer_release(container);
}


void fileexplorer_getdents_callback(struct messagecontainer_t* container)
{ 
	if (container == NULL)
		return;

	messagecontainer_acquire(container);

	uint16_t payloadLength = container->header.payloadLength;
	uint32_t expectedRequestSize = sizeof(uint16_t); // for whatever empty array's takes memory rounded up, so it can be larger than whats on wire
	if (payloadLength < expectedRequestSize)
	{
		WriteLog(LL_Error, "not enough payload (%d) wanted (%d).", payloadLength, expectedRequestSize);
		messagemanager_sendErrorResponse(MessageCategory_File, -ENOBUFS);
		goto cleanup;
	}

	struct fileexplorer_getdentsRequest_t* request = (struct fileexplorer_getdentsRequest_t*)container->payload;
	if (request->length == 0 || request->length > container->size)
	{
		WriteLog(LL_Error, "invalid path length (%d) (%d).", request->length, container->size);
		messagemanager_sendErrorResponse(MessageCategory_File, -ENOENT);
		goto cleanup;
	}

	struct thread_info_t threadInfo;
	oni_threadEscape(curthread, &threadInfo);
	int32_t directoryHandle = kopen(request->path, 0x0000 | 0x00020000, 0777);
	oni_threadRestore(curthread, &threadInfo);

	// Check if we opened the directory fine
	if (directoryHandle < 0)
	{
		WriteLog(LL_Error, "could not open directory (%s) (%d).", request->path, directoryHandle);
		messagemanager_sendErrorResponse(MessageCategory_File, directoryHandle);
		goto cleanup;
	}

	uint64_t dentCount = 0;
	struct dirent* dent = NULL;
	const uint32_t bufferSize = 0x8000;
	uint8_t* buffer = k_malloc(bufferSize);
	if (buffer == NULL)
	{
		WriteLog(LL_Error, "could not allocate buffer");
		messagemanager_sendErrorResponse(MessageCategory_File, -ENOMEM);
		goto cleanup;
	}
	(void)memset(buffer, 0, bufferSize);

	while (kgetdents(directoryHandle, (char*)buffer, bufferSize) > 0)
	{
		dent = (struct dirent*)buffer;

		while (dent->d_fileno)
		{
			if (dent->d_type == DT_UNKNOWN)
				break;

			dentCount++;
			dent = (struct dirent*)((uint8_t*)dent + dent->d_reclen);
		}
	}

	kclose(directoryHandle);
	directoryHandle = -1;

	oni_threadEscape(curthread, &threadInfo);
	directoryHandle = kopen(request->path, 0x0000 | 0x00020000, 0777);
	oni_threadRestore(curthread, &threadInfo);

	// Check if we opened the directory fine
	if (directoryHandle < 0)
	{
		WriteLog(LL_Error, "could not open directory (%s) (%d).", request->path, directoryHandle);

		// Free the buffer
		if (buffer)
			k_free(buffer);
		buffer = NULL;

		messagemanager_sendErrorResponse(MessageCategory_File, directoryHandle);
		goto cleanup;
	}

	struct fileexplorer_getdentsResponse_t response =
	{
		.totalDentCount = dentCount,
	};

	// Send the response back, this creates a copy of the data
	struct messagecontainer_t* responseContainer = messagecontainer_createMessage(MessageCategory_File, (uint32_t)directoryHandle, false, &response, sizeof(response));
	if (responseContainer == NULL)
	{
		WriteLog(LL_Error, "could not allocate response");

		// Free the buffer
		if (buffer)
			k_free(buffer);
		buffer = NULL;

		// Close the handle
		kclose(directoryHandle);

		messagemanager_sendErrorResponse(MessageCategory_File, -ENOMEM);
		goto cleanup;
	}

	messagemanager_sendResponse(responseContainer);
	messagecontainer_release(responseContainer);

	// TODO: Close handles where needed

	int32_t connectionSocket = rpcserver_findSocketFromThread(mira_getFramework()->framework.rpcServer, curthread);
	
	(void)memset(buffer, 0, bufferSize);
	dent = NULL;
	while (kgetdents(directoryHandle, (char*)buffer, bufferSize) > 0)
	{
		dent = (struct dirent*)buffer;
		while (dent->d_fileno)
		{
			if (dent->d_type == DT_UNKNOWN)
				break;

			uint32_t allocatedDentSize = sizeof(struct fileexplorer_dent_t) + dent->d_namlen;
			struct fileexplorer_dent_t* allocatedDent = k_malloc(allocatedDentSize);
			if (allocatedDent == NULL)
			{
				// At this point it's too late in the loop to try and recover the client
				WriteLog(LL_Error, "could not allocate dent");

				// Free the buffer
				if (buffer)
					k_free(buffer);
				buffer = NULL;

				// Close the handle
				kclose(directoryHandle);

				goto cleanup;
			}

			// Zero out the dent
			(void)memset(allocatedDent, 0, allocatedDentSize);

			allocatedDent->fileno = dent->d_fileno;
			allocatedDent->reclen = dent->d_reclen;
			allocatedDent->type = dent->d_type;
			allocatedDent->namlen = dent->d_namlen;
			memcpy(allocatedDent->name, dent->d_name, dent->d_namlen);

			// Write it directly out to socket
			WriteLog(LL_Debug, "writing dent (%s).", dent->d_name);
			kwrite(connectionSocket, allocatedDent, allocatedDentSize);

			// Free the allocated memory for this dent
			k_free(allocatedDent);
			allocatedDent = NULL;

			dent = (struct dirent*)((uint8_t*)dent + dent->d_reclen);
		}
	}

	// Free the buffer
	if (buffer)
		k_free(buffer);
	buffer = NULL;

	// Close the handle
	kclose(directoryHandle);

cleanup:
	messagecontainer_release(container);
}

void fileexplorer_stat_callback(struct messagecontainer_t* container)
{ 
	WriteLog(LL_Error, "stat is not implemented");

	messagemanager_sendErrorResponse(MessageCategory_File, -ENOEXEC);
}

void fileexplorer_mkdir_callback(struct messagecontainer_t* container)
{ 
	WriteLog(LL_Error, "mkdir is not implemented");

	messagemanager_sendErrorResponse(MessageCategory_File, -ENOEXEC);
}

void fileexplorer_rmdir_callback(struct messagecontainer_t* container)
{ 
	WriteLog(LL_Error, "rmdir is not implemented");

	messagemanager_sendErrorResponse(MessageCategory_File, -ENOEXEC);
}

void fileexplorer_unlink_callback(struct messagecontainer_t* container)
{
	WriteLog(LL_Error, "unlink is not implemented");

	messagemanager_sendErrorResponse(MessageCategory_File, -ENOEXEC);
}