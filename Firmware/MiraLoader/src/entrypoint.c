#include <oni/utils/types.h>

#include <loader/elfloader.h>

#include <oni/init/initparams.h>
#include <oni/utils/syscall.h>
#include <oni/utils/dynlib.h>
#include <oni/utils/escape.h>
#include <oni/utils/patches.h>
#include <oni/utils/kernel.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/cpu.h>

#include <sys/elf64.h>
#include <sys/socket.h>
#include <netinet/in.h>

//uint8_t buffer[0x4000];

void loader_displayNotification(int32_t id, char* text);

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
	loader_displayNotification(222, msg);

	//struct mdbg_service_arg arg = 
	//{
	//	0,		// unknown0
	//	0,		// padding4
	//	msg,	// unknown8
	//	0,		// unknown10
	//	0,		// unknown18
	//	0		// unknown20
	//};

	//mdbg_service(0x7, &arg, NULL);
}

uintptr_t __stack_chk_guard = 0;

void __stack_chk_fail(void)
{

}

#include <sys/proc.h>

void mira_escape(struct thread* td, void* uap)
{
	gKernelBase = (uint8_t*)kernelRdmsr(0xC0000082) - kdlsym_addr_Xfast_syscall;

	void(*critical_enter)(void) = kdlsym(critical_enter);
	void(*crtical_exit)(void) = kdlsym(critical_exit);

	struct ucred* cred = td->td_proc->p_ucred;
	struct filedesc* fd = td->td_proc->p_fd;

	cred->cr_uid = 0;
	cred->cr_ruid = 0;
	cred->cr_rgid = 0;
	cred->cr_groups[0] = 0;

	cred->cr_prison = *(void**)kdlsym(prison0);
	fd->fd_rdir = fd->fd_jdir = *(void**)kdlsym(rootvnode);

	// Apply patches
	critical_enter();
	cpu_disable_wp();

	oni_installPrePatches();

	cpu_enable_wp();
	crtical_exit();
}

void loader_displayNotification(int32_t id, char* text)
{
	if (!text)
		return;

	// Prompt the user
	int moduleId = -1;
	sys_dynlib_load_prx("/system/common/lib/libSceSysUtil.sprx", &moduleId);

	if (moduleId == -1)
		return;

	int(*sceSysUtilSendSystemNotificationWithText)(int messageType, char* message) = NULL;

	sys_dynlib_dlsym(moduleId, "sceSysUtilSendSystemNotificationWithText", &sceSysUtilSendSystemNotificationWithText);

	if (sceSysUtilSendSystemNotificationWithText)
		sceSysUtilSendSystemNotificationWithText(222, text);

	sys_dynlib_unload_prx(moduleId);
}

void ghetto_memset(void* address, int32_t val, size_t len)
{
	for (size_t i = 0; i < len; ++i)
		*(((uint8_t*)address) + i) = 0;
}
void* mira_entry(void* args)
{
	//int(*sceKernelLoadStartModule)(const char *name, size_t argc, const void *argv, unsigned int flags, int, int) = NULL;
	int(*sceNetSocket)(const char *, int, int, int) = NULL;
	int(*sceNetSocketClose)(int) = NULL;
	//int(*sceNetConnect)(int, struct sockaddr *, int) = NULL;
	//int(*sceNetSend)(int, const void *, size_t, int) = NULL;
	int(*sceNetBind)(int, struct sockaddr *, int) = NULL;
	int(*sceNetListen)(int, int) = NULL;
	int(*sceNetAccept)(int, struct sockaddr *, unsigned int *) = NULL;
	int(*sceNetRecv)(int, void *, size_t, int) = NULL;
	//int(*sceNetSocketAbort)(int, int) = NULL;

	int(*snprintf)(char *str, size_t size, const char *format, ...) = NULL;

	syscall2(11, mira_escape, NULL);

	//int32_t sysUtilModuleId = -1;
	int32_t netModuleId = -1;
	int32_t libcModuleId = -1;
	//int32_t libKernelWebModuleId = -1;

	{
		sys_dynlib_load_prx("libSceLibcInternal.sprx", &libcModuleId);

		sys_dynlib_dlsym(libcModuleId, "snprintf", &snprintf);
	}

	// Networking resolving
	{
		sys_dynlib_load_prx("libSceNet.sprx", &netModuleId);

		sys_dynlib_dlsym(netModuleId, "sceNetSocket", &sceNetSocket);
		sys_dynlib_dlsym(netModuleId, "sceNetSocketClose", &sceNetSocketClose);
		sys_dynlib_dlsym(netModuleId, "sceNetBind", &sceNetBind);
		sys_dynlib_dlsym(netModuleId, "sceNetListen", &sceNetListen);
		sys_dynlib_dlsym(netModuleId, "sceNetAccept", &sceNetAccept);
		sys_dynlib_dlsym(netModuleId, "sceNetRecv", &sceNetRecv);
	}

	// Initialize
	uint8_t buffer[PAGE_SIZE];
	ghetto_memset(buffer, 0, sizeof(buffer));

	struct sockaddr_in serverAddress;

	ghetto_memset(&serverAddress, 0, sizeof(serverAddress));

	serverAddress.sin_len = sizeof(serverAddress);
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = __bswap16(9021); // port 9020
	serverAddress.sin_family = AF_INET;
	
	// Create a new socket
	int32_t serverSocket = sceNetSocket("loader", AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0)
	{
		writelog("socket error\n");
		return 0;
	}

	// Bind to localhost
	int32_t result = sceNetBind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	if (result < 0)
	{
		writelog("bind error\n");
		return 0;
	}

	// Listen
	result = sceNetListen(serverSocket, 10);

	writelog("waiting for clients\n");

	// Wait for a client to send something
	int32_t clientSocket = sceNetAccept(serverSocket, NULL, NULL);
	if (clientSocket < 0)
	{
		writelog("accept errror\n");
		return 0;
	}

	int32_t currentSize = 0;
	int32_t recvSize = 0;

	// Recv one byte at a time until we get our buffer
	while ((recvSize = sceNetRecv(clientSocket, buffer + currentSize, sizeof(buffer) - currentSize, 0)) > 0)
	{
		currentSize += recvSize;

		if (sizeof(buffer) - 1 >= currentSize)
			break;
	}

	// Close the client and server socket connections
	sceNetSocketClose(clientSocket);
	sceNetSocketClose(serverSocket);

	clientSocket = -1;
	serverSocket = -1;

	// Determine if we launch a elf or a payload
	if (buffer[0] == ELFMAG0 &&
		buffer[1] == ELFMAG1 &&
		buffer[2] == ELFMAG2 &&
		buffer[3] == ELFMAG3) // 0x7F 'ELF'
	{
		// Launch ELF
		writelog("launching elf\n");

		// TODO: Check/Add a flag to the elf that determines if this is a kernel or userland elf
		ElfLoader_t loader;
		if (!elfloader_initFromMemory(&loader, buffer, currentSize))
		{
			writelog("could not init from memory\n");
			return NULL;
		}

		if (!elfloader_isElfValid(&loader))
		{
			writelog("elf not valid\n");
			return NULL;
		}

		if (!elfloader_handleRelocations(&loader))
		{
			writelog("could not handle relocation\n");
			return NULL;
		}

		if (!loader.elfMain)
		{
			writelog("could not find main\n");
			return NULL;
		}

		char buf[64];
		ghetto_memset(buf, 0, sizeof(buf));

		snprintf(buf, sizeof(buf), "elfMain: %p", loader.elfMain);

		writelog(buf);

		loader.elfMain();
	}
	else
	{
		// Launch Userland Payload
		writelog("launching payload");

		void(*payload_start)() = (void(*)())buffer;
		payload_start();
	}

	return NULL;
}
