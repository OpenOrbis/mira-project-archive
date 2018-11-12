#include <oni/utils/types.h>

#include <loader/elfloader.h>
#include <utils/notify.h>
#include <utils/utils.h>

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
#include <sys/proc.h>

#include <netinet/in.h>

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


void* mira_entry(void* args)
{
	// Escape the jail and sandbox
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



	// Allocate a 5MB buffer
	uint8_t* buffer = (uint8_t*)_Allocate5MB();
	size_t bufferSize = 0x500000;
	if (!buffer)
	{
		WriteLizog("could not allocate 5MB buffer");
		return NULL;
	}
	loader_memset(buffer, 0, bufferSize);

	// Hold our server socket address
	struct sockaddr_in serverAddress;
	loader_memset(&serverAddress, 0, sizeof(serverAddress));

	// Listen on port 9021
	serverAddress.sin_len = sizeof(serverAddress);
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = __bswap16(9021); // port 9020
	serverAddress.sin_family = AF_INET;

	// Create a new socket
	int32_t serverSocket = sceNetSocket("loader", AF_INET, SOCK_STREAM, 0);
	if (serverSocket < 0)
	{
		WriteLizog("socket error");
		return 0;
	}

	// Bind to localhost
	int32_t result = sceNetBind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	if (result < 0)
	{
		WriteLizog("bind error");
		return 0;
	}

	// Listen
	result = sceNetListen(serverSocket, 10);

	WriteLizog("waiting for clients");

	// Wait for a client to send something
	int32_t clientSocket = sceNetAccept(serverSocket, NULL, NULL);
	if (clientSocket < 0)
	{
		WriteLizog("accept errror");
		return 0;
	}

	int32_t currentSize = 0;
	int32_t recvSize = 0;

	// Recv one byte at a time until we get our buffer
	while ((recvSize = sceNetRecv(clientSocket, buffer + currentSize, bufferSize - currentSize, 0)) > 0)
		currentSize += recvSize;

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

		// TODO: Check/Add a flag to the elf that determines if this is a kernel or userland elf
		ElfLoader_t loader;
		if (!elfloader_initFromMemory(&loader, buffer, currentSize))
		{
			WriteLizog("could not init from memory");
			return NULL;
		}

		if (!elfloader_isElfValid(&loader))
		{
			WriteLizog("elf not valid");
			return NULL;
		}

		if (!elfloader_handleRelocations(&loader))
		{
			WriteLizog("could not handle relocation");
			return NULL;
		}

		if (!loader.elfMain)
		{
			WriteLizog("could not find main");
			return NULL;
		}

		char buf[64];
		loader_memset(buf, 0, sizeof(buf));

		snprintf(buf, sizeof(buf), "elfMain: %p", loader.elfMain);

		WriteLizog(buf);

		loader.elfMain();
	}
	else
	{
		// Launch Userland Payload
		WriteLizog("launching payload");

		void(*payload_start)() = (void(*)())buffer;
		payload_start();
	}

	return NULL;
}
