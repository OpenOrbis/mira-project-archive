#include "gdbstub_plugin.h"

#include <sys/socket.h>
#include <oni/framework.h>
#include <oni/init/initparams.h>
#include <oni/utils/logger.h>
#include <oni/utils/sys_wrappers.h>

void gdbstub_plugin_init(struct gdbstub_plugin_t* plugin)
{
	if (plugin == NULL)
		return;

	// Zero out this plugin
	gdb_memset(plugin, 0, sizeof(*plugin));

	plugin->plugin.name = "GDB Stub";
	plugin->plugin.description = "Kernel Debugging Stub";

	plugin->plugin.plugin_load = (uint8_t(*)(void*)) gdbstub_load;
	plugin->plugin.plugin_unload = (uint8_t(*)(void*)) gdbstub_unload;

	// Set default values
	plugin->port = 2345;
}

uint8_t gdbstub_load(struct gdbstub_plugin_t* plugin)
{
	if (plugin == NULL)
		return false;

	return true;
}

uint8_t gdbstub_unload(struct gdbstub_plugin_t* plugin)
{
	if (plugin == NULL)
		return false;

	return true;
}

static int gdb_server_socket = -1;

int gdb_strlen(char* str)
{
	const char* s;
	for (s = str; *s; ++s) {}
	return (s - str);
}

int gdb_write(void* buff, int len)
{
	return kwrite(gdb_server_socket, buff, len);
}

void gdb_putDebugChar(unsigned char c)
{
	if (gdb_server_socket != -1)
		gdb_write(&c, 1);
}

int gdb_read(void* buff, int len)
{
	// If no connection is available, start server
	if (gdb_server_socket == -1)
	{
		gdb_server_socket = gdb_start_server(9099);
	}

	return kread(gdb_server_socket, buff, len);
}


unsigned char gdb_getDebugChar()
{
	unsigned char c = 0x00;

	// If no connection is available, start server
	if (gdb_server_socket == -1)
	{
		gdb_server_socket = gdb_start_server(2345);
	}

	while (gdb_read(&c, 1) != 1) {}

	return c;
}

void gdb_strcpy(char* dest, const char* src)
{
	size_t i;
	for (i = 0; src[i] != '\0'; i++)
		dest[i] = src[i];
	dest[i] = '\0';
}

void gdb_exceptionHandler(int exception_number, void* exception_address)
{
	WriteLog(LL_Info, "gdb_exceptionHandler Start\n");

	uint64_t funcAddrIU64 = (uint64_t)exception_address;
	uint8_t* funcAddress = (uint8_t*)& funcAddrIU64;

	// Read idrt
	uint8_t idtr[10];
	cpu_sidt((struct idt*) & idtr);

	// Find idt begin
	uint64_t* base = (uint64_t*)& idtr[2];

	WriteLog(LL_Info, "found idt at 0x%llx\n", *base);

	// Find correct entry
	uint8_t* entry = (uint8_t*)* base;
	entry += (0x10 * exception_number);

	WriteLog(LL_Info, "Entry 0x%llx at 0x%llx\n", exception_number, entry);

	// Replace function pointer
	cpu_disable_wp();
	entry[0] = funcAddress[0];
	entry[1] = funcAddress[1];
	entry[6] = funcAddress[2];
	entry[7] = funcAddress[3];
	entry[8] = funcAddress[4];
	entry[9] = funcAddress[5];
	entry[10] = funcAddress[6];
	entry[11] = funcAddress[7];
	cpu_enable_wp();

	WriteLog(LL_Info, "gdb_exceptionHandler End\n");
	return;
}

void gdb_memset(void* buff, uint8_t value, int len)
{
	uint8_t* tmp = (uint8_t*)buff;

	int i = 0;
	for (i = 0; i < len; i++)
		tmp[i] = value;

	return;
}

int gdb_start_server(int port)
{
	void* (*memset)(void* s, int c, size_t n) = kdlsym(memset);

	WriteLog(LL_Warn, "here");

	// Create a new socket
	int32_t serverSocket = ksocket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0)
	{
		WriteLog(LL_Error, "could not initialize socket (%d).", serverSocket);
		return serverSocket;
	}

	WriteLog(LL_Warn, "socket created: %d", serverSocket);

	// Set the server address information
	struct sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(port);
	serverAddress.sin_len = sizeof(serverAddress);

	WriteLog(LL_Warn, "filled out information");

	// Bind to the port
	int32_t ret = kbind(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress));
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not bind socket (%d).", ret);
		kclose(serverSocket);
		serverSocket = -1;
		return ret;
	}

	WriteLog(LL_Warn, "server socket bound: %d", serverSocket);

	ret = klisten(serverSocket, 1);
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not listen on socket (%d).", ret);
		kclose(serverSocket);
		serverSocket = -1;
		return ret;
	}

	// Wait for a client
	struct sockaddr_in clientAddress;
	int32_t clientSocket = -1;
	size_t addressSize = sizeof(clientAddress);
	clientAddress.sin_len = sizeof(clientAddress);
	clientSocket = kaccept(serverSocket, (struct sockaddr*) &clientAddress, &addressSize);

	// Check for any errors
	if (clientSocket < 0)
	{
		WriteLog(LL_Error, "could not accept socket (%d).", clientSocket);
		kclose(serverSocket);
		serverSocket = -1;
		return clientSocket;
	}
	uint32_t addr = (uint32_t)clientAddress.sin_addr.s_addr;

	WriteLog(LL_Debug, "got new gdb connection (%d) from IP %03d.%03d.%03d.%03d .", connection->socket,
		(addr & 0xFF),
		(addr >> 8) & 0xFF,
		(addr >> 16) & 0xFF,
		(addr >> 24) & 0xFF);

	kclose(serverSocket);
	serverSocket = -1;

	return clientSocket;
}
