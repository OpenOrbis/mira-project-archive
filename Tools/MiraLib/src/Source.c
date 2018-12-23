#define _CRT_SECURE_NO_WARNINGS

#include "protobuf-c.h"
#include "fileexplorer.pb-c.h"
#include "mirabuiltin.pb-c.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PAGE_SIZE 0x4000


int main()
{
	PbMessage message = PB_MESSAGE__INIT;

	EchoResponse response = ECHO_RESPONSE__INIT;
	IntValue error = INT_VALUE__INIT;
	error.value = ERRORS__EOK;

	response.error = &error;
	size_t size2 = echo_response__get_packed_size(&response);

	message.data.data = (uint8_t*)malloc(size2);
	message.data.len = size2;

	size_t size = pb_message__get_packed_size(&message);

	uint8_t* serializedData = (uint8_t*)malloc(size);
	if (!serializedData)
	{
		fprintf(stderr, "could not allocate data\n");
		return -1;
	}
	memset(serializedData, 0, size);

	size_t packedSize = pb_message__pack(&message, serializedData);


	FILE* file = fopen("dump.bin", "wb");
	if (!file)
	{
		fprintf(stderr, "could not open file for writing\r\n");
		return -1;
	}

	fwrite(serializedData, size, 1, file);
	fclose(file);
	file = NULL;

	// Create new socket connection



	// Free the serialized buffer
	/*free(buf);
	buf = NULL;*/

	return 0;
}