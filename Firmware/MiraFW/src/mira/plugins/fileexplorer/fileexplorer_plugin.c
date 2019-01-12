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

#include <mira/utils/flat_vector.h>
#include <mira/utils/vector.h>

#include <string.h>

#include <protobuf-c/mirabuiltin.pb-c.h>

#include "fileexplorer.pb-c.h"

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
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Open, fileexplorer_open_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Close, fileexplorer_close_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__GetDents, fileexplorer_getdents_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Write, fileexplorer_write_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Read, fileexplorer_read_callback);
	//messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__RmDir, fileexplorer_rmdir_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Stat, fileexplorer_stat_callback);
	messagemanager_registerCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Echo, fileexplorer_echo_callback);
	
	return true;
}

uint8_t fileexplorer_unload(struct fileexplorer_plugin_t* plugin)
{
	// Unregister all of the callbacks
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Open, fileexplorer_open_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Close, fileexplorer_close_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__GetDents, fileexplorer_getdents_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Write, fileexplorer_write_callback);;
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Read, fileexplorer_read_callback);
	//messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__RmDir, fileexplorer_rmdir_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Stat, fileexplorer_stat_callback);
	messagemanager_unregisterCallback(gFramework->messageManager, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Echo, fileexplorer_echo_callback);

	return true;
}

void fileexplorer_echo_callback(PbContainer* container)
{
	if (container == NULL || container->message == NULL)
		return;

	PB_DECODE(container, EchoRequest, echo_request, request);

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

void fileexplorer_open_callback(PbContainer* container)
{
	if (container == NULL || container->message == NULL)
		return;

	PB_DECODE(container, OpenRequest, open_request, request);

	WriteLog(LL_Debug, "open: p(%s) f(%d) m(%d)", request->path, request->flags, request->mode);

	struct thread_info_t threadInfo;
	oni_threadEscape(curthread, &threadInfo);

	int32_t ret = kopen(request->path, request->flags, request->mode);

	oni_threadRestore(curthread, &threadInfo);

	OpenResponse response = OPEN_RESPONSE__INIT;
	response.error = (ret < 0 ? ret : 0);
	response.handle = (ret < 0 ? -1 : ret);

	PB_ENCODE(container, response, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Open, open_response, responseContainer);

	// Send the response back and release this pbcontainer which should free
	messagemanager_sendResponse(responseContainer);
	pbcontainer_release(responseContainer);

	PB_RELEASE(container);
}

void fileexplorer_close_callback(PbContainer* container)
{ 
	if (container == NULL || container->message == NULL)
		return;

	PB_DECODE(container, CloseRequest, close_request, request);

	WriteLog(LL_Debug, "closing handle(%d).", request->handle);

	kclose(request->handle);

	CloseResponse response = CLOSE_RESPONSE__INIT;
	response.error = 0;

	PB_ENCODE(container, response, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Close, close_response, responseContainer);

	messagemanager_sendResponse(responseContainer);
	pbcontainer_release(responseContainer);

	PB_RELEASE(container);
}

void fileexplorer_read_callback(PbContainer* container)
{
	if (container == NULL || container->message == NULL)
		return;

	PB_DECODE(container, ReadRequest, read_request, request);

	//WriteLog(LL_Debug, "read h(%d) offset(%llx) size(%llx).", request->handle, request->offset, request->size);

	const uint32_t cMaxReadSize = PAGE_SIZE;
	if (request->size > cMaxReadSize)
	{
		WriteLog(LL_Error, "requested read size (%llx) > max (%llx).", request->size, cMaxReadSize);
		goto cleanup;
	}

	uint8_t* buffer = k_malloc(request->size);
	if (!buffer)
	{
		WriteLog(LL_Error, "could not allocate buffer for reading.");
		goto cleanup;
	}
	(void)memset(buffer, 0, request->size);

	ssize_t bytesRead = kread(request->handle, buffer, request->size);
	if (bytesRead != request->size)
		WriteLog(LL_Warn, "bytes read (%llx) != bytes requested (%llx).", bytesRead, request->size);

	ReadResponse response = READ_RESPONSE__INIT;
	response.error = (bytesRead < 0 ? bytesRead : 0);
	response.data.data = (bytesRead < 0 ? NULL : buffer);
	response.data.len = (bytesRead < 0 ? 0 : bytesRead);

	PB_ENCODE(container, response, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Read, read_response, responseContainer);

	messagemanager_sendResponse(responseContainer);
	pbcontainer_release(responseContainer);

	k_free(buffer);
	buffer = NULL;

	PB_RELEASE(container);
}

void fileexplorer_write_callback(PbContainer* container)
{
	if (container == NULL || container->message == NULL)
		return;

	PB_DECODE(container, WriteRequest, write_request, request);

	const uint32_t cMaxWriteSize = PAGE_SIZE;
	if (request->data.len > cMaxWriteSize)
	{
		WriteLog(LL_Error, "requested write size (%llx) > max allowed (%llx).", request->data.len, cMaxWriteSize);
		goto cleanup;
	}

	// Seek to the offset that we want to write
	(void)klseek(request->handle, request->offset, SEEK_SET);
	ssize_t bytesWritten = kwrite(request->handle, request->data.data, request->data.len);
	if (bytesWritten != request->data.len)
		WriteLog(LL_Warn, "wrote only (%llx) out of (%llx) bytes", bytesWritten, request->data.len);

	WriteResponse response = WRITE_RESPONSE__INIT;
	response.error = bytesWritten < 0 ? bytesWritten : 0;

	PB_ENCODE(container, response, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Write, write_response, responseContainer);

	messagemanager_sendResponse(responseContainer);
	pbcontainer_release(responseContainer);

	PB_RELEASE(container);
}


void fileexplorer_getdents_callback(PbContainer* container)
{ 
	if (container == NULL || container->message == NULL)
		return;

	PB_DECODE(container, GetDentsRequest, get_dents_request, request);

	WriteLog(LL_Debug, "getting dents for: (%s).", request->path);

	struct thread_info_t threadInfo;
	oni_threadEscape(curthread, &threadInfo);

	int32_t handle = kopen(request->path, O_RDONLY | O_DIRECTORY, 0);
	if (handle < 0)
	{
		WriteLog(LL_Error, "could not open the handle for path (%s).", request->path);
		goto cleanup;
	}

	oni_threadRestore(curthread, &threadInfo);

	const uint32_t cDentBufferSize = 0x8000;
	struct dirent* dent = NULL;
	uint8_t* dentData = k_malloc(cDentBufferSize);
	if (dentData == NULL)
	{
		WriteLog(LL_Error, "could not allocate dent space");
		goto cleanup;
	}
	(void)memset(dentData, 0, cDentBufferSize);

	uint64_t dentCount = 0;
	while (kgetdents(handle, (char*)dentData, cDentBufferSize) > 0)
	{
		dent = (struct dirent*)dentData;

		while (dent->d_fileno)
		{
			if (dent->d_type == DT_UNKNOWN)
				break;

			dentCount++;
			dent = (struct dirent*)((uint8_t*)dent + dent->d_reclen);
		}
	}

	//WriteLog(LL_Debug, "dentCount: %lld!!!!!!!!!!", dentCount);

	// This resets the gedents
	kclose(handle);
	handle = -1;

	// Allocate the dents
	Vector s_Vector;
	vector_initialize(&s_Vector, sizeof(DirEnt*));

	DirEnt** dents = k_malloc(dentCount * sizeof(DirEnt));
	if (dents == NULL)
	{
		WriteLog(LL_Error, "could not allocate dents.");
		goto cleanup_dents;
	}

	// Allocate all of the directory entries
	for (uint64_t i = 0; i < dentCount; ++i)
	{
		dents[i] = k_malloc(sizeof(DirEnt));
		if (dents[i] == NULL)
		{
			WriteLog(LL_Error, "could not allocate directory entry");
			goto cleanup_dents;
		}
	}

	// Re-open the path
	oni_threadEscape(curthread, &threadInfo);
	handle = kopen(request->path, O_RDONLY | O_DIRECTORY, 0);
	if (handle < 0)
	{
		WriteLog(LL_Error, "could not open the handle for path (%s).", request->path);
		goto cleanup_dents;
	}
	oni_threadRestore(curthread, &threadInfo);

	// Iterate again
	uint64_t currentDentIndex = 0;
	while (kgetdents(handle, (char*)dentData, cDentBufferSize) > 0)
	{
		dent = (struct dirent*)dentData;

		while (dent->d_fileno)
		{
			if (dent->d_type == DT_UNKNOWN)
				break;

			// This shouldn't happen ever
			if (dents[currentDentIndex] == NULL)
			{
				WriteLog(LL_Error, "could not get directory entry");
				goto next;
			}

			// Initialize the message
			dir_ent__init(dents[currentDentIndex]);

			// Assign the file number
			dents[currentDentIndex]->fileno = dent->d_fileno;
			
			// Set the name information
			dents[currentDentIndex]->name = k_malloc(dent->d_namlen);
			if (dents[currentDentIndex]->name == NULL)
				dents[currentDentIndex]->name = "_mira_error_invalid.bin";
			else
			{
				// Zero out the name buffer and copy it over
				(void)memset(dents[currentDentIndex]->name, 0, dent->d_namlen);
				(void)memcpy(dents[currentDentIndex]->name, dent->d_name, dent->d_namlen);
			}

			// Update the rest of the structure
			dents[currentDentIndex]->reclen = dent->d_reclen;
			dents[currentDentIndex]->type = dent->d_type;
		next:
			currentDentIndex++;
			dent = (struct dirent*)((uint8_t*)dent + dent->d_reclen);
		}
	}

	GetDentsResponse response = GET_DENTS_RESPONSE__INIT;
	get_dents_response__init(&response);

	response.n_entries = currentDentIndex;
	response.entries = dents;
	response.error = 0;

	//WriteLog(LL_Warn, "here: %p", get_dents_response__pack);

	
	PB_ENCODE(container, response, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__GetDents, get_dents_response, responseContainer);

	//WriteLog(LL_Warn, "here");

	messagemanager_sendResponse(responseContainer);
	pbcontainer_release(responseContainer);

	// Cleanup our allocation mess
cleanup_dents:
	if (dents)
	{
		// free all child objects
		for (uint64_t i = 0; i < dentCount; ++i)
		{
			if (dents[i] == NULL)
				continue;

			k_free(dents[i]);
			dents[i] = NULL;
		}

		k_free(dents);
		dents = NULL;
	}

	if (dentData)
	{
		k_free(dentData);
		dentData = NULL;
	}

	PB_RELEASE(container);
}

void fileexplorer_stat_callback(PbContainer* container)
{ 
	if (container == NULL || container->message == NULL)
		return;

	PB_DECODE(container, StatRequest, stat_request, request);

	struct stat stat;
	(void)memset(&stat, 0, sizeof(stat));

	struct thread_info_t threadInfo;
	oni_threadEscape(curthread, &threadInfo);

	int32_t ret = 0;
	if (request->path == NULL)
		ret = kfstat(request->handle, &stat);
	else
		ret = kstat(request->path, &stat);

	//WriteLog(LL_Debug, "%s returned %d", request->path == NULL ? "kfstat" : "kstat", ret);

	oni_threadRestore(curthread, &threadInfo);

	StatResponse response = STAT_RESPONSE__INIT;
	stat_response__init(&response);
	
	// atim
	response.atim = k_malloc(sizeof(TimeSpec));
	if (response.atim == NULL)
	{
		WriteLog(LL_Error, "could not allocate time struct");
		goto cleanup;
	}
	time_spec__init(response.atim);
	response.atim->nsec = stat.st_atim.tv_nsec;
	response.atim->sec = stat.st_atim.tv_sec;

	// TODO: Allocate
	response.birthtim = k_malloc(sizeof(TimeSpec));
	if (response.birthtim == NULL)
	{
		WriteLog(LL_Error, "could not allocate time struct");
		goto cleanup;
	}
	time_spec__init(response.birthtim);
	response.birthtim->nsec = stat.st_birthtim.tv_nsec;
	response.birthtim->sec = stat.st_birthtim.tv_sec;

	response.blksize = stat.st_blksize;
	response.blocks = stat.st_blocks;

	response.ctim = k_malloc(sizeof(TimeSpec));
	if (response.ctim == NULL)
	{
		WriteLog(LL_Error, "could not allocate time struct");
		goto cleanup;
	}
	time_spec__init(response.ctim);
	response.ctim->nsec = stat.st_ctim.tv_nsec;
	response.ctim->sec = stat.st_ctim.tv_sec;

	response.dev = stat.st_dev;
	response.flags = stat.st_flags;

	response.gen = stat.st_gen;
	response.gid = stat.st_gid;
	response.ino = stat.st_ino;
	response.lspare = stat.st_lspare;
	response.mode = stat.st_mode;
	response.mtim = k_malloc(sizeof(TimeSpec));
	if (response.mtim == NULL)
	{
		WriteLog(LL_Error, "could not allocate time struct");
		goto cleanup;
	}
	time_spec__init(response.mtim);
	response.mtim->nsec = stat.st_mtim.tv_nsec;
	response.mtim->sec = stat.st_mtim.tv_sec;

	response.nlink = stat.st_nlink;

	if (request->path == NULL)
		response.path = NULL;
	else
	{
		size_t pathLength = strlen(response.path) + 1;
		response.path = k_malloc(pathLength);
		if (response.path == NULL)
		{
			WriteLog(LL_Error, "could not allocate the path");
			goto cleanup;
		}

		(void)memset(response.path, 0, pathLength + 1);
		(void)memcpy(response.path, request->path, pathLength);
	}

	response.rdev = stat.st_rdev;
	response.size = stat.st_size;
	response.uid = stat.st_uid;


	PB_ENCODE(container, response, MESSAGE_CATEGORY__FILE, FILE_TRANSFER_COMMANDS__Stat, stat_response, responseContainer);

	messagemanager_sendResponse(responseContainer);
	pbcontainer_release(responseContainer);

	PB_RELEASE(container);
}
//
//void fileexplorer_mkdir_callback(PbContainer* container){ }
//
//void fileexplorer_rmdir_callback(PbContainer* container){ }
//
//void fileexplorer_unlink_callback(PbContainer* container){ }