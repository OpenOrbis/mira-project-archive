#include <oni/rpc/rpcserver.h>
#include <oni/rpc/rpcconnection.h>
#include <oni/utils/logger.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/memory/allocator.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include <oni/framework.h>
#include <oni/init/initparams.h>

void rpcserver_serverThread(void* userData);

void rpcserver_init(struct rpcserver_t* server)
{
	void(*mtx_init)(struct mtx *m, const char *name, const char *type, int opts) = kdlsym(mtx_init);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	// Assigns all of the functions that we should need
	if (!server)
		return;

	// Zero out the entire server structure
	memset(server, 0, sizeof(*server));
	
	// Initialize the connections lock
	mtx_init(&server->connectionsLock, "", NULL, 0);
}

uint8_t rpcserver_startup(struct rpcserver_t* server, uint16_t port)
{
	int(*kthread_add)(void(*func)(void*), void* arg, struct proc* procptr, struct thread** tdptr, int flags, int pages, const char* fmt, ...) = kdlsym(kthread_add);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	WriteLog(LL_Warn, "here");

	// Create a new socket
	server->socket = ksocket(AF_INET, SOCK_STREAM, 0);
	if (server->socket < 0)
	{
		WriteLog(LL_Error, "could not initialize socket (%d).", server->socket);
		return false;
	}

	WriteLog(LL_Warn, "socket created: %d", server->socket);


	// Set the server address information
	memset(&server->address, 0, sizeof(server->address));
	server->address.sin_family = AF_INET;
	server->address.sin_addr.s_addr = htonl(INADDR_ANY);
	server->address.sin_port = htons(9999); // TODO: Make configurable
	server->address.sin_len = sizeof(server->address);

	WriteLog(LL_Warn, "filled out information");

	// Bind to the port
	int32_t ret = kbind(server->socket, (struct sockaddr*)&server->address, sizeof(server->address));
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not bind socket (%d).", ret);
		kclose(server->socket);
		server->socket = -1;
		return false;
	}

	WriteLog(LL_Warn, "socket bound: %d", server->socket);

	ret = klisten(server->socket, ARRAYSIZE(server->connections));
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not listen on socket (%d).", ret);
		kclose(server->socket);
		server->socket = -1;
		return false;
	}

	// Create the new thread
	int creationResult = kthread_add(rpcserver_serverThread, server, gInitParams->process, (struct thread**)&server->thread, 0, 0, "rpcserver");

	WriteLog(LL_Debug, "creationResult (%d).", creationResult);

	return creationResult == 0;
}

uint8_t rpcserver_shutdown(struct rpcserver_t* server)
{
	if (!server)
		return false;

	WriteLog(LL_Warn, "here");


	if (server->socket == -1)
		return false;

	WriteLog(LL_Warn, "here");

	server->running = false;

	WriteLog(LL_Warn, "here");

	// Iterate through each of the connections and force connections to error and get cleaned up
	for (uint32_t i = 0; i < ARRAYSIZE(server->connections); ++i)
	{
		struct rpcconnection_t* connection = server->connections[i];
		if (!connection)
			continue;

		if (connection->socket < 0)
			continue;

		kshutdown(connection->socket, 2);
		kclose(connection->socket);
		connection->socket = -1;
	}

	WriteLog(LL_Warn, "here");

	// Shut down the actual server socket
	if (server->socket >= 0)
	{
		kshutdown(server->socket, 2);
		kclose(server->socket);
		server->socket = -1;
	}

	return true;
}

void rpcserver_serverThread(void* userData)
{
	void(*kthread_exit)(void) = kdlsym(kthread_exit);
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	// Verify that we have a server object
	if (!userData)
	{
		kthread_exit();
		return;
	}
	struct rpcserver_t* server = (struct rpcserver_t*)userData;

	// Update our running state
	server->running = true;

	// While the server is still running
	while (server->running)
	{
		struct rpcconnection_t* connection = k_malloc(sizeof(struct rpcconnection_t));
		if (!connection)
			goto exit;

		// Initialize our structures
		rpcconnection_init(connection);

		// Wait for a client
		size_t addressSize = sizeof(connection->address);

		connection->address.sin_len = sizeof(connection->address);

		connection->socket = kaccept(server->socket, (struct sockaddr*)&connection->address, &addressSize);

		// Check for any errors
		if (connection->socket < 0)
		{
			WriteLog(LL_Error, "could not accept socket (%d).", connection->socket);
			k_free(connection);
			connection = NULL;
			goto exit;
		}

		WriteLog(LL_Debug, "got new pbconnection (%d).", connection->socket);

		// Nothing below this should jump out in error
		_mtx_lock_flags(&server->connectionsLock, 0, "", 0);

		// Handle error case
		int32_t connectionIndex = rpcserver_findFreeConnectionIndex(server);
		if (connectionIndex < 0)
		{
			WriteLog(LL_Error, "could not find a free index.");
			kclose(connection->socket);
			k_free(connection);
			connection = NULL;
		}
		else
		{
			// Set the disconnect callback
			connection->onClientDisconnect = rpcserver_handleClientDisconnect;

			// Set our connection in our server
			server->connections[connectionIndex] = connection;

			// Fire off a new connection thread
			rpcserver_handleConnection(server, connection);

			WriteLog(LL_Debug, "added new connection (%p) to index (%d).", connection, connectionIndex);
		}
		
		_mtx_unlock_flags(&server->connectionsLock, 0, "", 0);
	}

exit:
	WriteLog(LL_Warn, "here");

	server->running = false;

	WriteLog(LL_Debug, "pbserver is shutting down.");
	kthread_exit();
}

void rpcserver_handleConnection(struct rpcserver_t* server, struct rpcconnection_t* connection)
{
	int(*kthread_add)(void(*func)(void*), void* arg, struct proc* procptr, struct thread** tdptr, int flags, int pages, const char* fmt, ...) = kdlsym(kthread_add);
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	if (!connection || !server)
		return;

	if (connection->socket < 0)
		return;

	struct proc* miraProc = gInitParams->process;
	if (!miraProc)
		return;

	// Lock the connection thread
	_mtx_lock_flags(&connection->lock, 0, __FILE__, __LINE__);

	int32_t result = kthread_add((void(*)(void*))rpcconnection_thread, (void*)connection, miraProc, &connection->thread, 0, 0, "pbconn");

	WriteLog(LL_Debug, "pbconn thread creation: (%d).", result);

	_mtx_unlock_flags(&connection->lock, 0, __FILE__, __LINE__);

	if (result < 0)
	{
		WriteLog(LL_Error, "could not create new connection thread (%d).", result);

		// Forcefully disconnect the client
		rpcserver_handleClientDisconnect(server, connection);

		return;
	}
}

void rpcserver_handleClientDisconnect(struct rpcserver_t* server, struct rpcconnection_t* connection)
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

	int32_t connectionIndex = rpcserver_findConnectionIndex(server, connection);
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

int32_t rpcserver_findFreeConnectionIndex(struct rpcserver_t* server)
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

int32_t rpcserver_findConnectionIndex(struct rpcserver_t* server, struct rpcconnection_t* connection)
{
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

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

int32_t rpcserver_findSocketFromThread(struct rpcserver_t* server, struct thread* td)
{
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	if (!server || !td)
		return -1;

	int32_t foundSocket = -1;

	_mtx_lock_flags(&server->connectionsLock, 0, "", 0);
	for (size_t index = 0; index < ARRAYSIZE(server->connections); index++)
	{
		struct rpcconnection_t* connection = server->connections[index];
		if (connection == NULL)
			continue;

		if (connection->thread == td)
		{
			foundSocket = connection->socket;
			break;
		}
	}
	_mtx_unlock_flags(&server->connectionsLock, 0, "", 0);

	return foundSocket;
}