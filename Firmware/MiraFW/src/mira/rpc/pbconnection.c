#include "pbconnection.h"
#include <oni/utils/logger.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/ref.h>

#include <oni/messaging/messagemanager.h>

#include <nanopb/pb_common.h>
#include <nanopb/pb_decode.h>

#include <mira/rpc/mirabuiltin.pb.h>

void pbconnection_init(struct pbconnection_t* connection)
{
	if (!connection)
		return;

	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void(*mtx_init)(struct mtx *m, const char *name, const char *type, int opts) = kdlsym(mtx_init);

	memset(connection, 0, sizeof(connection));

	mtx_init(&connection->lock, "", NULL, 0);

	connection->socket = -1;
	connection->thread = NULL;
}

void pbconnection_thread(struct pbconnection_t* connection)
{
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	if (!connection)
		return;

	if (connection->socket < 0)
		return;

	// Grab the lock, this should prevent races from the spawner thread
	_mtx_lock_flags(&connection->lock, 0, "", 0);

	WriteLog(LL_Info, "pbconnection thread created socket: (%d), addr: (%x), thread: (%p).", connection->socket, connection->address.sin_addr.s_addr, connection->thread);

	// Do not hold this lock
	_mtx_unlock_flags(&connection->lock, 0, "", 0);

	connection->running = true;

	const uint32_t maxMessageSize = PAGE_SIZE;
	uint8_t* data = NULL;
	while (connection->running)
	{
		uint32_t dataLength = 0;
		data = NULL;

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

		// Decode the message header
		MessageHeader header;
		pb_istream_t stream = pb_istream_from_buffer(data, dataLength);
		if (!pb_decode(&stream, MessageHeader_fields, &header))
		{
			WriteLog(LL_Error, "could not decode header (%s).", PB_GET_ERROR(&stream));
			goto disconnect;
		}

		// Validate the message category
		MessageCategory category = header.category;
		if (category < MessageCategory_NONE || category > 5) // TODO: Add Max
		{
			WriteLog(LL_Error, "invalid category (%d).", category);
			goto disconnect;
		}

		// Validate the error code
		if (header.error != 0)
		{
			WriteLog(LL_Error, "error should not be set on requests (%d).", header.error);
			goto disconnect;
		}

		// This creates a copy of the data
		struct ref_t* reference = ref_fromObject(data, dataLength);
		if (!reference)
		{
			WriteLog(LL_Error, "could not create new reference of data");
			goto disconnect;
		}

		// TODO: You will have to change endpoint to accept protobuf
		//messagemanager_sendRequest(reference);
		k_free(data);
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
