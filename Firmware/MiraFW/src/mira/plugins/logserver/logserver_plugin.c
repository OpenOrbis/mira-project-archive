#include "logserver_plugin.h"
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/log/logger.h>
#include <oni/framework.h>

#include <sys/socket.h>
#include <sys/proc.h>
#include <sys/fcntl.h>
#include <sys/endian.h>

uint8_t logserver_load(struct logserver_plugin_t* plugin);
uint8_t logserver_unload(struct logserver_plugin_t* plugin);

void logserver_init(struct logserver_plugin_t* plugin)
{
	plugin->plugin.name = "LogServer";
	plugin->plugin.description = "(empty)";
	plugin->plugin.plugin_load = logserver_load;
	plugin->plugin.plugin_unload = logserver_unload;

	plugin->socket = -1;
	plugin->port = 9998;
	plugin->thread = NULL;
}

uint8_t logserver_load(struct logserver_plugin_t* plugin)
{
	if (!plugin)
		return false;

	// Create a new socket
	plugin->socket = ksocket(AF_INET, SOCK_STREAM, 0);
	if (plugin->socket == -1)
	{
		WriteLog(LL_Error, "could not create listen socket");
		return false;
	}

	// Set up address
	kmemset(&plugin->address, 0, sizeof(plugin->address));
	plugin->address.sin_family = AF_INET;
	plugin->address.sin_addr.s_addr = INADDR_ANY;
	plugin->address.sin_port = __bswap16(plugin->port);
	plugin->address.sin_len = sizeof(plugin->address);

	// Bind to port
	int ret = kbind(plugin->socket, (struct sockaddr*)&plugin->address, sizeof(plugin->address));
	if (ret < 0)
	{
		kshutdown(plugin->socket, 2);
		kclose(plugin->socket);
		plugin->socket = -1;

		WriteLog(LL_Error, "could not bind to socket %d", ret);
		return false;
	}

	// Listen for clients
	if (klisten(plugin->socket, 3) == -1)
	{
		kshutdown(plugin->socket, 2);
		kclose(plugin->socket);
		plugin->socket = -1;

		WriteLog(LL_Error, "could not listen to the socket.");
		return false;
	}

	utilUSleep(100, "");

	int creationResult = kthread_add(logserver_serverThread, plugin, curthread->td_proc, (struct thread**)&plugin->thread, 0, 0, "logserver");
	if (creationResult != 0)
		return 0;

	WriteLog(LL_Debug, "logserver thread started.");

	return true;
}

uint8_t logserver_unload(struct logserver_plugin_t* plugin)
{
	if (!plugin)
		return false;


	plugin->isRunning = false;
	return true;
}

void logserver_serverThread(void* data)
{
	if (!data)
	{
		kthread_exit();
		return;
	}

	WriteLog(LL_Info, "Entered Log Thread");

	struct logserver_plugin_t* plugin = (struct logserver_plugin_t*)data;
	plugin->isRunning = true;

	struct sockaddr_in address;
	size_t clientAddressSize = sizeof(address);
	kmemset(&address, 0, sizeof(address));

	int32_t socket = -1;

	char buffer[2];

	WriteLog(LL_Info, "Accepting klog clients");
	while ((socket = kaccept(plugin->socket, (struct sockaddr*)&address, &clientAddressSize)) >= 0)
	{
		if (!plugin->isRunning)
			break;

		WriteLog(LL_Info, "New logclient connected");

		if (socket == -1)
			continue;

		WriteLog(LL_Info, "Opening klog");
		int32_t klog = kopen("/dev/klog", O_RDONLY, 0);
		if (klog < 0)
		{
			WriteLog(LL_Error, "could not open klog for reading (%d).", klog);
			break;
		}

		int32_t bytesRead = 0;
		
		while ((bytesRead = kread(klog, buffer, 1)) > 0)
		{
			if (!plugin->isRunning)
				break;

			if (kwrite(socket, buffer, 1) < 0)
				break;

			kmemset(buffer, 0, sizeof(buffer));
		}

		kclose(klog);

		kshutdown(socket, 2);
		kclose(socket);
		socket = -1;
	}

	WriteLog(LL_Info, "shutting down thread");
	plugin->isRunning = false;

	kshutdown(plugin->socket, 2);
	kclose(plugin->socket);
	plugin->socket = -1;

	kthread_exit();
}