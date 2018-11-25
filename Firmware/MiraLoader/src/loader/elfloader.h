#pragma once
#ifndef _WIN32
#include <oni/utils/types.h>
#include <sys/elf64.h>
#else
#include <stdint.h>
#include "elf64.h"
#endif

#define ALLOC_3MB	0x300000
#define ALLOC_5MB	0x500000

typedef struct _ElfLoader_t
{
	uint8_t* data;
	uint64_t dataSize;

	// Information to be filled out by the loader
	uint64_t elfSize;
	char* interpreter;

	uint8_t isKernel;

	void(*elfMain)();
} ElfLoader_t;

uint8_t elfloader_initFromFile(ElfLoader_t* loader, const char* filePath);
uint8_t elfloader_initFromMemory(ElfLoader_t* loader, uint8_t* data, uint64_t dataLength);

Elf64_Phdr* elfloader_getProgramHeaderByIndex(ElfLoader_t* loader, int32_t index);

Elf64_Shdr* elfloader_getSectionHeaderByIndex(ElfLoader_t* loader, int32_t index);
Elf64_Shdr* elfloader_getSectionHeaderByName(ElfLoader_t* loader, const char* name);

Elf64_Sym* elfloader_getSymbolByIndex(ElfLoader_t* loader, int32_t index);

uint8_t elfloader_internalGetStringTable(ElfLoader_t* loader, const char** outStringTable, uint64_t* outStringTableSize);

uint8_t elfloader_setProtection(ElfLoader_t* loader, uint8_t* data, uint64_t dataSize, int32_t protection);

uint8_t elfloader_handleRelocations(ElfLoader_t* loader);

uint8_t elfloader_updateElfProtections(ElfLoader_t* loader);

uint8_t elfloader_isElfValid(ElfLoader_t* loader);

void elfloader_memcpy(ElfLoader_t* loader, void* dst, void* src, size_t cnt);
void elfloader_memset(ElfLoader_t* loader, void* address, int32_t val, size_t len);
int32_t elfloader_strcmp(const char *s1, const char *s2);

void* elfloader_allocate(ElfLoader_t* loader, uint64_t size);