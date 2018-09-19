#pragma once
#include <oni/plugins/plugin.h>
#include <sys/param.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#define CHEAT_MAXTHREADS 8

enum cheat_thread_status_t
{
	// Thread is currently stopped
	CTS_Stopped = 0,

	// Thread is currently running
	CTS_Running,

	// Thread is currently finished
	CTS_Finished
};

struct cheat_thread_t
{
	// Plugin reference, DO NOT FREE
	struct cheat_plugin_t* plugin;

	// Thread object
	void* thread;

	// Current status of this thread
	enum cheat_thread_status_t status;

	// Start address
	uint64_t start;

	// End address
	uint64_t end;

	// Current offset in between start and end
	uint64_t current;

	// Block size to read at a time, default 0x4000
	uint64_t blockSize;

	// Alignment, default (4), can be 8, or 1 if you want to scan every byte (slow)
	uint64_t alignment;

	// Percentage completed (10, 20, 30, 50, etc)
	uint32_t percentage;

	// Lock on the thread structure
	struct mtx lock;
};

struct cheat_result_t
{
	// Address the result was found at
	uint64_t address;

	// Double linked list for easy tracking
	struct cheat_result_t* prev;

	struct cheat_result_t* next;
};

struct cheat_plugin_t
{
	struct plugin_t plugin;

	// Process
	int32_t processId;

	// Our threads
	struct cheat_thread_t* threads[CHEAT_MAXTHREADS];
	struct mtx threadsLock;

	// Our results
	struct cheat_result_t* results;
	struct mtx resultsLock;

	// Data we are scanning for
	uint8_t* data;

	// Length of the data we are scanning for
	uint64_t dataLength;
};

void cheat_plugin_init(struct cheat_plugin_t* plugin);
