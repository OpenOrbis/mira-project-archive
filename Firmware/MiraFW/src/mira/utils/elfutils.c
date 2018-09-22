#include "elfutils.h"
#include <oni/utils/logger.h>
#include <oni/utils/kdlsym.h>

#define PT_MIRAHEADER	0x70000001

static uint8_t elfutils_verifyHeader(uint8_t* elfData, size_t elfSize)
{
	int(*memcmp)(const void* s1, const void* s2, size_t n) = kdlsym(memcmp);

	if (!elfData || elfSize == 0)
	{
		WriteLog(LL_Error, "data or size is invalid");
		return false;
	}

	if (elfSize < sizeof(Elf64_Ehdr))
	{
		WriteLog(LL_Error, "elf size is too small have (%llx) want (%llx).", elfSize, sizeof(Elf64_Ehdr));
		return false;
	}

	Elf64_Ehdr* header = (Elf64_Ehdr*)elfData;

	// Verify the elf header
	if (memcmp(header->e_ident, ELFMAG, SELFMAG))
	{
		WriteLog(LL_Error, "elf header check failed");
		return false;
	}

	// Verify the elf type
	if (header->e_type != ET_EXEC || header->e_machine != EM_X86_64)
	{
		WriteLog(LL_Error, "elf not an executable, or not compiled for x86_64");
		return false;
	}

	return true;
}

Elf64_Phdr* elfutils_getProgramHeaderByIndex(uint8_t* elfStart, size_t elfSize, uint64_t index)
{
	// Validate the header
	if (!elfutils_verifyHeader(elfStart, elfSize))
		return NULL;

	Elf64_Ehdr* header = (Elf64_Ehdr*)elfStart;

	// Iterate through each of the elf segments to find the one that we want
	Elf64_Half programHeaderCount = header->e_phnum;
	Elf64_Half programHeaderStart = header->e_phoff;

	// TODO: More bounds checking

	if (index > programHeaderCount)
		return NULL;

	return (Elf64_Phdr*)((elfStart + programHeaderStart) + (index * sizeof(Elf64_Phdr)));
}

Elf64_Phdr* elfutils_getProgramHeaderByType(uint8_t* elfStart, size_t elfSize, uint32_t headerType)
{
	// Validate the header
	if (!elfutils_verifyHeader(elfStart, elfSize))
		return NULL;

	Elf64_Ehdr* header = (Elf64_Ehdr*)elfStart;

	// Iterate through each of the elf segments to find the one that we want
	Elf64_Half programHeaderCount = header->e_phnum;
	Elf64_Half programHeaderStart = header->e_phoff;

	for (size_t headerIndex = 0; headerIndex < programHeaderCount; ++headerIndex)
	{
		// TODO: More bounds checking

		Elf64_Phdr* programHeader = (Elf64_Phdr*)((elfStart + programHeaderStart) + (headerIndex * sizeof(Elf64_Phdr)));
		if (!programHeader)
			continue;

		// If the header type matches return
		if (programHeader->p_type == headerType)
			return programHeader;
	}

	return NULL;
}

Elf64_Shdr* elfutils_getSectionHeaderByIndex(uint8_t* elfStart, size_t elfSize, uint64_t index)
{
	// Validate the header
	if (!elfutils_verifyHeader(elfStart, elfSize))
		return NULL;

	Elf64_Ehdr* header = (Elf64_Ehdr*)elfStart;

	Elf64_Half sectionHeaderCount = header->e_shnum;
	Elf64_Half sectionHeaderStart = header->e_shoff;

	if (index >= sectionHeaderCount)
		return NULL;

	// TODO: More bounds checking

	return (Elf64_Shdr*)((elfStart + sectionHeaderStart) + (index * sizeof(Elf64_Shdr)));
}

Elf64_Shdr* elfutils_getSectionHeaderByType(uint8_t* elfStart, size_t elfSize, uint32_t headerType)
{
	// Validate the header
	if (!elfutils_verifyHeader(elfStart, elfSize))
		return NULL;

	Elf64_Ehdr* header = (Elf64_Ehdr*)elfStart;

	Elf64_Half sectionHeaderCount = header->e_shnum;
	Elf64_Half sectionHeaderStart = header->e_shoff;

	for (size_t headerIndex = 0; headerIndex < sectionHeaderCount; ++headerIndex)
	{
		// TODO: More bounds checking before we deref

		Elf64_Shdr* sectionHeader = (Elf64_Shdr*)((elfStart + sectionHeaderStart) + (headerIndex * sizeof(Elf64_Shdr)));

		if (!sectionHeader)
			continue;

		if (sectionHeader->sh_type == headerType)
			return sectionHeader;
	}
	return NULL;
}

Elf64_Shdr* elfutils_getSectionHeaderByName(uint8_t* elfStart, size_t elfSize, const char* name)
{
	int(*strncmp)(const char* str1, const char* str2, size_t n) = kdlsym(strncmp);
	size_t(*strlen)(const char* str) = kdlsym(strlen);

	// Validate the header
	if (!elfutils_verifyHeader(elfStart, elfSize))
		return NULL;

	Elf64_Ehdr* header = (Elf64_Ehdr*)elfStart;

	Elf64_Half sectionHeaderCount = header->e_shnum;
	Elf64_Half sectionHeaderStart = header->e_shoff;

	uint64_t stringTableSize = 0;
	const char* stringTable =  elfutils_getStringTableSectionData(elfStart, elfSize, &stringTableSize);
	if (!stringTable || stringTableSize == 0)
	{
		WriteLog(LL_Error, "could not get the string table");
		return NULL;
	}

	for (size_t headerIndex = 0; headerIndex < sectionHeaderCount; ++headerIndex)
	{
		// TODO: More bounds checking before we deref

		Elf64_Shdr* sectionHeader = (Elf64_Shdr*)((elfStart + sectionHeaderStart) + (headerIndex * sizeof(Elf64_Shdr)));

		if (!sectionHeader)
			continue;

		if (sectionHeader->sh_type != SHT_PROGBITS)
			continue;

		uint64_t offset = (uint64_t)sectionHeader->sh_type;
		if (offset > elfSize || offset < (uint64_t)elfStart)
			continue;

		const char* sectionName = stringTable + offset;

		int32_t nameComparison = strncmp(sectionName, name, strlen(name));
		if (nameComparison != 0)
			continue;

		return sectionHeader;
	}

	return NULL;
}

const char* elfutils_getStringTableSectionData(uint8_t* elfStart, size_t elfSize, uint64_t* outStringTableSize)
{
	if (outStringTableSize)
		*outStringTableSize = 0;

	if (!elfutils_verifyHeader(elfStart, elfSize))
		return NULL;

	Elf64_Ehdr* header = (Elf64_Ehdr*)elfStart;

	Elf64_Half sectionHeaderCount = header->e_shnum;
	Elf64_Half sectionHeaderStart = header->e_shoff;

	for (size_t headerIndex = 0; headerIndex < sectionHeaderCount; ++headerIndex)
	{
		// TODO: More bounds checking before we deref

		Elf64_Shdr* sectionHeader = (Elf64_Shdr*)((elfStart + sectionHeaderStart) + (headerIndex * sizeof(Elf64_Shdr)));

		if (!sectionHeader)
			continue;

		if (sectionHeader->sh_type != SHT_STRTAB)
			continue;

		uint64_t offset = (uint64_t)sectionHeader->sh_type;
		if (offset > elfSize || offset < (uint64_t)elfStart)
			continue;

		if (outStringTableSize)
			*outStringTableSize = sectionHeader->sh_size;

		return (const char*)(elfStart + sectionHeader->sh_offset);
	}

	return NULL;
}

Elf64_Addr elfutils_getEntryPoint(uint8_t* elfData, size_t elfSize)
{
	// Validate the header
	if (!elfutils_verifyHeader(elfData, elfSize))
		return NULL;

	// Get the header
	Elf64_Ehdr* header = (Elf64_Ehdr*)elfData;

	// Return the entry point
	return header->e_entry;
}