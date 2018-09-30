#include "pbserver.h"
#include <oni/utils/logger.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/kdlsym.h>
#include <nanopb/pb.h>
#include <nanopb/pb_encode.h>
#include <nanopb/pb_decode.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include <mira/miraframework.h>

#include "mirabuiltin.pb.h"



void pbserver_serverThread(void* userData);

void pbserver_init(struct pbserver_t* server)
{
	// Assigns all of the functions that we should need
	init_nanopb();

	if (!server)
		return;

	struct sockaddr_in serverAddress;

	int32_t listenSocket = ksocket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket < 0)
	{
		WriteLog(LL_Error, "could not initialize socket (%d).", listenSocket);
		return;
	}

	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	serverAddress.sin_port = htons(1234);

	int32_t ret = kbind(listenSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not bind socket (%d).", ret);
		return;
	}

	ret = klisten(listenSocket, 5);
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not listen on socket (%d).", ret);
		return;
	}

	server->listenSocket = listenSocket;
	server->connectionSocket = -1;
	server->reuse = false;
	server->thread = NULL;

	int(*kthread_add)(void(*func)(void*), void* arg, struct proc* procptr, struct thread** tdptr, int flags, int pages, const char* fmt, ...) = kdlsym(kthread_add);

	int creationResult = kthread_add(pbserver_serverThread, server, mira_getProc(), (struct thread**)&server->thread, 0, 0, "pbserver");

	WriteLog(LL_Debug, "creationResult (%d).", creationResult);
}

static bool write_callback(pb_ostream_t *stream, const uint8_t *buf, size_t count)
{
	int fd = (intptr_t)stream->state;
	return ksend(fd, (caddr_t)buf, count, 0) == count;
}

static bool read_callback(pb_istream_t *stream, uint8_t *buf, size_t count)
{
	int fd = (intptr_t)stream->state;
	int result;

	result = krecv(fd, buf, count, MSG_WAITALL);

	if (result == 0)
		stream->bytes_left = 0; /* EOF */

	return result == count;
}

pb_ostream_t pb_ostream_from_socket(int fd)
{
	pb_ostream_t stream = { &write_callback, (void*)(intptr_t)fd, SIZE_MAX, 0 };
	return stream;
}

pb_istream_t pb_istream_from_socket(int fd)
{
	pb_istream_t stream = { &read_callback, (void*)(intptr_t)fd, SIZE_MAX };
	return stream;
}

void handle_connection(int32_t conn)
{
	// Decode incoming message
	UtilsDumpHddKeysRequest request = { };
	pb_istream_t input = pb_istream_from_socket(conn);

	if (!pb_decode_delimited(&input, UtilsDumpHddKeysRequest_fields, &request))
	{
		WriteLog(LL_Error, "decoding failed (%s)", PB_GET_ERROR(&input));
		return;
	}

	UtilsDumpHddKeysResponse response = { };
	pb_ostream_t output = pb_ostream_from_socket(conn);

	response.error = -1;
	response.encrypted.funcs.encode = NULL;

	if (!pb_encode_delimited(&output, UtilsDumpHddKeysResponse_fields, &response))
	{
		WriteLog(LL_Error, "encoding failed (%s)", PB_GET_ERROR(&output));
		return;
	}


}

void pbserver_serverThread(void* userData)
{
	void(*kthread_exit)(void) = kdlsym(kthread_exit);

	if (!userData)
	{
		kthread_exit();
		return;
	}

	struct pbserver_t* server = (struct pbserver_t*)userData;


	for (;;)
	{
		// Wait for a client
		int32_t conn = kaccept(server->listenSocket, NULL, NULL);
		if (conn < 0)
			break;

		WriteLog(LL_Debug, "got new pbconnection (%d).", conn);

		handle_connection(conn);

		WriteLog(LL_Debug, "closing connection.");

		kclose(conn);
	}
	kthread_exit();
}