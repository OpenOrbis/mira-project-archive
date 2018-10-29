//#include "loader.h"
//#include "dynlib.h"
//
//
//int(*sceSysUtilSendSystemNotificationWithText)(int messageType, char* message) = NULL;
//
//int(*sceKernelLoadStartModule)(const char *name, size_t argc, const void *argv, unsigned int flags, int, int) = NULL;
//
//
//int(*sceNetSocket)(const char *, int, int, int) = NULL;
//int(*sceNetSocketClose)(int) = NULL;
//int(*sceNetConnect)(int, struct sockaddr *, int) = NULL;
//int(*sceNetSend)(int, const void *, size_t, int) = NULL;
//int(*sceNetBind)(int, struct sockaddr *, int) = NULL;
//int(*sceNetListen)(int, int) = NULL;
//int(*sceNetAccept)(int, struct sockaddr *, unsigned int *) = NULL;
//int(*sceNetRecv)(int, void *, size_t, int) = NULL;
//int(*sceNetSocketAbort)(int, int) = NULL;
//
//void main(int32_t argCount, char* args[])
//{
//	// Unused
//}
//
//void loader_init()
//{
//	int32_t sysUtilModuleId = -1;
//	int32_t netModuleId = -1;
//	int32_t libKernelWebModuleId = -1;
//
//	// libkernel
//	{
//		sys_dynlib_load_prx("libkernel.sprx", &libKernelWebModuleId);
//
//		sys_dynlib_dlsym(libKernelWebModuleId, "sceKernelLoadStartModule", &sceKernelLoadStartModule);
//	}
//
//	// initialize system notifications
//	{
//		sys_dynlib_load_prx("/system/common/lib/libSceSysUtil.sprx", &sysUtilModuleId);
//
//		sys_dynlib_dlsym(sysUtilModuleId, "sceSysUtilSendSystemNotificationWithText", &sceSysUtilSendSystemNotificationWithText);
//	}
//
//	// 
//	{
//		sys_dynlib_load_prx("/system/common/lib/libSceSysUtil.sprx", &libKernelWebModuleId);
//
//		sys_dynlib_dlsym(libKernelWebModuleId, "open", &sceSysUtilSendSystemNotificationWithText);
//	}
//
//	// Networking resolving
//	{
//		sys_dynlib_load_prx("libSceNet.sprx", &netModuleId);
//
//		sys_dynlib_dlsym(netModuleId, "sceNetSocket", &sceNetSocket);
//		sys_dynlib_dlsym(netModuleId, "sceNetSocketClose", &sceNetSocketClose);
//		sys_dynlib_dlsym(netModuleId, "sceNetBind", &sceNetBind);
//		sys_dynlib_dlsym(netModuleId, "sceNetListen", &sceNetListen);
//		sys_dynlib_dlsym(netModuleId, "sceNetAccept", &sceNetAccept);
//		sys_dynlib_dlsym(netModuleId, "sceNetRecv", &sceNetRecv);
//	}
//}
//
//void loader_uninit()
//{
//
//}
//
//void webkit_entry(struct initparams_t* webkitParams)
//{
//	// Do not do anything if the webkit parameters aren't set correctly
//	if (!webkitParams)
//		return;
//
//	uint64_t payloadAddress = webkitParams->payloadBase;
//	uint64_t payloadSize = webkitParams->payloadSize;
//
//	// Verify that we have a valid address, and size
//	if (!payloadAddress || !payloadSize)
//		return;
//}
//
//uint8_t allocate_memory(uint8_t** outputData, uint32_t* outputSize)
//{
//	if (!outputData || !outputSize)
//		return false;
//
//	uint32_t allocationSize = *outputSize;
//	if (!allocationSize)
//		return false;
//
//
//}
//
//uint8_t install_and_relocate()
//{
//	return false;
//}
//
//#define bswap16(x) (((x) >> 8) | ((x) << 8))
//
//int32_t _mmap(void* addr, int len, int prot, int flags, int fd, long pos)
//{
//
//}
//
//void blah()
//{
//	uint8_t buffer[0x4000];
//
//	// Initialize
//	struct sockaddr_in serverAddress;
//	serverAddress.sin_len = sizeof(serverAddress);
//	serverAddress.sin_addr.s_addr = INADDR_ANY;
//	serverAddress.sin_port = bswap16(9020);
//
//	for (size_t i = 0; i < ARRAYSIZE(serverAddress.sin_zero); ++i)
//		serverAddress.sin_zero[i] = 0;
//
//	// Create a new socket
//	int32_t serverSocket = sceNetSocket("loader", AF_INET, SOCK_STREAM, 0);
//	if (serverSocket < 0)
//		return;
//
//	// Bind to localhost
//	int32_t result = sceNetBind(serverSocket, &serverAddress, sizeof(serverAddress));
//	if (result < 0)
//		return;
//
//	// Listen
//	result = sceNetListen(serverSocket, 10);
//
//	// Wait for a client to send something
//	int32_t clientSocket = sceNetAccept(serverSocket, NULL, NULL);
//	if (clientSocket < 0)
//		return;
//
//	size_t currentSize = 0;
//	size_t recvSize = 0;
//
//	// Recv one byte at a time until we get our buffer
//	while ((recvSize = sceNetRecv(clientSocket, buffer + currentSize, sizeof(uint8_t), 0) > 0))
//	{
//		currentSize += recvSize;
//
//		if (sizeof(buffer) - 1 >= currentSize)
//			break;
//	}
//
//	// Close the client and server socket connections
//	sceNetSocketClose(clientSocket);
//	sceNetSocketClose(serverSocket);
//
//	clientSocket = -1;
//	serverSocket = -1;
//
//	uint32_t magic = *(uint32_t*)buffer;
//	if (magic == 0x464C457F)
//	{
//		// Launch ELF
//
//		// TODO: Check/Add a flag to the elf that determines if this is a kernel or userland elf
//	}
//	else
//	{
//		// Launch Userland Payload
//
//		void(*payload_start)() = (void(*)())buffer;
//		payload_start();
//	}
////}