#pragma once
#include <oni/utils/types.h>
#include <sys/elf64.h>

Elf64_Phdr* elfutils_getProgramHeaderByIndex(uint8_t* elfStart, size_t elfSize, uint64_t index);

Elf64_Phdr* elfutils_getProgramHeaderByType(uint8_t* elfStart, size_t elfSize, uint32_t headerType);

Elf64_Shdr* elfutils_getSectionHeaderByIndex(uint8_t* elfStart, size_t elfSize, uint64_t index);

Elf64_Shdr* elfutils_getSectionHeaderByType(uint8_t* elfStart, size_t elfSize, uint32_t headerType);

Elf64_Shdr* elfutils_getSectionHeaderByName(uint8_t* elfStart, size_t elfSize, const char* name);

const char* elfutils_getStringTableSectionData(uint8_t* elfStart, size_t elfSize, uint64_t* outStringTableSize);

Elf64_Addr elfutils_getEntryPoint(uint8_t* elfStart, size_t elfSize);