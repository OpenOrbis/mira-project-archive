#define _CRT_SECURE_NO_WARNINGS

#include "mirabuiltin.pb-c.h"
#include <stdlib.h>
#include <stdio.h>

#define PAGE_SIZE 0x4000


int main()
{
	MessageHeader header = MESSAGE_HEADER__INIT;

	header.category = MESSAGE_CATEGORY__CMD;
	header.error = 0;
	header.type = 1337;

	size_t size = message_header__get_packed_size(&header);

	uint8_t* buf = malloc(size);
	if (!buf)
	{
		fprintf(stderr, "could not allocate buffer size (%llx)\r\n", size);
		return -1;
	}
	message_header__pack(&header, buf);

	FILE* file = fopen("dump.bin", "wb");
	if (!file)
	{
		fprintf(stderr, "could not open file for writing\r\n");
		return -1;
	}

	fwrite(buf, size, 1, file);
	fclose(file);
	file = NULL;

	// Create new socket connection



	// Free the serialized buffer
	free(buf);
	buf = NULL;

	return 0;
}