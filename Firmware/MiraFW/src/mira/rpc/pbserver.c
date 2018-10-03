#include "pbserver.h"
#include <oni/utils/logger.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/memory/allocator.h>

#include <nanopb/pb.h>
#include <nanopb/pb_encode.h>
#include <nanopb/pb_decode.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include <mira/miraframework.h>

#include "mirabuiltin.pb.h"

#include "pbconnection.h"



void pbserver_serverThread(void* userData);

void pbserver_init(struct pbserver_t* server)
{
	void(*mtx_init)(struct mtx *m, const char *name, const char *type, int opts) = kdlsym(mtx_init);

	int(*kthread_add)(void(*func)(void*), void* arg, struct proc* procptr, struct thread** tdptr, int flags, int pages, const char* fmt, ...) = kdlsym(kthread_add);


	// Assigns all of the functions that we should need
	init_nanopb();

	if (!server)
		return;

	// Zero out the entire server structure
	memset(server, 0, sizeof(*server));
	
	// Initialize the connections lock
	mtx_init(&server->connectionsLock, "", NULL, 0);

	// Create a new socket
	server->socket = ksocket(AF_INET, SOCK_STREAM, 0);
	if (server->socket < 0)
	{
		WriteLog(LL_Error, "could not initialize socket (%d).", listenSocket);
		return;
	}

	// Set the server address information
	server->address.sin_family = AF_INET;
	server->address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	server->address.sin_port = htons(9999); // TODO: Make configurable

	// Bind to the port
	int32_t ret = kbind(server->socket, (struct sockaddr*)&server->address, sizeof(server->address));
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not bind socket (%d).", ret);
		return;
	}

	ret = klisten(server->socket, ARRAYSIZE(server->connections));
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not listen on socket (%d).", ret);
		return;
	}

	// Create the new thread
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

void pbserver_serverThread(void* userData)
{
	void(*kthread_exit)(void) = kdlsym(kthread_exit);

	// Verify that we have a server object
	if (!userData)
	{
		kthread_exit();
		return;
	}
	struct pbserver_t* server = (struct pbserver_t*)userData;

	// Update our running state
	server->running = true;

	// While the server is still running
	while (server->running)
	{
		struct pbconnection_t* connection = k_malloc(sizeof(struct pbconnection_t));
		if (!connection)
			goto exit;

		// Initialize our structures
		pbconnection_init(connection);

		// Wait for a client
		size_t addressSize = sizeof(connection->address);

		connection->socket = kaccept(server->socket, &connection->address, &addressSize);

		// Check for any errors
		if (connection->socket < 0)
		{
			WriteLog(LL_Error, "could not accept socket (%d).", connection->socket);
			k_free(connection);
			connection = NULL;
			goto exit;
		}

		WriteLog(LL_Debug, "got new pbconnection (%d).", conn);

		// Nothing below this should jump out in error
		_mtx_lock_flags(&server->connectionsLock, 0, "", 0);

		int32_t connectionIndex = pbserver_findFreeConnectionIndex(server);


		// Handle error case
		if (connectionIndex < 0)
		{
			WriteLog(LL_Error, "could not find a free index.");
			k_free(connection);
			connection = NULL;
		}
		else
		{
			// Set our connection in our server
			server->connections[connectionIndex] = connection;

			// Fire off a new connection thread
			pbserver_handleConnection(server, connection);

			WriteLog(LL_Debug, "added new connection (%p) to index (%d).", connection, connectionIndex);
		}
		
		_mtx_unlock_flags(&server->connectionsLock, 0, "", 0);
	}

exit:
	server->running = false;

	WriteLog(LL_Debug, "pbserver is shutting down.");
	kthread_exit();
}

void pbserver_handleConnection(struct pbserver_t* server, struct pbconnection_t* connection)
{
	int(*kthread_add)(void(*func)(void*), void* arg, struct proc* procptr, struct thread** tdptr, int flags, int pages, const char* fmt, ...) = kdlsym(kthread_add);
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	if (!connection || !server)
		return;

	if (connection->socket < 0)
		return;

	void* miraProc = mira_getProc();
	if (!miraProc)
		return;

	// Lock the connection thread
	_mtx_lock_flags(&connection->lock, 0, "", 0);

	int32_t result = kthread_add(pbconnection_thread, connection, miraProc, &connection->thread, 0, 0, "pbconn");

	WriteLog(LL_Debug, "pbconn thread creation: (%d).", result);

	_mtx_unlock_flags(&connection->lock, 0, "", 0);

	if (result < 0)
	{
		WriteLog(LL_Error, "could not create new connection thread (%d).", result);

		// Forcefully disconnect the client
		pbserver_handleClientDisconnect(server, connection);

		return;
	}
}

void pbserver_handleClientDisconnect(struct pbserver_t* server, struct pbconnection_t* connection)
{
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	// Verify that our server and connection objects are valid
	if (!server || !connection)
		return;

	// If we have a valid socket, shutdown and close it
	if (connection->socket > -1)
	{
		kshutdown(connection->socket, SHUT_RDWR);
		kclose(connection->socket);
	}

	int32_t connectionIndex = pbserver_findConnectionIndex(server, connection);
	// If there's a non-negative index then remove the
	if (connectionIndex > -1)
	{
		_mtx_lock_flags(&server->connectionsLock, 0, "", 0);

		server->connections[connectionIndex] = NULL;
		
		_mtx_unlock_flags(&server->connectionsLock, 0, "", 0);
	}

	// Free the connection since everything should be fine to be cleared
	if (connection)
		k_free(connection);
}

int32_t pbserver_findFreeConnectionIndex(struct pbserver_t* server)
{
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	if (!server)
		return -1;

	int32_t foundIndex = -1;

	_mtx_lock_flags(&server->connectionsLock, 0, "", 0);
	for (size_t index = 0; index < ARRAYSIZE(server->connections); index++)
	{
		if (server->connections[index] == NULL)
		{
			foundIndex = index;
			break;
		}
	}
	_mtx_unlock_flags(&server->connectionsLock, 0, "", 0);

	return foundIndex;
}

int32_t pbserver_findConnectionIndex(struct pbserver_t* server, struct pbconnection_t* connection)
{
	if (!server || !connection)
		return -1;

	int32_t foundIndex = -1;

	_mtx_lock_flags(&server->connectionsLock, 0, "", 0);
	for (size_t index = 0; index < ARRAYSIZE(server->connections); index++)
	{
		if (server->connections[index] == connection)
		{
			foundIndex = index;
			break;
		}
	}
	_mtx_unlock_flags(&server->connectionsLock, 0, "", 0);

	return foundIndex;
}