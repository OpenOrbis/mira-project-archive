#pragma once
#define LOCK_PROFILING
#include <sys/param.h>
#include <oni/plugins/plugin.h>

#include <sys/mutex.h>
#include <machine/frame.h>
#include <machine/reg.h>

// Maximum allowed breakpoints
#define MAX_BREAKPOINTS	1024
#define MAX_SEGMENTS	1024
#define MAX_THREADS		1024

struct allocation_t;

struct segment_t
{
	// Address of this segment
	void* address;

	// Size of this segment
	uint64_t size;

	// Protection of this segment
	uint32_t protection;
};

struct thread_t
{
	// Address of the thread structure
	void* address;

	// Last error code
	int32_t errorno;

	//
	// Stack
	//

	// Stack address
	void* stack;

	// Stack size in pages, PAGE_SIZE = 0x4000
	int32_t stackSizeInPage;

	// Current trapframe
	struct trapframe frame;

	// CPU Mask 
	uint8_t cpuId;
	uint8_t padding[3];

	//
	// Credentials & Sandbox
	//

	int32_t effectiveUserId;
	int32_t realUserId;
	int32_t savedUserId;

	int32_t realGroupId;
	int32_t savedGroupId;

	void* prison;

	//
	// Thread specific stuff
	//
	uint64_t authId; // Orbis/Mantle/Raspbian specific

	int32_t debuggerFlags;
	int32_t debuggerChildPid;

	uint64_t currentSignalMask;
};

struct breakpoint_t
{
	// Address of the breakpoint (process .text base + offset)
	void* address;

	// Offset from text base
	uint64_t offset;

	// Breakpoint size
	uint8_t size;

	// Is this a hardware breakpoint?, if so then backup will be NULL and backupLength will be 0
	uint8_t hardware;

	// Backup data
	uint8_t* backup;

	// Length of backup bytes
	uint8_t backupLength;
};

struct debugger_plugin_t
{
	struct plugin_t plugin;

	// Registers
	struct reg registers;
	struct fpreg floatingRegisters;
	struct dbreg debugRegisters;

	// The currently attached process
	int32_t pid;

	// The breakpoints
	struct breakpoint_t breakpoints[MAX_BREAKPOINTS];

	// Segment reference inside of the process
	struct segment_t segments[MAX_SEGMENTS];

	// Thread reference inside the process;
	struct thread_t threads[MAX_THREADS];

	struct hook_t* trapFatalHook;

	// Debugger mutex, if anything is being done with the debugger LOCK THIS
	struct mtx lock;
};



enum debugger_state_t
{
	DebuggerRunning = 1,
	DebuggerPaused = 2,
	DebuggerDetached = 3,
};

uint8_t debugger_load(struct debugger_plugin_t* plugin);
uint8_t debugger_unload(struct debugger_plugin_t* plugin);

void debugger_plugin_init(struct debugger_plugin_t* plugin);

// Iterate through each of the breakpoints automatically updating the address + re-initializes the breakpoint with a new address
void debugger_updateBreakpoints(struct debugger_plugin_t* plugin);

// Updates all of the segments, removing or adding as it goes along
void debugger_updateSegments(struct debugger_plugin_t* plugin);

// Updates all of the threads, removing or adding as it goes along
void debugger_updateThreads(struct debugger_plugin_t* plugin);

void debugger_onTrapFatal(struct trapframe* frame, vm_offset_t eva);

// Returns true on success, false otherwise
uint8_t debugger_continue(struct debugger_plugin_t* plugin);

// Returns true on success, false otherwise
uint8_t debugger_pause(struct debugger_plugin_t* plugin);

uint8_t debugger_attach(struct debugger_plugin_t* plugin, int32_t pid);

uint8_t debugger_detach(struct debugger_plugin_t* plugin);

void debugger_update(struct debugger_plugin_t* plugin);


//
// Helper functions (do not lock)
//

// Returns true on success, false otherwise
uint8_t debugger_isAddressMapped(struct debugger_plugin_t* plugin, void* address);

// returns address on success, NULL otherwise
void* debugger_getTextAddress(struct debugger_plugin_t* plugin);

// Returns 0+ index on success, -1 on failure
int32_t debugger_addBreakpoint(struct debugger_plugin_t* plugin, void* address, uint8_t size, uint8_t hardware);

// Returns true on success, false otherwise
uint8_t debugger_removeBreakpoint(struct debugger_plugin_t* plugin, void* address);


// Returns 0+ index on success, -1 on failure
int32_t debugger_findFreeBreakpointIndex(struct debugger_plugin_t* plugin);


// Returns true on success, false otherwise
uint8_t debugger_clearAllBreakpoints(struct debugger_plugin_t* plugin);

int32_t debugger_getDisassemblyMinLength(struct debugger_plugin_t* plugin, void* address, size_t length);


//
// RPC callbacks
//
void debugger_getprocs_callback(struct allocation_t* ref);
void debugger_readmem_callback(struct allocation_t* ref);
void debugger_writemem_callback(struct allocation_t* ref);
void debugger_ptrace_callback(struct allocation_t* ref);
void debugger_kill_callback(struct allocation_t* ref);

// Segments
void debugger_getthreads_callback(struct allocation_t* ref);
void debugger_getsegments_callback(struct allocation_t* ref);

// Threads
void debugger_update_callback(struct allocation_t* ref);