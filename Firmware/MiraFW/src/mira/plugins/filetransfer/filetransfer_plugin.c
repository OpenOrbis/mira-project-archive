#include "filetransfer_plugin.h"
#include <oni/framework.h>
#include <oni/utils/kdlsym.h>
#include <oni/messaging/messagemanager.h>
#include <oni/messaging/message.h>
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

#include <nanopb/mirabuiltin.pb.h>
#include <nanopb/pb_common.h>
#include <nanopb/pb_encode.h>
#include <nanopb/pb_decode.h>

#include "fileexplorer.pb.h"

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

void filetransfer_open_callback(struct ref_t* reference);
void filetransfer_close_callback(struct ref_t* reference);
void filetransfer_read_callback(struct ref_t* reference);
void filetransfer_readfile_callback(struct ref_t* reference);
void filetransfer_write_callback(struct ref_t* reference);
void filetransfer_writefile_callback(struct ref_t* reference);
void filetransfer_getdents_callback(struct ref_t* reference);
void filetransfer_delete_callback(struct ref_t* reference);
void filetransfer_stat_callback(struct ref_t* reference);
void filetransfer_mkdir_callback(struct ref_t* reference);
void filetransfer_rmdir_callback(struct ref_t* reference);
void filetransfer_unlink_callback(struct ref_t* reference);

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
	messagemanager_registerCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_Open, filetransfer_open_callback);
	messagemanager_registerCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_Close, filetransfer_close_callback);
	messagemanager_registerCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_Read, filetransfer_read_callback);
	messagemanager_registerCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_ReadFile, filetransfer_readfile_callback);
	messagemanager_registerCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_Write, filetransfer_write_callback);
	messagemanager_registerCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_WriteFile, filetransfer_writefile_callback);
	messagemanager_registerCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_GetDents, filetransfer_getdents_callback);
	messagemanager_registerCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_Delete, filetransfer_delete_callback);
	messagemanager_registerCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_Stat, filetransfer_stat_callback);
	messagemanager_registerCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_Mkdir, filetransfer_mkdir_callback);
	messagemanager_registerCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_Rmdir, filetransfer_rmdir_callback);

	return true;
}

uint8_t filetransfer_unload(struct filetransfer_plugin_t* plugin)
{
	messagemanager_unregisterCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_Open, filetransfer_open_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_Close, filetransfer_close_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_Read, filetransfer_read_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_ReadFile, filetransfer_readfile_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_Write, filetransfer_write_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_WriteFile, filetransfer_writefile_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_GetDents, filetransfer_getdents_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_Delete, filetransfer_delete_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_Stat, filetransfer_stat_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_Mkdir, filetransfer_mkdir_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MessageCategory_FILE, FileTransfer_Rmdir, filetransfer_rmdir_callback);

	return true;
}

#define SEND_ERROR_RESPONSE(ResponseType, Header, Error) \
	{ size_t _bufferSize = 2 * PAGE_SIZE; \
	ResponseType _response; \
	_response.header = Header; \
	_response.header.error = Error; \
	uint8_t* _buffer = k_malloc(_bufferSize); \
	if (!_buffer) goto cleanup; \
	memset(_buffer, 0, _bufferSize); \
	pb_ostream_t _stream = pb_ostream_from_buffer(_buffer, _bufferSize); \
	if (!pb_encode(&_stream, ResponseType ## _fields, &_response)) { \
	WriteLog(LL_Error, "could not encode response (%s)", _stream.errmsg); \
	goto cleanup; } \
	size_t _messageSize = _stream.bytes_written; \
	struct ref_t* _responseRef = ref_alloc(_messageSize); \
	if (!_responseRef) {\
	WriteLog(LL_Error, "could not allocate reference"); \
	goto cleanup; } \
	uint8_t* _refData = ref_getData(_responseRef); \
	size_t _refDataSize = ref_getSize(_responseRef); \
	if (_refData == NULL || _refDataSize != _messageSize) { \
	WriteLog(LL_Error, "could not get ref data, or invalid size (%llx)", _refDataSize); \
	ref_release(_responseRef); \
	goto cleanup; } \
	memset(_refData, 0, _refDataSize); \
	memcpy(_refData, _buffer, _messageSize); \
	messagemanager_sendResponse(_responseRef); \
	ref_release(_responseRef); }

#define SEND_RESPONSE(ResponseType, Response, Header) \
	{ size_t _bufferSize = 2 * PAGE_SIZE; \
Response.header = Header; \
Response.header.error = 0; \
uint8_t* _buffer = k_malloc(_bufferSize); \
if (!_buffer) goto cleanup; \
memset(_buffer, 0, _bufferSize); \
pb_ostream_t _stream = pb_ostream_from_buffer(_buffer, _bufferSize); \
if (!pb_encode(&_stream, ResponseType##_fields, &Response)) { \
	WriteLog(LL_Error, "could not encode response (%s)", _stream.errmsg); \
	goto cleanup; \
} \
size_t _messageSize = _stream.bytes_written; \
struct ref_t* _responseRef = ref_alloc(_messageSize); \
if (!_responseRef) { \
	WriteLog(LL_Error, "could not allocate reference"); \
	goto cleanup; \
} \
uint8_t* _refData = ref_getData(_responseRef); \
size_t _refDataSize = ref_getSize(_responseRef); \
if (_refData == NULL || _refDataSize != _messageSize) \
{ \
	WriteLog(LL_Error, "could not get ref data, or invalid size (%llx)", _refDataSize); \
	ref_release(_responseRef); \
	goto cleanup; \
} \
memset(_refData, 0, _refDataSize); \
memcpy(_refData, _buffer, _messageSize); \
messagemanager_sendResponse(_responseRef); \
ref_release(_responseRef); \
}

// CRITICAL TEMP TRACKING
//{
//// We have to allocate some memory for the protobuf stuff to work
//size_t responseDataBufferSize = 2 * PAGE_SIZE;
//uint8_t* responseDataBuffer = k_malloc(responseDataBufferSize);
//if (!responseDataBuffer)
//	goto cleanup;

//// Zero out the response
//memset(responseDataBuffer, 0, responseDataBufferSize);

//StatResponse response;
//pb_ostream_t stream = pb_ostream_from_buffer(responseDataBuffer, responseDataBufferSize);
//if (!pb_encode(&stream, StatResponse_fields, &response))
//{
//	WriteLog(LL_Error, "could not encode response errmsg: (%s) state: (%p).", stream.errmsg, stream.state);
//	goto cleanup;
//}

//// Get the actual size to send
//size_t messageSize = stream.bytes_written;

//// Create a new reference for the size of the message
//struct ref_t* responseRef = ref_alloc(messageSize);
//if (!responseRef)
//{
//	WriteLog(LL_Error, "could not allocate ref memory");
//	goto cleanup;
//}

//// Get the data
//uint8_t* refData = ref_getData(responseRef);
//size_t refDataSize = ref_getSize(responseRef);

//if (refData == NULL || refDataSize != messageSize)
//{
//	WriteLog(LL_Error, "could not get ref data, or invalid size (%llx)", refDataSize);
//	ref_release(responseRef);
//	goto cleanup;
//}

//memset(refData, 0, refDataSize);
//memcpy(refData, responseDataBuffer, messageSize);

//messagemanager_sendResponse(responseRef);
//ref_release(responseRef);
//}

void filetransfer_stat_callback(struct ref_t* reference)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	if (!reference)
		return;

	uint8_t* messageData = ref_getDataAndAcquire(reference);
	if (!messageData)
		goto cleanup;

	size_t messageDataSize = ref_getSize(reference);

	StatRequest message;
	pb_istream_t stream = pb_istream_from_buffer(messageData, messageDataSize);
	if (!pb_decode(&stream, StatRequest_fields, &message))
	{
		WriteLog(LL_Error, "could not decode header (%s).", PB_GET_ERROR(&stream));
		goto cleanup;
	}

	// Verify that our reference has enough space for our payload
	if (messageDataSize < sizeof(StatRequest))
	{
		WriteLog(LL_Error, "not enough space to hold payload");

		SEND_ERROR_RESPONSE(StatResponse, message.header, -ENOMEM);
		goto cleanup;
	}
	struct stat stat;
	memset(&stat, 0, sizeof(stat));

	int result = kstat(message.path, &stat);
	if (result < 0)
	{
		WriteLog(LL_Error, "kstat (%s) returned (%d)", message.path, result);

		SEND_ERROR_RESPONSE(StatResponse, message.header, result);
		goto cleanup;
	}

	// Fill out the struct
	StatResponse response;
	response.header = message.header;
	response.header.error = 0;

	response.dev = stat.st_dev;
	response.has_dev = true;

	response.ino = stat.st_ino;
	response.has_ino = true;

	response.mode = stat.st_mode;
	response.has_mode = true;

	response.nlink = stat.st_nlink;
	response.has_nlink = true;

	response.uid = stat.st_uid;
	response.has_uid = true;

	response.gid = stat.st_gid;
	response.has_gid = true;

	response.rdev = stat.st_rdev;
	response.has_rdev = true;

	response.atim.sec = stat.st_atim.tv_sec;
	response.atim.nsec = stat.st_atim.tv_nsec;
	response.has_atim = true;

	response.mtim.sec = stat.st_mtim.tv_sec;
	response.mtim.nsec = stat.st_mtim.tv_nsec;
	response.has_mtim = true;

	response.ctim.sec = stat.st_ctim.tv_sec;
	response.ctim.nsec = stat.st_ctim.tv_nsec;
	response.has_ctim = true;

	response.size = stat.st_size;
	response.has_size = true;

	response.blocks = stat.st_blocks;
	response.has_blocks = true;

	response.blksize = stat.st_blksize;
	response.has_blksize = true;

	response.flags = stat.st_flags;
	response.has_flags = true;

	response.gen = stat.st_gen;
	response.has_gen = true;

	response.lspare = stat.st_lspare;
	response.has_lspare = true;

	response.birthtim.nsec = stat.st_birthtim.tv_nsec;
	response.birthtim.sec = stat.st_birthtim.tv_sec;
	response.has_birthtim = true;
	// Send success message

	SEND_RESPONSE(StatResponse, response, message.header);
cleanup:
	ref_release(reference);
}

void filetransfer_open_callback(struct ref_t* reference)
{
	if (!reference)
		return;

	uint8_t* messageData = ref_getDataAndAcquire(reference);
	if (!messageData)
		return;

	size_t messageDataSize = ref_getSize(reference);

	OpenRequest message;
	pb_istream_t stream = pb_istream_from_buffer(messageData, messageDataSize);
	if (!pb_decode(&stream, OpenRequest_fields, &message))
	{
		WriteLog(LL_Error, "could not decode header (%s).", PB_GET_ERROR(&stream));
		goto cleanup;
	}

	WriteLog(LL_Debug, "[+] openRequest %s %d %d.", message.path, message.flags, message.mode);

	struct thread_info_t threadInfo;
	memset(&threadInfo, 0, sizeof(threadInfo));

	oni_threadEscape(curthread, &threadInfo);

	int32_t handle = kopen(message.path, message.flags, message.mode);

	oni_threadRestore(curthread, &threadInfo);

	if (handle < 0)
	{
		WriteLog(LL_Error, "[-] could not open file %s %d.", message.path, handle);
		SEND_ERROR_RESPONSE(OpenResponse, message.header, handle);
		goto cleanup;
	}

	OpenResponse response;
	response.handle = handle;
	response.has_handle = true;

	SEND_RESPONSE(OpenResponse, response, message.header);

cleanup:
	ref_release(reference);
}

void filetransfer_rmdir_callback(struct ref_t* reference)
{
	if (!reference)
		return;

	uint8_t* messageData = ref_getDataAndAcquire(reference);
	if (!messageData)
	{
		WriteLog(LL_Error, "could not get message data.");
		goto cleanup;
	}

	size_t messageDataSize = ref_getSize(reference);

	RemoveDirectoryRequest request;
	pb_istream_t stream = pb_istream_from_buffer(messageData, messageDataSize);
	if (!pb_decode(&stream, RemoveDirectoryRequest_fields, &request))
	{
		WriteLog(LL_Error, "could not decode header (%s).", PB_GET_ERROR(&stream));
		goto cleanup;
	}

	WriteLog(LL_Debug, "rmdir (%s)", request.path);

	int result = krmdir(request.path);

	RemoveDirectoryResponse response;

	if (result == -1)
	{
		SEND_ERROR_RESPONSE(RemoveDirectoryResponse, request.header, -EPERM);
	}
	else
	{
		SEND_RESPONSE(RemoveDirectoryResponse, response, request.header);
	}

cleanup:
	ref_release(reference);
}

void filetransfer_mkdir_callback(struct ref_t* reference)
{
	if (!reference)
		return;

	uint8_t* messageData = ref_getDataAndAcquire(reference);
	if (!messageData)
	{
		WriteLog(LL_Error, "could not get message data.");
		goto cleanup;
	}

	size_t messageDataSize = ref_getSize(reference);

	MakeDirectoryRequest request;
	pb_istream_t stream = pb_istream_from_buffer(messageData, messageDataSize);
	if (!pb_decode(&stream, MakeDirectoryRequest_fields, &request))
	{
		WriteLog(LL_Error, "could not decode header (%s).", PB_GET_ERROR(&stream));
		goto cleanup;
	}

	WriteLog(LL_Debug, "mkdir (%s)", request.path);

	int result = kmkdir(request.path, request.mode);

	MakeDirectoryResponse response;

	if (result == -1)
	{
		SEND_ERROR_RESPONSE(MakeDirectoryResponse, request.header, -EPERM);
	}
	else
	{
		SEND_RESPONSE(MakeDirectoryResponse, response, request.header);
	}

cleanup:
	ref_release(reference);
}

void filetransfer_close_callback(struct ref_t* reference)
{
	if (!reference)
		return;

	uint8_t* messageData = ref_getDataAndAcquire(reference);
	if (!messageData)
	{
		WriteLog(LL_Error, "could not get message data.");
		goto cleanup;
	}

	size_t messageDataSize = ref_getSize(reference);

	CloseRequest request;
	pb_istream_t stream = pb_istream_from_buffer(messageData, messageDataSize);
	if (!pb_decode(&stream, CloseRequest_fields, &request))
	{
		WriteLog(LL_Error, "could not decode header (%s).", PB_GET_ERROR(&stream));
		goto cleanup;
	}

	WriteLog(LL_Debug, "close (%d)", request.handle);

	kclose(request.handle);

	CloseResponse response;

	SEND_RESPONSE(CloseResponse, response, request.header);

cleanup:
	ref_release(reference);
}

void filetransfer_read_callback(struct ref_t* reference)
{
	if (!reference)
		return;

	uint8_t* messageData = ref_getDataAndAcquire(reference);
	if (!messageData)
	{
		WriteLog(LL_Error, "could not get message data.");
		goto cleanup;
	}

	size_t messageDataSize = ref_getSize(reference);

	ReadRequest request;
	pb_istream_t stream = pb_istream_from_buffer(messageData, messageDataSize);
	if (!pb_decode(&stream, ReadRequest_fields, &request))
	{
		WriteLog(LL_Error, "could not decode header (%s).", PB_GET_ERROR(&stream));
		goto cleanup;
	}

	WriteLog(LL_Debug, "read (%d)", request.handle);

	// Restrict the maxium size that we can read out
	if (request.size > 0x4000)
	{
		WriteLog(LL_Error, "request size (%llx) is greater than max (%llx).", request.size, 0x4000);
		SEND_ERROR_RESPONSE(ReadResponse, request.header, -EINVAL);
		goto cleanup;
	}

	// If the offset is -1 then we do not seek anywhere before reading
	if (request.offset == -1)
		klseek(request.handle, request.offset, SEEK_SET);

	ReadResponse response;

	// Read out our data
	int32_t result = kread(request.handle, response.data.bytes, request.size);
	if (result < 0)
	{
		WriteLog(LL_Error, "could not read (%d).", result);
		SEND_ERROR_RESPONSE(ReadResponse, request.header, result);
		goto cleanup;
	}

	// Set our size and that we have data
	response.has_data = true;
	response.data.size = request.size;

	// Send the response back
	SEND_RESPONSE(ReadResponse, response, request.header);

cleanup:
	ref_release(reference);
}

void filetransfer_readfile_callback(struct ref_t* reference)
{
	if (!reference)
		return;

	uint8_t* messageData = ref_getDataAndAcquire(reference);
	if (!messageData)
	{
		WriteLog(LL_Error, "could not get message data.");
		goto cleanup;
	}

	size_t messageDataSize = ref_getSize(reference);

	ReadFileRequest request;
	pb_istream_t stream = pb_istream_from_buffer(messageData, messageDataSize);
	if (!pb_decode(&stream, ReadFileRequest_fields, &request))
	{
		WriteLog(LL_Error, "could not decode header (%s).", PB_GET_ERROR(&stream));
		goto cleanup;
	}

	WriteLog(LL_Debug, "readfile (%s)", request.path);

	struct thread_info_t outThreadInfo;
	oni_threadEscape(curthread, &outThreadInfo);

	// If the offset is -1 then we do not seek anywhere before reading
	int32_t handle = kopen(request.path, O_RDONLY, 0);
	if (handle < 0)
	{
		WriteLog(LL_Error, "could not open (%s) returned (%d).", request.path, handle);
		SEND_ERROR_RESPONSE(ReadFileResponse, request.header, handle);
		goto cleanup;
	}

	struct stat fileInfo;
	kfstat(handle, &fileInfo);

	uint64_t fileSize = fileInfo.st_size;
	uint64_t blockSize = 0x4000;
	uint64_t blockCount = fileInfo.st_size / blockSize;

	// Handle single block files that are smaller than our buffer
	if (fileSize < blockSize)
	{
		ReadResponse response;

		// Read out our data
		int32_t result = kread(handle, response.data.bytes, fileSize);
		if (result < 0)
		{
			WriteLog(LL_Error, "could not read (%d).", result);
			SEND_ERROR_RESPONSE(ReadFileResponse, request.header, result);
			goto cleanup;
		}

		// Set our size and that we have data
		response.has_data = true;
		response.data.size = fileSize;

		SEND_RESPONSE(ReadFileResponse, response, request.header);
	}
	else // Handle large file transfers that cannot fit in one buffer
	{
		uint64_t readDataCount = 0;

		uint64_t leftoverDataSize = fileSize % blockSize;

		for (uint64_t blockIndex = 0; blockIndex < blockCount; blockIndex++)
		{
			ReadFileResponse response;

			// Read out the block of data
			ssize_t result = kread(handle, response.data.bytes, blockSize);
			if (result < 0)
			{
				WriteLog(LL_Error, "could not read data (%d).", result);
				SEND_ERROR_RESPONSE(ReadFileResponse, request.header, result);
				goto cleanup;
			}

			// Set the data
			response.has_data = true;
			response.data.size = blockSize;

			// Set our current offset
			response.offset = readDataCount;

			// Add the count of data we are currently at
			readDataCount += result;

			// Send the response back
			SEND_RESPONSE(ReadFileResponse, response, request.header);
		}

		ReadFileResponse response;

		// Read out the block of data
		ssize_t result = kread(handle, response.data.bytes, leftoverDataSize);
		if (result < 0)
		{
			WriteLog(LL_Error, "could not read data (%d).", result);
			SEND_ERROR_RESPONSE(ReadFileResponse, request.header, result);
			goto cleanup;
		}

		// Set the data
		response.has_data = true;
		response.data.size = leftoverDataSize;

		// Set our current offset
		response.offset = readDataCount;

		// Add the count of data we are currently at
		readDataCount += result;

		SEND_RESPONSE(ReadFileResponse, response, request.header);
	}

	// Send the final response, data size of 0 with success
	ReadFileResponse terminatingResponse;
	terminatingResponse.has_data = true;
	terminatingResponse.data.size = 0;

	SEND_RESPONSE(ReadFileResponse, terminatingResponse, request.header);

cleanup:
	ref_release(reference);
}

void filetransfer_write_callback(struct ref_t* reference)
{
//	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
//
//	if (!reference)
//		return;
//
//	struct message_header_t* message = ref_getDataAndAcquire(reference);
//	if (!message)
//		return;
//
//	int32_t clientSocket = pbserver_findSocketFromThread(gFramework->rpcServer, curthread);
//	if (clientSocket < 0)
//	{
//		WriteLog(LL_Error, "client socket not open");
//		messagemanager_sendResponse(reference, -ESOCKTNOSUPPORT);
//		goto cleanup;
//	}
//
//	// Verify that our reference has enough space for our payload
//	if (ref_getSize(reference) < sizeof(struct filetransfer_write_t))
//	{
//		WriteLog(LL_Error, "not enough space to hold payload");
//		messagemanager_sendResponse(reference, -ENOMEM);
//		goto cleanup;
//	}
//
//	struct filetransfer_write_t* writeRequest = message_getData(message);
//
//	// Verify that the handle is valid
//	if (writeRequest->handle < 0)
//	{
//		messagemanager_sendResponse(reference, -ENOENT);
//		goto cleanup;
//	}
//
//	const int bufferSize = 0x4000;
//	uint8_t* buffer = (uint8_t*)kmalloc(bufferSize);
//	if (!buffer)
//	{
//		messagemanager_sendResponse(reference, -ENOMEM);
//		goto cleanup;
//	}
//
//	// Send success message sayuing we got all of our information
//	messagemanager_sendResponse(reference, 0);
//
//	// Seek to the position in file
//	klseek(writeRequest->handle, writeRequest->offset, 0);
//
//	// Zero the buffer
//	memset(buffer, 0, bufferSize);
//
//	// Write the header
//	message->request = false;
//
//	// Calculate the blocks and leftover data
//	uint64_t blocks = writeRequest->size / bufferSize;
//	uint64_t leftover = writeRequest->size % bufferSize;
//
//	// Write all blocks
//	for (uint64_t i = 0; i < blocks; ++i)
//	{
//		kread(clientSocket, buffer, bufferSize);
//		kwrite(writeRequest->handle, buffer, bufferSize);
//		memset(buffer, 0, bufferSize);
//	}
//
//	// Write leftover data
//	kread(clientSocket, buffer, leftover);
//	kwrite(writeRequest->handle, buffer, leftover);
//
//	kfree(buffer, bufferSize);
//
//cleanup:
//	ref_release(reference);
}

void filetransfer_writefile_callback(struct ref_t* reference)
{
//	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
//
//	if (!reference)
//		return;
//
//	struct message_header_t* message = ref_getDataAndAcquire(reference);
//	if (!message)
//		return;
//
//	int32_t clientSocket = pbserver_findSocketFromThread(gFramework->rpcServer, curthread);
//	if (clientSocket < 0)
//	{
//		WriteLog(LL_Error, "could not open socket");
//		messagemanager_sendResponse(reference, -ESOCKTNOSUPPORT);
//		goto cleanup;
//	}
//
//	// Verify that our reference has enough space for our payload
//	if (ref_getSize(reference) < sizeof(struct filetransfer_writefile_t))
//	{
//		WriteLog(LL_Error, "not enough space to hold payload");
//		messagemanager_sendResponse(reference, -ENOMEM);
//		goto cleanup;
//	}
//
//	struct filetransfer_writefile_t* writeRequest = message_getData(message);
//
//	// Verify that the handle is valid
//	if (writeRequest->handle < 0)
//	{
//		WriteLog(LL_Error, "invalid handle");
//		messagemanager_sendResponse(reference, -ENOENT);
//		goto cleanup;
//	}
//
//	const int bufferSize = 0x4000;
//	uint8_t* buffer = (uint8_t*)kmalloc(bufferSize);
//	if (!buffer)
//	{
//		WriteLog(LL_Error, "could not allocate temporary buffer");
//		messagemanager_sendResponse(reference, -ENOMEM);
//		goto cleanup;
//	}
//
//	messagemanager_sendResponse(reference, 0);
//
//	// Seek to the position in file
//	klseek(writeRequest->handle, 0, 0);
//
//	uint64_t totalSize = writeRequest->size;
//	uint64_t dataReceived = 0;
//
//	while (dataReceived < totalSize)
//	{
//		// seek to the right position
//		klseek(writeRequest->handle, dataReceived, 0);
//
//		int32_t currentSize = MIN(0x4000, writeRequest->size - dataReceived);
//		memset(buffer, 0, bufferSize);
//
//		int recvSize = krecv(clientSocket, buffer, currentSize, 0);
//		if (recvSize < 0)
//		{
//			WriteLog(LL_Error, "could not recv %d", recvSize);
//			goto cleanup;
//		}
//
//		dataReceived += recvSize;
//
//		kwrite(writeRequest->handle, buffer, currentSize);
//	}
//
//	kfree(buffer, bufferSize);
//	buffer = NULL;
//
//	
//
//cleanup:
//	ref_release(reference);
}

void filetransfer_getdents_callback(struct ref_t* reference)
{
//	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);
//	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
//
//	if (!reference)
//		return;
//
//	struct message_header_t* message = ref_getDataAndAcquire(reference);
//	if (!message)
//		return;
//
//	int32_t clientSocket = pbserver_findSocketFromThread(gFramework->rpcServer, curthread);
//	if (clientSocket < 0)
//	{
//		WriteLog(LL_Error, "could not open socket");
//		messagemanager_sendResponse(reference, -ESOCKTNOSUPPORT);
//		goto cleanup;
//	}
//
//	// Verify that our reference has enough space for our payload
//	if (ref_getSize(reference) < sizeof(struct filetransfer_getdents_t))
//	{
//		WriteLog(LL_Error, "not enough space to hold payload");
//		messagemanager_sendResponse(reference, -ENOMEM);
//		goto cleanup;
//	}
//
//	struct filetransfer_getdents_t* payload = message_getData(message);
//
//	WriteLog(LL_Debug, "[+] getdents %s", payload->path);
//
//	int handle = kopen(payload->path, 0x0000 | 0x00020000, 0);
//	if (handle < 0)
//	{
//		messagemanager_sendResponse(reference, handle);
//		WriteLog(LL_Error, "[-] could not open path %s", payload->path);
//		goto cleanup;
//	}
//
//	// Run through once to get the count
//	uint64_t dentCount = 0;
//	struct dirent* dent = 0;
//	const uint32_t bufferSize = 0x8000;
//	char* buffer = kmalloc(bufferSize);
//	if (!buffer)
//	{
//		messagemanager_sendResponse(reference, -ENOMEM);
//		WriteLog(LL_Error, "could not allocate memory");
//		goto cleanup;
//	}
//
//	// Zero out the buffer size
//	memset(buffer, 0, bufferSize);
//
//	// Get all of the directory entries the first time to get the count
//	while (kgetdents(handle, buffer, bufferSize) > 0)
//	{
//		dent = (struct dirent*)buffer;
//
//		while (dent->d_fileno)
//		{
//			if (dent->d_type == 0)
//				break;
//
//			dentCount++;
//			dent = (struct dirent*)((uint8_t*)dent + dent->d_reclen);
//		}
//	}
//
//	WriteLog(LL_Debug, "[+] closing handle");
//	kclose(handle);
//
//	// Re-open the handle
//	handle = kopen(payload->path, 0x0000 | 0x00020000, 0);
//	if (handle < 0)
//	{
//		kfree(buffer, bufferSize);
//		messagemanager_sendResponse(reference, handle);
//		WriteLog(LL_Error, "[-] could not open path %s", payload->path);
//		goto cleanup;
//	}
//
//	// Return success code
//	messagemanager_sendResponse(reference, 0);
//
//	// Send the dent count
//	kwrite(clientSocket, &dentCount, sizeof(dentCount));
//
//	struct filetransfer_getdents_t writeDent;
//	memset(&writeDent, 0, sizeof(writeDent));
//	memset(buffer, 0, bufferSize);
//
//	dent = 0;
//	while (kgetdents(handle, buffer, bufferSize) > 0)
//	{
//		dent = (struct dirent*)buffer;
//		while (dent->d_fileno)
//		{
//			//printf("[+] fileno %d\n", dent->d_fileno);
//
//			if (dent->d_type == 0)
//				break;
//
//			// Zero out the dent
//			memset(&writeDent, 0, sizeof(writeDent));
//
//			// Copy over the name, truncating it if need be
//			int nameLength = dent->d_namlen > 255 ? 255 : dent->d_namlen;
//
//			memcpy(writeDent.path, dent->d_name, nameLength);
//
//			writeDent.type = dent->d_type;
//			writeDent.handle = dent->d_fileno;
//
//			kwrite(clientSocket, &writeDent, sizeof(writeDent));
//
//			//printf("[+] writing dent %p\n", dent);
//			dent = (struct dirent*)((uint8_t*)dent + dent->d_reclen);
//		}
//	}
//
//	kfree(buffer, bufferSize);
//	kclose(handle);
//
//cleanup:
//	ref_release(reference);
}

void filetransfer_delete_callback(struct ref_t* reference)
{
//	if (!reference)
//		return;
//
//	struct message_header_t* message = ref_getDataAndAcquire(reference);
//	if (!message)
//		return;
//
//	int32_t clientSocket = pbserver_findSocketFromThread(gFramework->rpcServer, curthread);
//	if (clientSocket < 0)
//	{
//		WriteLog(LL_Error, "could not open socket");
//		messagemanager_sendResponse(reference, -ESOCKTNOSUPPORT);
//		goto cleanup;
//	}
//
//	// Verify that our reference has enough space for our payload
//	if (ref_getSize(reference) < sizeof(struct filetransfer_delete_t))
//	{
//		WriteLog(LL_Error, "not enough space to hold payload");
//		messagemanager_sendResponse(reference, -ENOMEM);
//		goto cleanup;
//	}
//
//	struct filetransfer_delete_t* deleteRequest = message_getData(message);
//
//	message->request = 0;
//	message->error_type = kunlink(deleteRequest->path);
//
//	 messagemanager_sendResponse(reference, 0);
//
//cleanup:
//	ref_release(reference);
}
