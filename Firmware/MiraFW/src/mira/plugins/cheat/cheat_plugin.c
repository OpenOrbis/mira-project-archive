#include "cheat_plugin.h"
#ifndef LOCK_PROFILING
#define LOCK_PROFILING
#endif
#include <sys/types.h>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/proc.h>
#include <sys/kthread.h>
#include <sys/mutex.h>
#include <vm/vm.h>
#include <vm/vm_page.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>
#include <vm/vm_param.h>
#include <sys/imgact.h>

#include <oni/messaging/messagecategory.h>
#include <oni/messaging/message.h>
#include <oni/messaging/messagemanager.h>

#include <oni/utils/kdlsym.h>
#include <oni/utils/logger.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/kernel.h>

#include <oni/framework.h>

#define DEFAULT_BLOCK_SIZE	0x4000
#define DEFAULT_ALIGNMENT	0x4

#define CHEAT_MAXDATASIZE	0x40 /* 64 */

static struct cheat_plugin_t* cheatPlugin = NULL;

enum CheatCmds
{
	CheatCmd_StartScan = 0xB62AAC26,
	CheatCmd_GetResults = 0x63618ED8,
	CheatCmd_Status = 0xC9F2C761,
};

struct cheatplugin_startscan_t
{
	int32_t processId;
	uint8_t alignment;
	uint8_t data[CHEAT_MAXDATASIZE];
	uint16_t dataLength;
};

uint8_t cheatplugin_load(struct cheat_plugin_t* plugin);
uint8_t cheatplugin_unload(struct cheat_plugin_t* plugin);
int32_t cheatplugin_findFreeThread(struct cheat_plugin_t* plugin);

void cheatplugin_prepareScan(struct cheat_plugin_t* plugin, int pid, uint8_t* data, uint32_t dataLen);
uint64_t cheatplugin_getResultsCount(struct cheat_result_t* headResult);

void cheatplugin_onStartScan(struct allocation_t* msg);
void cheatplugin_onGetResults(struct allocation_t* msg);
void cheatplugin_onStatus(struct allocation_t* msg);

void cheat_plugin_init(struct cheat_plugin_t* plugin)
{
	plugin->plugin.description = "CheatPlugin";
	plugin->plugin.name = "CheatPlugin";
	plugin->plugin.plugin_load = (uint8_t(*)(void*)) cheatplugin_load;
	plugin->plugin.plugin_unload = (uint8_t(*)(void*)) cheatplugin_unload;

	void(*mtx_init)(struct mtx *m, const char *name, const char *type, int opts) = kdlsym(mtx_init);

	// Initialize our mutex's
	mtx_init(&plugin->resultsLock, "results_lock", NULL, 0);
	mtx_init(&plugin->threadsLock, "threads_lock", NULL, 0);

	for (uint32_t i = 0; i < ARRAYSIZE(plugin->threads); ++i)
		plugin->threads[i] = NULL;

	plugin->results = NULL;
	plugin->data = NULL;
	plugin->dataLength = 0;

	plugin->processId = -1;

	// This is some hack shit
	cheatPlugin = plugin;
}

uint8_t cheatplugin_load(struct cheat_plugin_t* plugin)
{
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_SYSTEM, CheatCmd_StartScan, cheatplugin_onStartScan);
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_SYSTEM, CheatCmd_GetResults, cheatplugin_onGetResults);
	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_SYSTEM, CheatCmd_Status, cheatplugin_onStatus);

	return true;
}

uint8_t cheatplugin_unload(struct cheat_plugin_t* plugin)
{
	messagemanager_unregisterCallback(gFramework->messageManager, RPCCAT_SYSTEM, CheatCmd_StartScan, cheatplugin_onStartScan);
	messagemanager_unregisterCallback(gFramework->messageManager, RPCCAT_SYSTEM, CheatCmd_GetResults, cheatplugin_onGetResults);
	messagemanager_unregisterCallback(gFramework->messageManager, RPCCAT_SYSTEM, CheatCmd_Status, cheatplugin_onStatus);

	return true;
}

void cheatplugin_onGetResults(struct allocation_t* msg)
{
	if (!cheatPlugin)
		return;

	if (!msg)
		return;

	struct message_t* message = __get(msg);
	if (!message)
		return;

	if (message->header.request != true)
		goto cleanup;

	if (!message->payload)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, msg, ENOMEM);
		WriteLog(LL_Error, "invalid payload");
		goto cleanup;
	}

cleanup:
	__dec(msg);
}

void cheatplugin_onStatus(struct allocation_t* msg)
{
	if (!cheatPlugin)
		return;

	if (!msg)
		return;

	struct message_t* message = __get(msg);
	if (!message)
		return;

	if (message->header.request != true)
		goto cleanup;

	if (!message->payload)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, msg, ENOMEM);
		WriteLog(LL_Error, "invalid payload");
		goto cleanup;
	}

cleanup:
	__dec(msg);
}

void cheatplugin_onStartScan(struct allocation_t* msg)
{
	if (!cheatPlugin)
		return;

	if (!msg)
		return;

	struct message_t* message = __get(msg);
	if (!message)
		return;

	if (message->header.request != true)
		goto cleanup;

	if (!message->payload)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, msg, ENOMEM);
		WriteLog(LL_Error, "invalid payload");
		goto cleanup;
	}

	struct cheatplugin_startscan_t* payload = (struct cheatplugin_startscan_t*)message->payload;
	if (payload->processId < 0)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, msg, EEXIST);
		WriteLog(LL_Error, "invalid process id");
		goto cleanup;
	}

	// Verify the alignment is one of the allowed values
	switch (payload->alignment)
	{
	case 1:
	case 2:
	case 4:
	case 8:
		break;
	default:
		messagemanager_sendErrorMessage(gFramework->messageManager, msg, EIO);
		WriteLog(LL_Error, "incompatible alignment, must be 1, 2, 4, or 8");
		goto cleanup;
	}

	// Verify the max data length
	if (payload->dataLength > CHEAT_MAXDATASIZE)
	{
		messagemanager_sendErrorMessage(gFramework->messageManager, msg, ERANGE);
		WriteLog(LL_Error, "incompatible data length (provided: %d, max: %d)", payload->dataLength, CHEAT_MAXDATASIZE);
		goto cleanup;
	}

	// Launch the scan
	cheatplugin_prepareScan(cheatPlugin, payload->processId, payload->data, payload->dataLength);

	// Send a successful response back
	messagemanager_sendSuccessMessage(gFramework->messageManager, msg);

cleanup:
	__dec(msg);
}

int32_t cheatplugin_findFreeThread(struct cheat_plugin_t* plugin)
/*
Finds a free index

Returns:
>= 0 on success, -1 on error
*/
{
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	if (!plugin)
		return -1;

	// By default we will have -1 for error
	int freeIndex = -1;

	_mtx_lock_flags(&plugin->threadsLock, 0, __FILE__, __LINE__);
	for (uint32_t i = 0; i < ARRAYSIZE(plugin->threads); ++i)
	{
		if (plugin->threads[i] == NULL)
		{
			freeIndex = i;
			break;
		}
	}
	_mtx_unlock_flags(&plugin->threadsLock, 0, __FILE__, __LINE__);

	return freeIndex;
}

// Callback
void cheatplugin_onThreadCompleted(struct cheat_plugin_t* plugin)
/*
This callback gets fired every time a thread completes,
this will be responsible for checking each of the threads to ensure they are all finished

Then after that, will proceed with results counting.
*/
{
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);


	// Verify our plugin object is valid
	if (!plugin)
		return;

	// Write that we got a successful completion
	WriteLog(LL_Info, "CHEAT PLUGIN THREAD COMPLETE");

	// Iterate all of the threads, checking for finished state
	uint8_t threadStillRunning = false;
	_mtx_lock_flags(&plugin->threadsLock, 0, __FILE__, __LINE__);
	for (uint32_t i = 0; i < ARRAYSIZE(plugin->threads); ++i)
	{
		struct cheat_thread_t* thread = plugin->threads[i];
		if (!thread)
			continue;

		// If a thread is not in the finished state, then ignore
		if (thread->status == CTS_Running)
		{
			threadStillRunning = true;
			break;
		}
	}
	_mtx_unlock_flags(&plugin->threadsLock, 0, __FILE__, __LINE__);

	// Thread is still running
	if (threadStillRunning)
		return;

	// Once all threads are finished, we can count the results
	_mtx_lock_flags(&plugin->resultsLock, 0, __FILE__, __LINE__);
	uint64_t resultsCount = cheatplugin_getResultsCount(plugin->results);
	_mtx_unlock_flags(&plugin->resultsLock, 0, __FILE__, __LINE__);
	WriteLog(LL_Info, "SCAN FINISHED: %lld RESULTS!", resultsCount);

	// Resume the previously frozen process
	kkill(plugin->processId, SIGCONT);
}

struct cheat_result_t* cheatplugin_getLastResult(struct cheat_result_t* result)
{
	if (!result)
		return NULL;

	struct cheat_result_t* currentResult = result;
	while (currentResult->next != NULL)
		currentResult = currentResult->next;

	return currentResult;
}

struct cheat_result_t* cheatplugin_getFirstResult(struct cheat_result_t* result)
{
	if (!result)
		return NULL;

	struct cheat_result_t* currentResult = result;
	while (currentResult->prev != NULL)
		currentResult = currentResult->prev;

	return currentResult;
}

uint64_t cheatplugin_getResultsCount(struct cheat_result_t* headResult)
{
	if (!headResult)
		return 0;

	struct cheat_result_t* currentResult = headResult;
	uint64_t resultCount = 1;
	while (currentResult->next != NULL)
	{
		resultCount++;
		currentResult = currentResult->next;
	}

	return resultCount;
}

void cheatplugin_clearResults(struct cheat_plugin_t* plugin)
{
	if (!plugin)
		return;

	if (!plugin->results)
		return;

	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);


	_mtx_lock_flags(&plugin->resultsLock, 0, __FILE__, __LINE__);

	struct cheat_result_t* currentResult = cheatplugin_getLastResult(plugin->results);
	if (currentResult)
	{
		while (currentResult->prev != NULL)
		{
			// Save the object we are about to free
			struct cheat_result_t* aboutToFree = currentResult;

			// Assign our new current result
			currentResult = currentResult->prev;

			// Set the next pointer (the one we are about to free) to NULL
			currentResult->next = NULL;


			// Free the object
			kfree(aboutToFree, sizeof(*aboutToFree));

			// Cleanup
			aboutToFree = NULL;
		}

		// Delete the last reference
		plugin->results = NULL;

		// Free the final object
		kfree(currentResult, sizeof(*currentResult));
	}
	_mtx_unlock_flags(&plugin->resultsLock, 0, __FILE__, __LINE__);
}

void cheat_thread_init(struct cheat_plugin_t* plugin, struct cheat_thread_t* thread, uint64_t startAddress, uint64_t endAddress)
{
	if (!thread || !plugin)
		return;

	thread->plugin = plugin;
	thread->thread = NULL;
	thread->status = CTS_Stopped;
	thread->start = startAddress;
	thread->end = endAddress;
	thread->alignment = DEFAULT_ALIGNMENT;
	thread->blockSize = DEFAULT_BLOCK_SIZE;
	thread->percentage = 0;

	void(*mtx_init)(struct mtx *m, const char *name, const char *type, int opts) = kdlsym(mtx_init);
	mtx_init(&thread->lock, "scan_lock", NULL, 0);
}

void cheatplugin_addResult(struct cheat_plugin_t* plugin, uint64_t address)
{
	if (!plugin)
		return;

	if (address == NULL)
		return;

	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	_mtx_lock_flags(&plugin->resultsLock, 0, __FILE__, __LINE__);

	// If there is no head create one
	if (plugin->results == NULL)
	{
		struct cheat_result_t* result = kmalloc(sizeof(struct cheat_result_t));
		if (!result)
		{
			WriteLog(LL_Error, "Could not allocate new result.");
			goto end;
		}

		// Assign the address, HEAD prev is null
		result->address = address;
		result->prev = NULL;
		result->next = NULL;

		// Assign the head
		plugin->results = result;
	}
	else
	{
		struct cheat_result_t* lastResult = cheatplugin_getLastResult(plugin->results);

		struct cheat_result_t* result = kmalloc(sizeof(struct cheat_result_t));
		if (!result)
		{
			WriteLog(LL_Error, "Could not allocate new result.");
			goto end;
		}

		// Assign the address, HEAD prev is null
		result->address = address;
		result->prev = lastResult;
		result->next = NULL;

		// Add to our list
		lastResult->next = result;
	}
end:
	_mtx_unlock_flags(&plugin->resultsLock, 0, __FILE__, __LINE__);
}

void cheat_thread_scanThread(void* threadData)
/*
This is the kthread_add entry point, where we will be able to actually scan the thread
At this point, the process should be stopped with SIGSTOP already, otherwise you may be boned.

threadData - a cheat_thread_t structure pointer
*/
{
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	struct  proc* (*pfind)(pid_t) = kdlsym(pfind);
	void(*kthread_exit)(void) = kdlsym(kthread_exit);

	WriteLog(LL_Info, "Entered scan thread");

	if (!threadData)
	{
		WriteLog(LL_Error, "Could not start scanThread, no thread data passed!");
		kthread_exit();
		return;
	}

	struct cheat_thread_t* thread = (struct cheat_thread_t*)threadData;

	if (!thread)
	{
		WriteLog(LL_Error, "invalid thread");
		kthread_exit();
		return;
	}

	if (!thread->plugin)
	{
		WriteLog(LL_Error, "invalid plugin");
		kthread_exit();
		return;
	}

	if (!thread->plugin->data)
	{
		WriteLog(LL_Error, "invalid data");
		kthread_exit();
		return;
	}

	if (thread->start == NULL || thread->end == NULL)
	{
		WriteLog(LL_Error, "Invalid start/end parameters");
		kthread_exit();
		return;
	}

	if (thread->status != CTS_Stopped)
	{
		WriteLog(LL_Error, "Tried to start a running or finished thread.");
		kthread_exit();
		return;
	}

	// Set the block size if it is not set
	if (thread->blockSize == 0)
		thread->blockSize = DEFAULT_BLOCK_SIZE;

	// Set the alignment if it not set
	if (thread->alignment == 0)
		thread->alignment = DEFAULT_ALIGNMENT;

	int(*memcmp)(const void* ptr1, const void* ptr2, size_t num) = kdlsym(memcmp);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	// Get the total size
	uint64_t totalSize = thread->end - thread->start;
	uint64_t blockSize = thread->blockSize;

	uint64_t blockCount = totalSize / blockSize;
	uint64_t leftover = totalSize % blockSize;

	// Get the data from the plugin
	uint8_t* data = thread->plugin->data;
	uint64_t dataLength = thread->plugin->dataLength;

	uint8_t* buffer = (uint8_t*)kmalloc(blockSize);
	if (!buffer)
	{
		WriteLog(LL_Error, "could not allocate buffer");
		kthread_exit();
		return;
	}

	// Lock our thread
	_mtx_lock_flags(&thread->lock, 0, __FILE__, __LINE__);

	// We are starting off
	thread->percentage = 0;

	WriteLog(LL_Info, "Preparing to scan.");
	WriteLog(LL_Info, "Data: %p DataLength: %d", data, dataLength);
	WriteLog(LL_Info, "Alignment: %d", thread->alignment);

	// Iterate through the entire memory range
	for (uint64_t currentBlock = 0; currentBlock < blockCount; ++currentBlock)
	{
		// Get the start of the block
		uint64_t blockStart = (thread->start + (currentBlock * blockSize));

		// Zero out our temporary buffer
		memset(buffer, 0, blockSize);

		size_t readLength = 0;
		struct proc* process = pfind(thread->plugin->processId);
		if (process == NULL)
		{
			WriteLog(LL_Error, "DSADASDASDA SHITS FUCKKKKKKKKKKKEEEEDDD");
			continue;
		}

		int result = proc_rw_mem(process, (void*)blockStart, blockSize, buffer, &readLength, 0);
		_mtx_unlock_flags(&process->p_mtx, 0, __FILE__, __LINE__);

		if (result < 0)
			WriteLog(LL_Warn, "could not proc_rw_mem %d", result);

		// Scan inside the block
		for (uint64_t blockPosition = 0; blockPosition < blockSize; blockPosition += thread->alignment)
		{
			// Current position
			uint8_t* blockPositionData = buffer + blockPosition;

			// Don't go past the bounds of the block
			if (blockPosition + dataLength > blockSize)
				continue;

			int compareResult = memcmp(blockPositionData, data, dataLength);
			if (compareResult == 0)
				cheatplugin_addResult(thread->plugin, (uint64_t)blockPositionData);
		}
	}

	uint8_t* leftoverStart = (uint8_t*)(thread->start + (blockCount * blockSize));

	// Zero out our temporary buffer
	memset(buffer, 0, blockSize);

	size_t readLength = 0;
	struct proc* process = pfind(thread->plugin->processId);
	if (process == NULL)
	{
		WriteLog(LL_Error, "DSADASDASDA SHITS FUCKKKKKKKKKKKEEEEDDD22222222222222222");
	}

	int result = proc_rw_mem(process, leftoverStart, leftover, buffer, &readLength, 0);
	_mtx_unlock_flags(&process->p_mtx, 0, __FILE__, __LINE__);

	if (result < 0)
		WriteLog(LL_Warn, "could not proc_rw_mem2 %d", result);

	for (uint64_t leftoverPosition = 0; leftoverPosition < leftover; leftoverPosition += thread->alignment)
	{
		// Current position
		uint8_t* leftoverPositionData = buffer + leftoverPosition;

		// Don't go past the bounds of the block
		if (leftoverPosition + dataLength > leftover)
			continue;

		int compareResult = memcmp(leftoverPositionData, data, dataLength);
		if (compareResult == 0)
			cheatplugin_addResult(thread->plugin, (uint64_t)leftoverPositionData);
	}

	// free our buffer
	kfree(buffer, blockSize);

	// Update the status
	thread->status = CTS_Finished;

	WriteLog(LL_Info, "Thread finished.");

	// Unlock the thread
	_mtx_unlock_flags(&thread->lock, 0, __FILE__, __LINE__);

	// Fire the scan thread completion callback
	cheatplugin_onThreadCompleted(thread->plugin);

	// Exit this thread gracefully
	kthread_exit();
}

void cheatplugin_prepareScan(struct cheat_plugin_t* plugin, int pid, uint8_t* data, uint32_t dataLen)
{
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);
	void(*vmspace_free)(struct vmspace *) = kdlsym(vmspace_free);
	struct vmspace* (*vmspace_acquire_ref)(struct proc *) = kdlsym(vmspace_acquire_ref);
	void(*_vm_map_lock_read)(vm_map_t map, const char *file, int line) = kdlsym(_vm_map_lock_read);
	void(*_vm_map_unlock_read)(vm_map_t map, const char *file, int line) = kdlsym(_vm_map_unlock_read);
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	int(*kthread_add)(void(*func)(void*), void* arg, struct proc* procptr, struct thread** tdptr, int flags, int pages, const char* fmt, ...) = kdlsym(kthread_add);

	// Process id
	WriteLog(LL_Info, "Preparing to scan pid %d data %p dataLength %d.", pid, data, dataLen);

	if (!plugin)
		return;

	if (!data || !dataLen)
	{
		WriteLog(LL_Error, "Invalid data or data length (%p) (%x).", data, dataLen);
		return;
	}

	if (pid < 0)
	{
		WriteLog(LL_Error, "Invalid process id.");
		return;
	}

	// Allocate the new data
	uint8_t* allocData = kmalloc(dataLen);
	if (!allocData)
	{
		WriteLog(LL_Error, "Could not allocate scan buffer");
		return;
	}

	// Zero out our data buffer
	memset(allocData, 0, dataLen);

	// Copy the data to our buffer
	memcpy(allocData, data, dataLen);

	// Assign the data we are searching for
	plugin->data = allocData;
	plugin->dataLength = dataLen;
	plugin->processId = pid;

	// TODO: Stop all other scanning threads

	// Free all previous threads
	for (uint32_t i = 0; i < ARRAYSIZE(plugin->threads); ++i)
	{
		struct cheat_thread_t* thread = plugin->threads[i];

		if (thread == NULL)
			continue;

		// TODO: Stop the thread forefully if it's not
		if (thread->status == CTS_Running)
		{
			WriteLog(LL_Info, "Force killing running thread (%i).", i);
		}

		_mtx_lock_flags(&plugin->threadsLock, 0, __FILE__, __LINE__);

		// NULL out the thread entry
		plugin->threads[i] = NULL;

		// Free the thread structure that was previously in our slot
		kfree(thread, sizeof(*thread));

		_mtx_unlock_flags(&plugin->threadsLock, 0, __FILE__, __LINE__);
	}

	// Free the results list
	cheatplugin_clearResults(plugin);

	// Get the process from a pid
	struct  proc* (*pfind)(pid_t) = kdlsym(pfind);
	struct proc* process = pfind(pid);
	if (process == NULL)
	{
		WriteLog(LL_Error, "Process not found (pid: %d).", pid);
		return;
	}

	// Freeze the process
	kkill(pid, SIGSTOP);

	// Get the vm map
	struct vmspace* vm = vmspace_acquire_ref(process);
	vm_map_t map = &process->p_vmspace->vm_map;
	_vm_map_lock_read(map, __FILE__, __LINE__);

	struct vm_map_entry* entry = map->header.next;

	for (uint32_t i = 0; i < map->nentries; ++i)
	{
		// For each section that we have read perms on (so we don't fault)
		if (entry->protection & VM_PROT_READ)
		{
			// Find a new free thread index
			int freeIndex = cheatplugin_findFreeThread(plugin);
			if (freeIndex == -1)
			{
				WriteLog(LL_Error, "could not find free thread index.");
				goto cont;
			}

			struct cheat_thread_t* thread = kmalloc(sizeof(struct cheat_thread_t));
			if (!thread)
				goto cont;

			// Initialize a thread to be started
			cheat_thread_init(plugin, thread, entry->start, entry->end);

			// Assign the thread for tracking
			_mtx_lock_flags(&plugin->threadsLock, 0, __FILE__, __LINE__);
			plugin->threads[freeIndex] = thread;
			_mtx_unlock_flags(&plugin->threadsLock, 0, __FILE__, __LINE__);

			WriteLog(LL_Error, "Starting thread %p - %p : idx: %d", entry->start, entry->end, freeIndex);

			// Fire off the thread start
			int createResult = kthread_add(cheat_thread_scanThread, thread, curthread->td_proc, (struct thread**)&thread->thread, 0, 0, "oniscan");
			if (createResult != 0)
			{
				WriteLog(LL_Error, "could not create scanning thread.");
				goto cont;
			}

		cont:
			if (!(entry = entry->next))
				break;
		}
	}
	// Free the vmmap
	_vm_map_unlock_read(map, __FILE__, __LINE__);
	vmspace_free(vm);

	// You need to unlock the process, or the kernel will assert and hang
	_mtx_unlock_flags(&process->p_mtx, 0, __FILE__, __LINE__);

	WriteLog(LL_Info, "All cheat threads started.");
}