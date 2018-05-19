#include "filetransfer_plugin.h"
#include <oni/framework.h>
#include <oni/utils/kdlsym.h>
#include <oni/messaging/messagemanager.h>
#include <oni/messaging/message.h>
#include <oni/utils/sys_wrappers.h>
#include <sys/dirent.h>
#include <sys/stat.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/log/logger.h>

#ifndef MIN
#define MIN ( x, y ) ( (x) < (y) ? : (x) : (y) )
#endif

enum { FileTransfer_MaxPath = 255 };
enum FileTransferCmds
{
	FileTransfer_Open = 0x58AFA0D4,
	FileTransfer_Close = 0x43F82FDB,
	FileTransfer_GetDents = 0x7433E67A,
	FileTransfer_Read = 0x64886217,
	FileTransfer_ReadFile = 0x13B55E44,
	FileTransfer_Write = 0x2D92D440,
	FileTransfer_WriteFile = 0x3B91E812,
	FileTransfer_Delete = 0xB74A88DC,
	FileTransfer_Move = 0x13E11408,
	FileTransfer_Stat = 0xDC67DC51,
	FileTransfer_Mkdir = 0x5EB439FE,
	FileTransfer_Rmdir = 0x996F6671,
	FileTransfer_COUNT
};

// Request with an open handle, rest zeroed. Response with all info filled
struct filetransfer_getdents_t
{
	// Previously opened handle
	int32_t handle;

	// Dirent type
	uint8_t type;

	// Path of a return entry
	char path[FileTransfer_MaxPath];
};

struct filetransfer_mkdir_t
{
	char path[FileTransfer_MaxPath];
	int mode;
};

struct filetransfer_rmdir_t
{
	char path[FileTransfer_MaxPath];
};

struct filetransfer_open_t
{
	// File or folder path
	char path[255];

	// Flags to open with
	int32_t flags;

	// Mode to open with
	int32_t mode;

	// Handle handle returned
	int32_t handle;
};

struct filetransfer_close_t
{
	// File or folder handle
	int32_t handle;
};

struct filetransfer_readfile_t
{
	// File handle
	int32_t handle;
	// Size of file
	uint64_t size;
	// Payload of the file is sent afterwards
	//uint8_t data[size];
};

struct filetransfer_writefile_t
{
	// File handle
	int32_t handle;
	// File data to write
	uint64_t size;
	// Payload of the file is sent
	//uint8_t data[size];
};

struct filetransfer_read_t
{
	// File handle
	int32_t handle;

	// Offset to read data in file
	uint64_t offset;

	// Size to read
	uint64_t size;

	// Payload of the data is sent
};

struct filetransfer_write_t
{
	// File handle
	int32_t handle;

	// Offset in file to write
	uint64_t offset;
	// Size of the data to write
	uint64_t size;
	// Payload is sent afterwards
};

struct filetransfer_delete_t
{
	// Path to delete
	char path[FileTransfer_MaxPath];
};

struct filetransfer_stat_t
{
	// Path to stat
	char path[FileTransfer_MaxPath];
	// protection mode
	int32_t mode;
	// user id owner;
	int32_t uid;
	// group id owner
	int32_t gid;
	// size
	uint64_t size;
};

void filetransfer_open_callback(struct allocation_t* ref);
void filetransfer_close_callback(struct allocation_t* ref);
void filetransfer_read_callback(struct allocation_t* ref);
void filetransfer_readfile_callback(struct allocation_t* ref);
void filetransfer_write_callback(struct allocation_t* ref);
void filetransfer_writefile_callback(struct allocation_t* ref);
void filetransfer_getdents_callback(struct allocation_t* ref);
void filetransfer_delete_callback(struct allocation_t* ref);
void filetransfer_stat_callback(struct allocation_t* ref);
void filetransfer_mkdir_callback(struct allocation_t* ref);
void filetransfer_rmdir_callback(struct allocation_t* ref);
void filetransfer_unlink_callback(struct allocation_t* ref);

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
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_FILE, FileTransfer_Open, filetransfer_open_callback);
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_FILE, FileTransfer_Close, filetransfer_close_callback);
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_FILE, FileTransfer_Read, filetransfer_read_callback);
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_FILE, FileTransfer_ReadFile, filetransfer_readfile_callback);
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_FILE, FileTransfer_Write, filetransfer_write_callback);
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_FILE, FileTransfer_WriteFile, filetransfer_writefile_callback);
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_FILE, FileTransfer_GetDents, filetransfer_getdents_callback);
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_FILE, FileTransfer_Delete, filetransfer_delete_callback);
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_FILE, FileTransfer_Stat, filetransfer_stat_callback);
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_FILE, FileTransfer_Mkdir, filetransfer_mkdir_callback);
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_FILE, FileTransfer_Rmdir, filetransfer_rmdir_callback);

	return true;
}

uint8_t filetransfer_unload(struct filetransfer_plugin_t* plugin)
{
	return true;
}

void filetransfer_stat_callback(struct allocation_t* ref)
{
	if (!ref)
		return;

	struct message_t* message = __get(ref);
	if (!message)
		return;

	if (message->header.request != 1)
		goto cleanup;


	if (!message->payload)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOMEM);
		goto cleanup;
	}

	struct filetransfer_stat_t* fileStat = (struct filetransfer_stat_t*)message->payload;
	struct stat stat = { 0 };

	int result = kstat(fileStat->path, &stat);
	if (result < 0)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, result);
		WriteLog(LL_Error, "kstat %s returned %d", fileStat->path, result);
		goto cleanup;
	}

	// Send success message
	messagemanager_sendSuccessMessage(gFramework->messageManager, ref);

	// Fill out the struct
	fileStat->gid = stat.st_gid;
	fileStat->uid = stat.st_uid;
	fileStat->size = stat.st_size;
	fileStat->mode = stat.st_mode;

	// Send that shit back
	kwrite(message->socket, fileStat, sizeof(*fileStat));

cleanup:
	__dec(ref);
}

void filetransfer_open_callback(struct allocation_t* ref)
{
	if (!ref)
		return;

	struct message_t* message = __get(ref);
	if (!message)
		return;

	if (message->header.request != 1)
		goto cleanup;


	if (!message->payload)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOMEM);
		goto cleanup;
	}

	struct filetransfer_open_t* openRequest = (struct filetransfer_open_t*)message->payload;

	WriteLog(LL_Debug, "[+] openRequest %p %d %d.", openRequest, openRequest->flags, openRequest->mode);

	openRequest->handle = kopen(openRequest->path, openRequest->flags, openRequest->mode);
	if (openRequest->handle < 0)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, openRequest->handle);
		WriteLog(LL_Error, "[-] could not open file %s %d.", openRequest->path, openRequest->handle);
		goto cleanup;
	}

	messagemanager_sendSuccessMessage(gFramework->messageManager, ref);

	// Send to socket if needed
	if (message->socket > 0)
		kwrite(message->socket, openRequest, sizeof(*openRequest));

cleanup:
	__dec(ref);
}

void filetransfer_rmdir_callback(struct allocation_t* ref)
{
	if (!ref)
		return;

	struct message_t* message = __get(ref);
	if (!message)
		return;

	if (message->header.request != true)
		goto cleanup;

	if (!message->payload)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOMEM);
		goto cleanup;
	}

	struct filetransfer_rmdir_t* rmdirRequest = (struct filetransfer_rmdir_t*)message->payload;

	WriteLog(LL_Debug, "rmdir (%s)", rmdirRequest->path);

	int result = krmdir(rmdirRequest->path);
	if (result == -1)
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOENT);
	else
		messagemanager_sendSuccessMessage(gFramework->messageManager, ref);

cleanup:
	__dec(ref);
}

void filetransfer_mkdir_callback(struct allocation_t* ref)
{
	if (!ref)
		return;

	struct message_t* message = __get(ref);
	if (!message)
		return;

	if (message->header.request != true)
		goto cleanup;

	if (!message->payload)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOMEM);
		goto cleanup;
	}

	struct filetransfer_mkdir_t* mkdirRequest = (struct filetransfer_mkdir_t*)message->payload;

	WriteLog(LL_Debug, "mk (%s) (%d)", mkdirRequest->path, mkdirRequest->mode);

	int result = kmkdir(mkdirRequest->path, mkdirRequest->mode);
	if (result == -1)
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, EACCES);
	else
		messagemanager_sendSuccessMessage(gFramework->messageManager, ref);

cleanup:
	__dec(ref);
}

void filetransfer_close_callback(struct allocation_t* ref)
{
	struct message_t* message = __get(ref);
	if (!message)
		return;

	if (message->header.request != 1)
		goto cleanup;

	if (!message->payload)
		goto cleanup;

	struct filetransfer_close_t* closeRequest = (struct filetransfer_close_t*)message->payload;

	WriteLog(LL_Debug, "[+] closeRequest %p %d", closeRequest, closeRequest->handle);

	kclose(closeRequest->handle);

cleanup:
	__dec(ref);
}

void filetransfer_read_callback(struct allocation_t* ref)
{
	// TODO: implement
	if (!ref)
		return;

	struct message_t* message = __get(ref);
	if (!message)
		return;

	if (message->header.request != 1)
		goto cleanup;

	if (!message->payload)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOMEM);
		goto cleanup;
	}

	struct filetransfer_read_t* readRequest = (struct filetransfer_read_t*)message->payload;

	// Verify that the handle is valid
	if (readRequest->handle < 0)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOENT);
		goto cleanup;
	}

	WriteLog(LL_Debug, "[+] creating struct");
	struct stat statbuf;
	kmemset(&statbuf, 0, sizeof(statbuf));

	// Get the file size
	WriteLog(LL_Debug, "[+] calling stat(%d, %p);", readRequest->handle, &statbuf);
	int res = kfstat(readRequest->handle, &statbuf);
	if (res < 0)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, res);
		WriteLog(LL_Error, "[-] could not get %d handle stat: %d", readRequest->handle, res);
		goto cleanup;
	}

	// Verify that the offset is within bounds
	if (readRequest->offset > statbuf.st_size)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, EIO);
		WriteLog(LL_Error, "offset > fileSize");
		goto cleanup;
	}

	// Check to see if the offset + size is greater than the length
	if (readRequest->offset + readRequest->size > statbuf.st_size)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, EIO);
		WriteLog(LL_Error, "offset + size > fileSize");
		goto cleanup;

	}

	const int bufferSize = 0x4000;
	uint8_t* buffer = (uint8_t*)kmalloc(bufferSize);
	if (!buffer)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOMEM);
		goto cleanup;
	}

	// Send success message sayuing we got all of our information
	messagemanager_sendSuccessMessage(gFramework->messageManager, ref);

	// Seek to the position in file
	klseek(readRequest->handle, readRequest->offset, 0);

	// Zero the buffer
	kmemset(buffer, 0, bufferSize);

	// Write the header
	message->header.request = 0;
	kwrite(message->socket, readRequest, sizeof(*readRequest));

	// Calculate the blocks and leftover data
	uint64_t blocks = readRequest->size / bufferSize;
	uint64_t leftover = readRequest->size % bufferSize;

	// Write all blocks
	for (uint64_t i = 0; i < blocks; ++i)
	{
		kread(readRequest->handle, buffer, bufferSize);
		kwrite(message->socket, buffer, bufferSize);
		kmemset(buffer, 0, bufferSize);
	}

	// Write leftover data
	kread(readRequest->handle, buffer, leftover);
	kwrite(message->socket, buffer, leftover);

	kfree(buffer, bufferSize);
cleanup:
	__dec(ref);
}

void filetransfer_readfile_callback(struct allocation_t* ref)
{
	struct message_t* message = __get(ref);
	if (!message)
		return;

	if (message->header.request != 1)
		goto cleanup;

	// Validate that the socket is valid before we continue
	if (message->socket < 0)
	{
		WriteLog(LL_Error, "[-] socket not set");
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOMEM);
		goto cleanup;
	}

	struct filetransfer_readfile_t* readRequest = (struct filetransfer_readfile_t*)message->payload;
	if (!readRequest)
	{
		WriteLog(LL_Error, "[-] invalid payload");
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOMEM);
		goto cleanup;
	}

	WriteLog(LL_Debug, "[+] creating struct");
	struct stat statbuf;
	kmemset(&statbuf, 0, sizeof(statbuf));

	// Get the file size
	WriteLog(LL_Debug, "[+] calling stat(%d, %p);", readRequest->handle, &statbuf);
	int res = kfstat(readRequest->handle, &statbuf);
	if (res < 0)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, res);
		WriteLog(LL_Error, "[-] could not get %d handle stat: %d", readRequest->handle, res);
		goto cleanup;
	}

	const int bufferSize = 0x4000;
	uint8_t* buffer = (uint8_t*)kmalloc(bufferSize);
	if (!buffer)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOMEM);
		goto cleanup;
	}

	// Send success
	messagemanager_sendSuccessMessage(gFramework->messageManager, ref);

	// Set the size
	readRequest->size = statbuf.st_size;
	WriteLog(LL_Debug, "[+] file size: %llx", readRequest->size);

	// Zero the buffer
	kmemset(buffer, 0, bufferSize);

	// Write the header
	message->header.request = 0;
	kwrite(message->socket, readRequest, sizeof(*readRequest));

	// Calculate the blocks and leftover data
	uint64_t blocks = statbuf.st_size / bufferSize;
	uint64_t leftover = statbuf.st_size % bufferSize;

	// Write all blocks
	for (uint64_t i = 0; i < blocks; ++i)
	{
		kread(readRequest->handle, buffer, bufferSize);
		kwrite(message->socket, buffer, bufferSize);
		kmemset(buffer, 0, bufferSize);
	}

	// Write leftover data
	kread(readRequest->handle, buffer, leftover);
	kwrite(message->socket, buffer, leftover);

	kfree(buffer, bufferSize);

cleanup:
	__dec(ref);
}

void filetransfer_write_callback(struct allocation_t* ref)
{
	if (!ref)
		return;

	struct message_t* message = __get(ref);
	if (!message)
		return;

	if (message->header.request != 1)
		goto cleanup;

	if (!message->payload)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOMEM);
		goto cleanup;
	}

	struct filetransfer_write_t* writeRequest = (struct filetransfer_write_t*)message->payload;

	// Verify that the handle is valid
	if (writeRequest->handle < 0)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOENT);
		goto cleanup;
	}

	const int bufferSize = 0x4000;
	uint8_t* buffer = (uint8_t*)kmalloc(bufferSize);
	if (!buffer)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOMEM);
		goto cleanup;
	}

	// Send success message sayuing we got all of our information
	messagemanager_sendSuccessMessage(gFramework->messageManager, ref);

	// Seek to the position in file
	klseek(writeRequest->handle, writeRequest->offset, 0);

	// Zero the buffer
	kmemset(buffer, 0, bufferSize);

	// Write the header
	message->header.request = 0;

	// Calculate the blocks and leftover data
	uint64_t blocks = writeRequest->size / bufferSize;
	uint64_t leftover = writeRequest->size % bufferSize;

	// Write all blocks
	for (uint64_t i = 0; i < blocks; ++i)
	{
		kread(message->socket, buffer, bufferSize);
		kwrite(writeRequest->handle, buffer, bufferSize);
		kmemset(buffer, 0, bufferSize);
	}

	// Write leftover data
	kread(message->socket, buffer, leftover);
	kwrite(writeRequest->handle, buffer, leftover);

	kfree(buffer, bufferSize);
cleanup:
	__dec(ref);
}

void filetransfer_writefile_callback(struct allocation_t* ref)
{
	if (!ref)
		return;

	struct message_t* message = __get(ref);
	if (!message)
		return;

	if (message->header.request != 1)
		goto cleanup;

	if (!message->payload)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOMEM);
		goto cleanup;
	}

	struct filetransfer_writefile_t* writeRequest = (struct filetransfer_writefile_t*)message->payload;

	// Verify that the handle is valid
	if (writeRequest->handle < 0)
	{
		WriteLog(LL_Error, "invalid handle");
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOENT);
		goto cleanup;
	}

	const int bufferSize = 0x4000;
	uint8_t* buffer = (uint8_t*)kmalloc(bufferSize);
	if (!buffer)
	{
		WriteLog(LL_Error, "could not allocate temporary buffer");
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOMEM);
		goto cleanup;
	}

	// Seek to the position in file
	klseek(writeRequest->handle, 0, 0);

	uint64_t totalSize = writeRequest->size;
	uint64_t dataReceived = 0;

	while (dataReceived < totalSize)
	{
		// seek to the right position
		klseek(writeRequest->handle, dataReceived, 0);

		int32_t currentSize = MIN(0x4000, writeRequest->size - dataReceived);
		kmemset(buffer, 0, bufferSize);

		int recvSize = krecv(message->socket, buffer, currentSize, 0);
		if (recvSize < 0)
		{
			WriteLog(LL_Error, "could not recv %d", recvSize);
			goto cleanup;
		}

		dataReceived += recvSize;

		kwrite(writeRequest->handle, buffer, currentSize);
	}

	kfree(buffer, bufferSize);
	buffer = NULL;

	messagemanager_sendSuccessMessage(gFramework->messageManager, ref);

cleanup:
	__dec(ref);
}

void filetransfer_getdents_callback(struct allocation_t* ref)
{
	if (!ref)
		return;

	struct message_t* message = __get(ref);
	if (!message)
		return;

	if (message->header.request != 1)
		goto cleanup;

	if (!message->payload)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOMEM);
		WriteLog(LL_Error, "[-] invalid payload");
		goto cleanup;
	}

	struct filetransfer_getdents_t* payload = (struct filetransfer_getdents_t*)message->payload;

	WriteLog(LL_Debug, "[+] getdents %s", payload->path);

	int handle = kopen(payload->path, 0x0000 | 0x00020000, 0);
	if (handle < 0)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, handle);
		WriteLog(LL_Error, "[-] could not open path %s", payload->path);
		goto cleanup;
	}

	// Run through once to get the count
	uint64_t dentCount = 0;
	struct dirent* dent = 0;
	const uint32_t bufferSize = 0x8000;
	char* buffer = kmalloc(bufferSize);
	if (!buffer)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOMEM);
		WriteLog(LL_Error, "could not allocate memory");
		goto cleanup;
	}

	// Zero out the buffer size
	kmemset(buffer, 0, bufferSize);

	// Get all of the directory entries the first time to get the count
	while (kgetdents(handle, buffer, bufferSize) > 0)
	{
		dent = (struct dirent*)buffer;

		while (dent->d_fileno)
		{
			if (dent->d_type == 0)
				break;

			dentCount++;
			dent = (struct dirent*)((uint8_t*)dent + dent->d_reclen);
		}
	}

	WriteLog(LL_Debug, "[+] closing handle");
	kclose(handle);

	// Re-open the handle
	handle = kopen(payload->path, 0x0000 | 0x00020000, 0);
	if (handle < 0)
	{
		kfree(buffer, bufferSize);
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, handle);
		WriteLog(LL_Error, "[-] could not open path %s", payload->path);
		goto cleanup;
	}

	// Return success code
	messagemanager_sendSuccessMessage(gFramework->messageManager, ref);

	// Send the dent count
	kwrite(message->socket, &dentCount, sizeof(dentCount));

	struct filetransfer_getdents_t writeDent;
	kmemset(&writeDent, 0, sizeof(writeDent));
	kmemset(buffer, 0, bufferSize);

	dent = 0;
	while (kgetdents(handle, buffer, bufferSize) > 0)
	{
		dent = (struct dirent*)buffer;
		while (dent->d_fileno)
		{
			//printf("[+] fileno %d\n", dent->d_fileno);

			if (dent->d_type == 0)
				break;

			// Zero out the dent
			kmemset(&writeDent, 0, sizeof(writeDent));

			// Copy over the name, truncating it if need be
			int nameLength = dent->d_namlen > 255 ? 255 : dent->d_namlen;

			kmemcpy(writeDent.path, dent->d_name, nameLength);

			writeDent.type = dent->d_type;
			writeDent.handle = dent->d_fileno;

			kwrite(message->socket, &writeDent, sizeof(writeDent));

			//printf("[+] writing dent %p\n", dent);
			dent = (struct dirent*)((uint8_t*)dent + dent->d_reclen);
		}
	}

	kfree(buffer, bufferSize);
	kclose(handle);

cleanup:
	__dec(ref);
}

void filetransfer_delete_callback(struct allocation_t* ref)
{
	if (!ref)
		return;

	struct message_t* message = __get(ref);
	if (!message)
		return;

	if (message->header.request != 1)
		goto cleanup;

	if (!message->payload)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, ref, ENOMEM);
		goto cleanup;
	}

	struct filetransfer_delete_t* deleteRequest = (struct filetransfer_delete_t*)message->payload;

	message->header.request = 0;
	message->header.error_type = kunlink(deleteRequest->path);

	messagemanager_sendSuccessMessage(gFramework->messageManager, ref);

cleanup:
	__dec(ref);
}
