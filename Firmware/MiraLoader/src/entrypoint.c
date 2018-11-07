#include <util/types.h>
#include <util/syscall.h>

#include <loader/elfloader.h>

#include <util/initparams.h>

#include <sce/dynlib.h>

#include <sys/elf64.h>
#include <sys/socket.h>
#include <netinet/in.h>

//uint8_t buffer[0x4000];

int(*sceSysUtilSendSystemNotificationWithText)(int messageType, char* message) = NULL;
int(*sceKernelLoadStartModule)(const char *name, size_t argc, const void *argv, unsigned int flags, int, int) = NULL;
int(*sceNetSocket)(const char *, int, int, int) = NULL;
int(*sceNetSocketClose)(int) = NULL;
int(*sceNetConnect)(int, struct sockaddr *, int) = NULL;
int(*sceNetSend)(int, const void *, size_t, int) = NULL;
int(*sceNetBind)(int, struct sockaddr *, int) = NULL;
int(*sceNetListen)(int, int) = NULL;
int(*sceNetAccept)(int, struct sockaddr *, unsigned int *) = NULL;
int(*sceNetRecv)(int, void *, size_t, int) = NULL;
int(*sceNetSocketAbort)(int, int) = NULL;

struct mdbg_service_arg {
	uint32_t unknown0;
	uint32_t padding4;
	void* unknown8;
	void* unknown10;
	void* unknown18;
	void* unknown20;
};

void mdbg_service(uint32_t cmd, void* arg2, void* arg3)
{
	syscall3(601, (void*)(uint64_t)cmd, arg2, arg3);
}

void writelog(char* msg)
{
	struct mdbg_service_arg arg = 
	{
		0,		// unknown0
		0,		// padding4
		msg,	// unknown8
		0,		// unknown10
		0,		// unknown18
		0		// unknown20
	};

	mdbg_service(0x7, &arg, NULL);
}

void* mira_entry(void* args)
{
	// Prompt the user
	int moduleId = -1;
	sys_dynlib_load_prx("/system/common/lib/libSceSysUtil.sprx", &moduleId);

	int(*sceSysUtilSendSystemNotificationWithText)(int messageType, char* message) = NULL;

	sys_dynlib_dlsym(moduleId, "sceSysUtilSendSystemNotificationWithText", &sceSysUtilSendSystemNotificationWithText);

	if (sceSysUtilSendSystemNotificationWithText)
	{
		char* initMessage = "Mira Project Loaded\nRPC Server Port: 9999\nkLog Server Port: 9998\n";
		sceSysUtilSendSystemNotificationWithText(222, initMessage);
	}

	sys_dynlib_unload_prx(moduleId);

	return 0;
	//writelog("test");

	//int32_t sysUtilModuleId = -1;
	//int32_t netModuleId = -1;
	//int32_t libKernelWebModuleId = -1;

	//// libkernel
	//{
	//	sys_dynlib_load_prx("libkernel.sprx", &libKernelWebModuleId);

	//	sys_dynlib_dlsym(libKernelWebModuleId, "sceKernelLoadStartModule", &sceKernelLoadStartModule);
	//}

	//// initialize system notifications
	//{
	//	sys_dynlib_load_prx("/system/common/lib/libSceSysUtil.sprx", &sysUtilModuleId);

	//	sys_dynlib_dlsym(sysUtilModuleId, "sceSysUtilSendSystemNotificationWithText", &sceSysUtilSendSystemNotificationWithText);
	//}

	//// 
	//{
	//	sys_dynlib_load_prx("/system/common/lib/libSceSysUtil.sprx", &libKernelWebModuleId);

	//	sys_dynlib_dlsym(libKernelWebModuleId, "open", &sceSysUtilSendSystemNotificationWithText);
	//}

	//// Networking resolving
	//{
	//	sys_dynlib_load_prx("libSceNet.sprx", &netModuleId);

	//	sys_dynlib_dlsym(netModuleId, "sceNetSocket", &sceNetSocket);
	//	sys_dynlib_dlsym(netModuleId, "sceNetSocketClose", &sceNetSocketClose);
	//	sys_dynlib_dlsym(netModuleId, "sceNetBind", &sceNetBind);
	//	sys_dynlib_dlsym(netModuleId, "sceNetListen", &sceNetListen);
	//	sys_dynlib_dlsym(netModuleId, "sceNetAccept", &sceNetAccept);
	//	sys_dynlib_dlsym(netModuleId, "sceNetRecv", &sceNetRecv);
	//}

	//// Initialize
	//struct sockaddr_in serverAddress;
	//serverAddress.sin_len = sizeof(serverAddress);
	//serverAddress.sin_addr.s_addr = INADDR_ANY;
	//serverAddress.sin_port = __bswap16(9021); // port 9020

	//for (size_t i = 0; i < ARRAYSIZE(serverAddress.sin_zero); ++i)
	//	serverAddress.sin_zero[i] = 0;

	//// Create a new socket
	//int32_t serverSocket = sceNetSocket("loader", AF_INET, SOCK_STREAM, 0);
	//if (serverSocket < 0)
	//{
	//	writelog("socket error\n");
	//	return 0;
	//}

	//// Bind to localhost
	//int32_t result = sceNetBind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	//if (result < 0)
	//{
	//	writelog("bind error\n");
	//	return 0;
	//}

	//// Listen
	//result = sceNetListen(serverSocket, 10);

	//// Wait for a client to send something
	//int32_t clientSocket = sceNetAccept(serverSocket, NULL, NULL);
	//if (clientSocket < 0)
	//{
	//	writelog("accept errror\n");
	//	return 0;
	//}

	//size_t currentSize = 0;
	//size_t recvSize = 0;

	//// Recv one byte at a time until we get our buffer
	//while ((recvSize = sceNetRecv(clientSocket, buffer + currentSize, sizeof(uint8_t), 0) > 0))
	//{
	//	currentSize += recvSize;

	//	if (sizeof(buffer) - 1 >= currentSize)
	//		break;
	//}

	//// Close the client and server socket connections
	//sceNetSocketClose(clientSocket);
	//sceNetSocketClose(serverSocket);

	//clientSocket = -1;
	//serverSocket = -1;

	//uint32_t magic = *(uint32_t*)buffer;
	//if (magic == (uint32_t)1179403647) // 0x7F 'ELF'
	//{
	//	// Launch ELF
	//	writelog("launching elf\n");

	//	// TODO: Check/Add a flag to the elf that determines if this is a kernel or userland elf
	//	ElfLoader_t loader;
	//	if (!elfloader_initFromMemory(&loader, buffer, currentSize))
	//	{
	//		writelog("could not init from memory\n");
	//		return 0;
	//	}

	//	if (!elfloader_isElfValid(&loader))
	//	{
	//		writelog("elf not valid\n");
	//		return 0;
	//	}

	//	if (!elfloader_handleRelocations(&loader))
	//	{
	//		writelog("could not handle relocation\n");
	//		return 0;
	//	}

	//	if (!loader.elfMain)
	//	{
	//		writelog("could not find main\n");
	//		return 0;
	//	}

	//	loader.elfMain();
	//}
	//else
	//{
	//	// Launch Userland Payload
	//	writelog("launching payload");

	//	void(*payload_start)() = (void(*)())buffer;
	//	payload_start();
	//}
}
