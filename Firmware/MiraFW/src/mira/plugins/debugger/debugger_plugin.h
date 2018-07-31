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

	// The currently attached process
	struct proc* process;

	// The breakpoints
	struct breakpoint_t breakpoints[MAX_BREAKPOINTS];

	// Segment reference inside of the process
	struct segment_t segments[MAX_SEGMENTS];

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




// Returns true on success, false otherwise
uint8_t debugger_continue(struct debugger_plugin_t* plugin);

// Returns true on success, false otherwise
uint8_t debugger_pause(struct debugger_plugin_t* plugin);

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