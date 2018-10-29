#include "loader.h"
#include <sys/mman.h>
#include <machine/param.h>

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef NULL
#define NULL 0
#endif

int32_t elfloader_roundUp(int32_t number, int32_t multiple)
{
	if (multiple == 0)
		return number;

	int32_t remainder = number % multiple;
	if (remainder == 0)
		return number;

	return number + multiple - remainder;
}

boolean_t elfloader_loadFromUserMemory(struct elfloader_t* loader, uint8_t* data, uint64_t dataSize)
{
	if (!loader)
		return false;

	if (!data || dataSize == 0)
		return false;

	// Some basic sanity checking
	if (dataSize > __INT32_MAX__)
		return false;

	// Calculate the size of mmap call
	int32_t elfDataPageSize = elfloader_roundUp(dataSize, PAGE_SIZE);

	// Call mmap with RW permissions
	caddr_t elfData = syscall6(477, NULL, elfDataPageSize, PROT_READ | PROT_WRITE, MAP_ANON, -1, 0);

	// Check if there was an error mmaping the data
	if (elfData == MAP_FAILED)
		return false;

	// TODO: Copy over the incoming data to our new buffer

	return elfloader_internal_initFromMemory(loader, elfData, elfDataPageSize);
}

boolean_t elfloader_internal_initFromMemory(struct elfloader_t* loader, uint8_t* data, uint64_t dataSize)
{
	// Assign our variables so we don't lose them later
	loader->data = data;
	loader->dataSize = dataSize;

	// Validate the basics of the elf header
	if (!elfloader_hasValidData(loader))
		return false;

	uint8_t* dataStart = loader->data;
	Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)loader->data;

	// Iterate through all of the program headers
	Elf64_Half programHeaderCount = elfHeader->e_phnum;
	for (Elf64_Half programHeaderIndex = 0; programHeaderIndex < programHeaderCount; ++programHeaderIndex)
	{
		Elf64_Phdr* programHeader = elfloader_getProgramHeaderByIndex(loader, programHeaderIndex);

		// Verify that we got a valid program header
		if (!programHeader)
			continue;

		switch (programHeader->p_type)
		{
			// Ignored entries
		case PT_NULL:
		case PT_SHLIB:
			break;

			// Save the interperter string
		case PT_INTERP:
			if (!elfloader_parseInterpreter(loader, programHeader))
			{
				// TODO: Error
				return false;
			}
			break;


			// Unimplemented types
		case PT_LOAD:
		case PT_DYNAMIC:
		case PT_NOTE:
		case PT_PHDR:
		case PT_LOPROC:
		case PT_HIPROC:
		case PT_GNU_STACK:
			
			break;
		}
	}
}

boolean_t elfloader_parseInterpreter(struct elfloader_t* loader, Elf64_Phdr* interpreterHeader)
{
	if (!loader || !interpreterHeader)
		return false;

	// Validate that the offset is within bounds
	if (interpreterHeader->p_offset > loader->dataSize)
		return false;

	uint8_t* dataStart = loader->data;

	// Save the reference to the interpreter string
	loader->interpreterString = dataStart + interpreterHeader->p_offset;

	return true;
}

boolean_t elfloader_parseDynamic(struct elfloader_t* loader, Elf64_Phdr* dynamicHeader)
{
	if (!loader || !dynamicHeader)
		return false;

	// Validate that the offset is within bounds
	if (dynamicHeader->p_offset > loader->dataSize)
		return false;

	uint8_t* dataStart = loader->data;


}

uint8_t* elfloader_getData(struct elfloader_t* loader)
{
	if (!loader)
		return NULL;

	return loader->data;
}

Elf64_Ehdr* elfloader_getElfHeader(struct elfloader_t* loader)
{
	if (!loader)
		return NULL;

	if (!loader->data || loader->dataSize == 0)
		return NULL;

	return (Elf64_Ehdr*)loader->data;
}

Elf64_Phdr* elfloader_getProgramHeaderByIndex(struct elfloader_t* loader, int32_t index)
{
	if (!loader)
		return NULL;

	uint8_t* dataStart = loader->data;
	Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)loader->data;

	Elf64_Half programHeaderCount = elfHeader->e_phnum;
	if (index >= programHeaderCount)
		return NULL;

	return (Elf64_Phdr*)((dataStart + elfHeader->e_phoff) + (sizeof(Elf64_Phdr) * index));
}

Elf64_Shdr* elfloader_getSectionHeaderByIndex(struct elfloader_t* loader, int32_t index)
{
	if (!loader)
		return NULL;

	uint8_t* dataStart = loader->data;
	Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)dataStart;

	Elf64_Half sectionHeaderCount = elfHeader->e_shnum;
	if (index >= sectionHeaderCount)
		return NULL;

	return (Elf64_Shdr*)((dataStart + elfHeader->e_shoff) + (sizeof(Elf64_Shdr) * index));
}

boolean_t elfloader_hasValidData(struct elfloader_t* loader)
{
	if (!loader)
		return false;

	if (!loader->data || loader->dataSize == 0)
		return false;

	Elf64_Ehdr* header = (Elf64_Ehdr*)loader->data;
	
	// Validate the ELF header
	if (header->e_ident[EI_MAG0] != ELFMAG0 ||
		header->e_ident[EI_MAG1] != ELFMAG1 ||
		header->e_ident[EI_MAG2] != ELFMAG2 ||
		header->e_ident[EI_MAG3] != ELFMAG3)
		return false;

	// Validate the type is executable or relocatable
	if (header->e_type != ET_EXEC &&
		header->e_type != ET_REL)
		return false;

	// Only accept X64 binaries
	if (header->e_machine != EM_X86_64)
		return false;

	// Good nuff'
	return true;
}