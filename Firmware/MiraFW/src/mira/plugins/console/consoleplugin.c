#include "consoleplugin.h"
#include <oni/utils/memory/allocator.h>

#include <oni/messaging/message.h>
#include <oni/messaging/messagemanager.h>

#include <oni/utils/kdlsym.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/logger.h>

#include <mira/miraframework.h>

#include <machine/endian.h>
#include <sys/socket.h>
#include <netinet/in.h>

enum ConsoleCmds
{
	ConsoleCmd_Open = 0x2E8DE0C6,
	ConsoleCmd_Close = 0xB0377CD3
};

struct console_open_t
{
	// Input file descriptor that has already been opened
	int32_t fd;
	// Return port, ignored on request
	uint16_t port;
};

void consoleplugin_open_callback(struct allocation_t* ref);
void consoleplugin_close_callback(struct allocation_t* ref);
void consoleplugin_consoleThread(struct console_t* console);

void consoleplugin_init(struct consoleplugin_t* plugin)
{
	if (!plugin)
		return;

	plugin->plugin.name = "consoleplugin";
	plugin->plugin.description = "(empty)";
	plugin->plugin.plugin_load = (uint8_t(*)(void*)) consoleplugin_load;
	plugin->plugin.plugin_unload = (uint8_t(*)(void*)) consoleplugin_unload;

	void(*mtx_init)(struct mtx *m, const char *name, const char *type, int opts) = kdlsym(mtx_init);
	mtx_init(&plugin->mutex, "", NULL, 0);

	// Initialize all of the consoles
	for (uint32_t i = 0; i < ARRAYSIZE(plugin->consoles); ++i)
		plugin->consoles[i] = NULL;
}

int32_t consoleplugin_getFreeIndex(struct consoleplugin_t* plugin)
{
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);


	if (!plugin)
		return -1;

	int32_t freeIndex = -1;

	_mtx_lock_flags(&plugin->mutex, 0, __FILE__, __LINE__);

	for (uint32_t i = 0; i < ARRAYSIZE(plugin->consoles); ++i)
	{
		// Skip over all filled slots
		if (plugin->consoles[i])
			continue;

		freeIndex = i;
		break;
	}

	_mtx_unlock_flags(&plugin->mutex, 0, __FILE__, __LINE__);

	return freeIndex;
}

uint8_t consoleplugin_load(struct consoleplugin_t* plugin)
{
	if (!plugin)
		return false;

	messagemanager_registerCallback(mira_getFramework()->framework.messageManager, RPCCAT_LOG, ConsoleCmd_Open, consoleplugin_open_callback);
	messagemanager_registerCallback(mira_getFramework()->framework.messageManager, RPCCAT_LOG, ConsoleCmd_Close, consoleplugin_close_callback);
	
	return true;
}

uint8_t consoleplugin_unload(struct consoleplugin_t* plugin)
{
	if (!plugin)
		return false;

	messagemanager_unregisterCallback(mira_getFramework()->framework.messageManager, RPCCAT_LOG, ConsoleCmd_Open, consoleplugin_open_callback);
	messagemanager_unregisterCallback(mira_getFramework()->framework.messageManager, RPCCAT_LOG, ConsoleCmd_Close, consoleplugin_close_callback);

	return true;
}

struct console_t* consoleCreateConsole(struct consoleplugin_t* plugin, int32_t tty, int32_t socket)
{
	void * (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	if (!plugin)
		return NULL;

	// Allocate a new console object
	struct console_t* console = (struct console_t*)kmalloc(sizeof(struct console_t));
	if (!console)
		return NULL;

	// Zero out the allocated buffer
	memset(console, 0, sizeof(*console));

	// Assign the plugin reference
	console->plugin = plugin;
	
	// Set the descriptors
	console->ttyDescriptor = tty;
	console->socketDescriptor = socket;

	// NULL out thread
	console->consoleThread = NULL;

	// Currently the state is not set
	console->isRunning = false;

	return console;
}

void consoleDestroyConsole(struct console_t* console)
{
	if (!console)
		return;

	console->isRunning = false;

	// Kill the socket
	kshutdown(console->socketDescriptor, 2);
	kclose(console->socketDescriptor);
	console->socketDescriptor = -1;

	// Close the descriptor we opened previously
	kclose(console->socketDescriptor);
	console->socketDescriptor = -1;

	console->plugin = NULL;

	// Free the console object itself
	kfree(console, sizeof(*console));
}

void consoleplugin_open_callback(struct allocation_t* ref)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	int(*kthread_add)(void(*func)(void*), void* arg, struct proc* procptr, struct thread** tdptr, int flags, int pages, const char* fmt, ...) = kdlsym(kthread_add);
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);


	struct message_t* message = __get(ref);
	if (!message)
		return;

	if (message->header.request != true)
		goto cleanup;

	if (!message->payload)
		goto cleanup;

	struct console_open_t* request = (struct console_open_t*)message->payload;
	if (request->fd < 0)
	{
		request->port = -1;
		messagemanager_sendErrorMessage(mira_getFramework()->framework.messageManager, ref, EADDRNOTAVAIL);
		WriteLog(LL_Error, "invalid file descriptor");
		goto cleanup;
	}
	// TODO: Rewrite this to be less racey

	// Get the free index
	int32_t index = consoleplugin_getFreeIndex(mira_getFramework()->consolePlugin);
	if (index < 0)
	{
		WriteLog(LL_Error, "no free index");
		request->port = -1;
		messagemanager_sendErrorMessage(mira_getFramework()->framework.messageManager, ref, EADDRNOTAVAIL);
		goto cleanup;
	}
	
	// Select a port based on index
	uint16_t selectedPort = PORT_START + index;

	struct console_t* console = consoleCreateConsole(mira_getFramework()->consolePlugin, request->fd, -1/*We don't have a socket yet*/);
	if (!console)
	{
		WriteLog(LL_Error, "could not allocate console");
		request->port = -1;
		messagemanager_sendErrorMessage(mira_getFramework()->framework.messageManager, ref, ENOMEM);
		goto cleanup;
	}

	// Set up address
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = __bswap16(selectedPort);
	address.sin_len = sizeof(address);

	WriteLog(LL_Debug, "allocating new socket on port %u", selectedPort);

	console->socketDescriptor = ksocket(AF_INET, SOCK_STREAM, 0);
	if (console->socketDescriptor < 0)
	{
		
		WriteLog(LL_Error, "could not allocate listen socket");
		request->port = -1;
		messagemanager_sendErrorMessage(mira_getFramework()->framework.messageManager, ref, console->socketDescriptor);

		kfree(console, sizeof(*console));
		goto cleanup;
	}

	int32_t ret = kbind(console->socketDescriptor, (struct sockaddr*)&address, sizeof(address));
	if (ret < 0)
	{

		kshutdown(console->socketDescriptor, 2);
		kclose(console->socketDescriptor);
		console->socketDescriptor = -1;
		WriteLog(LL_Error, "could not bind to the socket (err: %d)", ret);

		kfree(console, sizeof(*console));
		goto cleanup;
	}

	ret = klisten(console->socketDescriptor, 1);
	if (ret < 0)
	{
		kshutdown(console->socketDescriptor, 2);
		kclose(console->socketDescriptor);
		console->socketDescriptor = -1;
		WriteLog(LL_Error, "could not listen to the socket (err: %d)", ret);

		kfree(console, sizeof(*console));
		goto cleanup;
	}

	ret = kthread_add((void(*)(void*))consoleplugin_consoleThread, console, mira_getProc(), (struct thread**)&console->consoleThread, 0, 0, "mconsole");
	if (ret != 0)
	{
		kshutdown(console->socketDescriptor, 2);
		kclose(console->socketDescriptor);
		console->socketDescriptor = -1;
		WriteLog(LL_Error, "could not start thread(err: %d)", ret);

		kfree(console, sizeof(*console));
		goto cleanup;
	}

	// Set the console for tracking for later
	_mtx_lock_flags(&mira_getFramework()->consolePlugin->mutex, 0, __FILE__, __LINE__);

	mira_getFramework()->consolePlugin->consoles[index] = console;

	_mtx_unlock_flags(&mira_getFramework()->consolePlugin->mutex, 0, __FILE__, __LINE__);

	// Set the port
	request->port = selectedPort;

	// Send the RPC message back to the client with the port
	messagemanager_sendSuccessMessage(mira_getFramework()->framework.messageManager, ref);

	// Send to socket if needed
	kwrite(message->socket, request, sizeof(*request));

cleanup:
	__dec(ref);
}

void consoleplugin_close_callback(struct allocation_t* ref)
{
	// TODO: Implement
}

void consoleplugin_consoleThread(struct console_t* console)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void(*kthread_exit)(void) = kdlsym(kthread_exit);

	if (!console)
		goto cleanup;

	struct sockaddr_in address;
	size_t clientAddressSize = sizeof(address);
	memset(&address, 0, sizeof(address));

	WriteLog(LL_Debug, "accepting clients on console");

	int32_t clientSocket = kaccept(console->socketDescriptor, (struct sockaddr*)&address, &clientAddressSize);
	if (clientSocket < 0)
	{
		WriteLog(LL_Error, "error accepting client (%d).", clientSocket);
		goto cleanup;
	}

	console->isRunning = true;

	char character;

	int32_t bytesRead = 0;

	while ((bytesRead = kread(console->ttyDescriptor, &character, sizeof(character))) > 0)
	{
		if (!console->isRunning)
			break;

		if (kwrite(clientSocket, &character, sizeof(character)) < 0)
			break;

		character = '\0';
	}

cleanup:
	if (console)
	{
		console->isRunning = false;
		kshutdown(console->socketDescriptor, 2);
		kclose(console->socketDescriptor);
		console->socketDescriptor = -1;
	}

	kthread_exit();
}