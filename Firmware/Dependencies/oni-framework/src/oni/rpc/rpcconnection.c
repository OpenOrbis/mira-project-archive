#include <oni/rpc/rpcconnection.h>
#include <oni/utils/logger.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/ref.h>

#include <oni/messaging/messagemanager.h>
#include <oni/messaging/messagecontainer.h>

#include <oni/framework.h>
#include <oni/init/initparams.h>

#include <oni/utils/escape.h>

/// <summary>
/// Initializes a pbconnection structure
/// </summary>
/// <param name="connection">Connection</param>
void rpcconnection_init(struct rpcconnection_t* connection)
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
void rpcconnection_thread(struct rpcconnection_t* connection)
{
	void(*kthread_exit)(void) = kdlsym(kthread_exit);
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);

	if (!connection)
		return;

	if (connection->socket < 0)
		return;

	// Grab the lock, this should prevent races from the spawner thread
	_mtx_lock_flags(&connection->lock, 0, __FILE__, __LINE__);

	WriteLog(LL_Info, "pbconnection thread created socket: (%d), addr: (%x), thread: (%p).", connection->socket, connection->address.sin_addr.s_addr, connection->thread);

	// Do not hold this lock
	_mtx_unlock_flags(&connection->lock, 0, __FILE__, __LINE__);

	struct thread_info_t threadInfo;
	oni_threadEscape(curthread, &threadInfo);

	connection->running = true;

	while (connection->running)
	{
		struct messageheader_t header;
		memset(&header, 0, sizeof(header));

		// Read the data length size
		ssize_t result = krecv(connection->socket, &header, sizeof(header), 0);
		if (result <= 0)
		{
			WriteLog(LL_Error, "recv returned (%d).", result);
			goto disconnect;
		}

		// Verify the data we wanted was recv'd
		if (result != sizeof(header))
		{
			WriteLog(LL_Error, "did not recv enough data, got (%d) wanted (%d).", result, sizeof(header));
			goto disconnect;
		}

		// Validate header
		if (header.magic != MESSAGEHEADER_MAGIC)
		{
			WriteLog(LL_Error, "incorrect magic got(%d) wanted (%d).", header.magic, MESSAGEHEADER_MAGIC);
			goto disconnect;
		}

		// Validate the message category
		enum MessageCategory msgCategory = (enum MessageCategory)header.category;
		if (msgCategory < MessageCategory_None || msgCategory > MessageCategory_Max)
		{
			WriteLog(LL_Error, "invalid category (%d).", msgCategory);
			goto disconnect;
		}

		if (header.isRequest != true)
		{
			WriteLog(LL_Error, "tried to handle a outgoing message, check ya code");
			goto disconnect;
		}

		// Validate maximum payload length
		if (header.payloadLength > MESSAGECONTAINER_MAXBUFSZ)
		{
			WriteLog(LL_Error, "data length (%x) > max (%x).", header.payloadLength, MESSAGECONTAINER_MAXBUFSZ);
			goto disconnect;
		}

		// Allocate some new data
		size_t allocationSize = sizeof(struct messagecontainer_t) + header.payloadLength;
		struct messagecontainer_t* messageContainer = kmalloc(allocationSize);
		if (messageContainer == NULL)
		{
			WriteLog(LL_Error, "could not allocate message container");
			goto disconnect;
		}
		// Zero our newly allocated buffer
		memset(messageContainer, 0, allocationSize);

		messageContainer->count = 0;

		// copy over the header
#ifdef _DEBUG
		if (sizeof(&messageContainer->header) != sizeof(header))
			WriteLog(LL_Error, "header size mismatch, yo code is broken");
#endif
		(void)memcpy(&messageContainer->header, &header, sizeof(header));

		// Recv our payload buffer
		uint32_t bufferRecv = 0;
		result = krecv(connection->socket, messageContainer->payload, header.payloadLength, 0);
		if (result <= 0)
		{
			// Free our messagecontainer first
			kfree(messageContainer, sizeof(struct messagecontainer_t) + messageContainer->header.payloadLength);
			messageContainer = NULL;

			WriteLog(LL_Error, "could not recv message data (%d).", result);
			goto disconnect;
		}

		// Set our current buffer recv count
		bufferRecv = result;

		// Ensure that we get all of our data
		while (bufferRecv < header.payloadLength)
		{
			// Calculate how much data we have left
			uint32_t amountLeft = header.payloadLength - bufferRecv;
			if (amountLeft == 0)
				break;

			// Attempt to read the rest of the buffer
			result = krecv(connection->socket, &messageContainer->payload[bufferRecv], amountLeft, 0);

			// Check for errors
			if (result <= 0)
			{
				// Free our messagecontainer first
				kfree(messageContainer, sizeof(struct messagecontainer_t) + messageContainer->header.payloadLength);
				messageContainer = NULL;

				WriteLog(LL_Error, "could not recv the rest of data (%d).", result);
				goto disconnect;
			}

			// Add the new amount of data that we recv'd
			bufferRecv += result;
		}

		//WriteLog(LL_Error, "at least we got here");

		messagecontainer_acquire(messageContainer);

		/*WriteLog(LL_Debug, "GOT MSG: c(%d) t(%x) m(%p) s(0x%x)", 
			messageContainer->header.category,
			messageContainer->header.errorType,
			messageContainer->payload,
			messageContainer->header.payloadLength);*/

		// Send the protobuf request off through the message system
		messagemanager_sendRequest(messageContainer);

		// Release our reference, any other threads would have acquired by now
		messagecontainer_release(messageContainer);
	}

disconnect:
	connection->running = false;

	oni_threadRestore(curthread, &threadInfo);

	// Validate everything and send the disconnect message, pbserver handles cleanup
	if (connection->server && connection->onClientDisconnect)
		connection->onClientDisconnect(connection->server, connection);

	kthread_exit();
}
