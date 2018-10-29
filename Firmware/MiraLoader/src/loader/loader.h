#pragma once
#include <sys/types.h>
#include <sys/elf64.h>

struct elfloader_t
{
	// Allocated data buffer to hold the elf file, must be freed
	uint8_t* data;
	uint64_t dataSize;

	// These fields get filled out on load
	const char* interpreterString;
};

boolean_t elfloader_initFromFile(struct elfloader_t* loader, const char* filePath);
boolean_t elfloader_loadFromUserMemory(struct elfloader_t* loader, uint8_t* data, uint64_t dataSize);
boolean_t elfloader_loadFromKernelMemory(struct elfloader_t* loader, uint8_t* data, uint64_t dataSize);

boolean_t elfloader_internal_initFromMemory(struct elfloader_t* loader, uint8_t* data, uint64_t dataSize);

uint8_t* elfloader_getData(struct elfloader_t* loader);
Elf64_Ehdr* elfloader_getElfHeader(struct elfloader_t* loader);
Elf64_Phdr* elfloader_getProgramHeaderByIndex(struct elfloader_t* loader, int32_t index);
Elf64_Shdr* elfloader_getSectionHeaderByIndex(struct elfloader_t* loader, int32_t index);

boolean_t elfloader_hasValidData(struct elfloader_t* loader);

// Handles signature verification
boolean_t elfloader_isElfMiraSigned(struct elfloader_t* loader);

// 

boolean_t elfloader_parseInterpreter(struct elfloader_t* loader, Elf64_Phdr* interpreterHeader);

boolean_t elfloader_parseDynamic(struct elfloader_t* loader, Elf64_Phdr* dynamicHeader);