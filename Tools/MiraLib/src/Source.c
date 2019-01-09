#define _CRT_SECURE_NO_WARNINGS

#include "protobuf-c.h"
#include "fileexplorer.pb-c.h"
#include "mirabuiltin.pb-c.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vector.h"

#define PAGE_SIZE 0x4000


int main()
{
	PbMessage message = PB_MESSAGE__INIT;

	const uint32_t dentCount = 33;

	//DirEnt** dents = malloc(sizeof(DirEnt*) * dentCount);
	//if (!dents)
	//	return -1;

	Vector list;
	vector_initialize(&list, sizeof(DirEnt*));

	for (uint32_t i = 0; i < dentCount; ++i)
	{
		DirEnt* dent = malloc(sizeof(DirEnt));
		dir_ent__init(dent);
		dent->fileno = i;
		dent->name = malloc(256);
		sprintf(dent->name, "file %d.bin", i);
		dent->reclen = i;
		dent->type = i;

		vector_append(&list, dent);
	}
	
	GetDentsResponse response = GET_DENTS_RESPONSE__INIT;

	get_dents_response__init(&response);
	response.error = 0;
	response.entries = list.data;
	response.n_entries = list.size;
	

	size_t size2 = get_dents_response__get_packed_size(&response);

	message.payload.data = (uint8_t*)malloc(size2);
	message.payload.len = size2;

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