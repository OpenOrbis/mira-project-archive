#include <oni/rpc/pbconnection.h>
#include <oni/utils/logger.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/ref.h>

#include <oni/messaging/messagemanager.h>

#include <oni/framework.h>
#include <oni/init/initparams.h>
#include <string.h>


/// <summary>
/// Initializes a pbconnection structure
/// </summary>
/// <param name="connection">Connection</param>
void pbconnection_init(struct pbconnection_t* connection)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void(*mtx_init)(struct mtx *m, const char *name, const char *type, int opts) = kdlsym(mtx_init);

	if (!connection)
		return;

	memset(connection, 0, sizeof(connection));

	mtx_init(&connection->lock, "", NULL, 0);

	connection->socket = -1;
	connection->thread = NULL;
	connection->running = false;
}

/// <summary>
/// The main thread loop that each connection will run
/// </summary>
/// <param name="connection">Connection</param>
void pbconnection_thread(struct pbconnection_t* connection)
{
	void(*kthread_exit)(void) = kdlsym(kthread_exit);
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	//void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);

	if (!connection)
		return;

	if (connection->socket < 0)
		return;

	// Grab the lock, this should prevent races from the spawner thread
	_mtx_lock_flags(&connection->lock, 0, __FILE__, __LINE__);

	WriteLog(LL_Info, "pbconnection thread created socket: (%d), addr: (%x), thread: (%p).", connection->socket, connection->address.sin_addr.s_addr, connection->thread);

	// Do not hold this lock
	_mtx_unlock_flags(&connection->lock, 0, __FILE__, __LINE__);

	connection->running = true;

	const uint32_t maxMessageSize = 0x10000;
	while (connection->running)
	{
		uint32_t dataLength = 0;
		uint8_t* data = NULL;

		// Read the data length size
		ssize_t result = krecv(connection->socket, &dataLength, sizeof(dataLength), 0);

		// Verify the recv worked successfully
		if (result <= 0)
		{
			WriteLog(LL_Error, "recv returned (%d).", result);
			goto disconnect;
		}

		// Verify the data we wanted was recv'd
		if (result != sizeof(dataLength))
		{
			WriteLog(LL_Error, "did not recv enough data, got (%d) wanted (%d).", result, sizeof(dataLength));
			goto disconnect;
		}

		// We will set PAGE_SIZE as our artifical max length
		if (dataLength > maxMessageSize)
		{
			WriteLog(LL_Error, "data length (%x) > max (%x).", dataLength, maxMessageSize);
			goto disconnect;
		}

		// Allocate some new data
		data = k_malloc(dataLength);
		if (!data)
		{
			WriteLog(LL_Error, "could not allocate message length (%x).", dataLength);
			goto disconnect;
		}

		// Zero our newly allocated buffer
		memset(data, 0, dataLength);

		// Recv our message buffer
		uint32_t bufferRecv = 0;
		result = krecv(connection->socket, data, dataLength, 0);
		if (result <= 0)
		{
			WriteLog(LL_Error, "could not recv message data (%d).", result);
			goto disconnect;
		}

		// Set our current buffer recv count
		bufferRecv = result;

		// Ensure that we get all of our data
		while (bufferRecv < dataLength)
		{
			// Calculate how much data we have left
			uint32_t amountLeft = dataLength - bufferRecv;
			if (amountLeft == 0)
				break;

			// Attempt to read the rest of the buffer
			result = krecv(connection->socket, data + bufferRecv, amountLeft, 0);

			// Check for errors
			if (result <= 0)
			{
				WriteLog(LL_Error, "could not recv the rest of data (%d).", result);
				goto disconnect;
			}

			// Add the new amount of data that we recv'd
			bufferRecv += result;
		}

		// TODO: Fixme
		// Decode the message header		
		/*PbMessage* pbMessage = pb_message__unpack(NULL, dataLength, data);
		if (!pbMessage)
		{
			WriteLog(LL_Error, "could not decode header\n");
			goto disconnect;
		}*/

		// We no longer need the original buffer, free it as soon as possible
		k_free(data);
		data = NULL;

		// TODO: Fixme
		/*PbContainer* container = pbcontainer_create(pbMessage, false);
		if (!container)
		{
			pb_message__free_unpacked(pbMessage, NULL);
			WriteLog(LL_Error, "could not allocate pbcontainer memory.");
			goto disconnect;
		}*/

		// Validate the message category
		// TODO: Fixme
		//MessageCategory category = pbMessage->category;
		//if (category < MESSAGE_CATEGORY__NONE || category > MESSAGE_CATEGORY__MAX) // TODO: Add Max
		//{
		//	pbcontainer_release(container);
		//	WriteLog(LL_Error, "invalid category (%d).", category);
		//	goto disconnect;
		//}

		// Send the protobuf request off through the message system
		// TODO: fixme
		//messagemanager_sendRequest(container);

		// We no longer need to hold this reference
		// TODO: fixme
		//pbcontainer_release(container);
	}

disconnect:
	connection->running = false;

	// Validate everything and send the disconnect message, pbserver handles cleanup
	if (connection->server && connection->onClientDisconnect)
	{
		connection->onClientDisconnect(connection->server, connection);
	}

	kthread_exit();
}
