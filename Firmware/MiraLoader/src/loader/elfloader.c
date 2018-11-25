#include "elfloader.h"

#ifdef _WIN32
#include <Windows.h>
#include <stdio.h>

#define PROT_READ	0x1     /* Page can be read.  */
#define PROT_WRITE	0x2     /* Page can be written.  */
#define PROT_EXEC	0x4     /* Page can be executed.  */
#define PROT_NONE	0x0     /* Page can not be accessed.  */

#define PAGE_SIZE 0x4000
#define	MAP_ANON	 0x1000	/* allocated from memory, swap space */
#define	MAP_PRIVATE	0x0002		/* changes are private */
typedef uint8_t* caddr_t;
typedef uint64_t off_t;

enum LogLevels
{
	LL_None,
	LL_Info,
	LL_Warn,
	LL_Error,
	LL_Debug,
	LL_All
};

#define WriteLog(x, y, ...)
#define WriteNotificationLog(x)

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#else
#include <sys/param.h>
#include <sys/elf64.h>
#include <oni/utils/syscall.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/malloc.h>

#include <oni/utils/kdlsym.h>
#include <oni/utils/logger.h>
#include <utils/notify.h>
#endif

//
//	Utility Functions
//


#ifdef _WIN32
void* _Allocate5MB()
{
	void* data = malloc(ALLOC_5MB);
	if (!data)
		return NULL;

	DWORD oldProtect = 0;
	VirtualProtect(data, ALLOC_5MB, PAGE_EXECUTE_READWRITE, &oldProtect);

	return data;
}

caddr_t _mmap(caddr_t addr, size_t len, int prot, int flags, int fd, off_t pos)
{
	void* data = malloc(len);
	if (!data)
		return NULL;

	DWORD oldProtect = 0;
	VirtualProtect(data, len, PAGE_EXECUTE_READWRITE, &oldProtect);

	return data;
}
#endif

uint64_t elfloader_roundUp(uint64_t number, uint64_t multiple)
{
	if (multiple == 0)
		return number;

	uint64_t remainder = number % multiple;
	if (remainder == 0)
		return number;

	return number + multiple - remainder;
}

uint8_t elfloader_initFromFile(ElfLoader_t* loader, const char* filePath)
{
	if (!loader)
		return false;

	if (!filePath)
		return false;

	loader->data = NULL;
	loader->dataSize = 0;

	loader->elfSize = 0;
	loader->interpreter = NULL;
	loader->elfMain = NULL;

#ifdef _WIN32
	FILE* file = NULL;
	errno_t error = fopen_s(&file, filePath, "rb");
	if (error)
		return false;

	fseek(file, 0, SEEK_END);
	uint64_t elfLength = ftell(file);
	fseek(file, 0, SEEK_SET);

	uint64_t allocationSize = elfloader_roundUp(elfLength, 0x4000);
	uint8_t* allocationData = malloc(allocationSize);
	if (!allocationData)
	{
		fclose(file);
		file = NULL;

		return false;
	}

	// Zero the memory allocation
	memset(allocationData, 0, allocationSize);

	size_t sizeRead = fread_s(allocationData, allocationSize, sizeof(uint8_t), elfLength, file);
	if (sizeRead != elfLength)
	{
		free(allocationData);
		allocationData = NULL;

		fclose(file);
		file = NULL;

		return false;
	}

	// Close the file handle, don't free the memory
	fclose(file);
	file = NULL;

	// Set the information in structure
	loader->data = allocationData;
	loader->elfSize = elfLength;
	loader->dataSize = allocationSize;


#endif


	return true;
}

uint8_t elfloader_initFromMemory(ElfLoader_t* loader, uint8_t* data, uint64_t dataLength)
{
	if (!loader || !data || dataLength == 0)
		return false;

	loader->data = NULL;
	loader->dataSize = 0;

	loader->elfMain = NULL;
	loader->elfSize = 0;
	loader->interpreter = NULL;

	// This is slightly different, we aren't in kernel mode so we need to 

	// Get the elf size
	size_t elfSize = dataLength;

	// Round up to the nearest page size
	uint64_t allocationSize = elfloader_roundUp(elfSize, PAGE_SIZE);

	if (loader->isKernel)
		WriteLog(LL_Debug, "allocationSize: %llx\n", allocationSize);

	// Allocate RWX data
	caddr_t allocationData = NULL;

	// Allocate some memory
	allocationData = (caddr_t)elfloader_allocate(loader, allocationSize);

	// Validate that we got the allocation data we wanted
	if (!allocationData)
		return false;

	// Zero out the allocaiton
	elfloader_memset(loader, allocationData, 0, allocationSize);

	// Copy over the elf data
	elfloader_memcpy(loader, (uint8_t*)allocationData, data, dataLength);

	loader->data = (uint8_t*)allocationData;
	loader->dataSize = allocationSize;
	loader->elfSize = elfSize;

	return true;
}


Elf64_Phdr* elfloader_getProgramHeaderByIndex(ElfLoader_t* loader, int32_t index)
{
	// Validate the loader
	if (!loader)
		return NULL;

	// Validate the data
	if (!loader->data || loader->dataSize == 0)
		return NULL;

	// Get the start of the entire elf
	uint8_t* dataStart = loader->data;

	// Get the elf header
	Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)loader->data;

	// Get the count of program headers
	Elf64_Half programHeaderCount = elfHeader->e_phnum;

	// Validate the index bounds
	if (index < 0 || index >= programHeaderCount)
		return NULL;

	// Bounds check the program header offset
	if (elfHeader->e_phoff >= loader->dataSize)
		return NULL;

	Elf64_Phdr* programHeader = (Elf64_Phdr*)((dataStart + elfHeader->e_phoff) + (sizeof(Elf64_Phdr) * index));

	if (programHeader->p_offset >= loader->dataSize)
		return NULL;

	// Return happy
	return programHeader;
}

Elf64_Shdr* elfloader_getSectionHeaderByIndex(ElfLoader_t* loader, int32_t index)
{
	// Validate the loader
	if (!loader)
		return NULL;

	// Validate the data
	if (!loader->data || loader->dataSize == 0)
		return NULL;

	// Get the start of the entire elf
	uint8_t* dataStart = loader->data;

	// Get the elf header
	Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)dataStart;

	// Get the total count of section headers
	Elf64_Half sectionHeaderCount = elfHeader->e_shnum;

	// Validate the bounds of our section headers
	if (index < 0 || index >= sectionHeaderCount)
		return NULL;

	// Bounds check the section header offset
	if (elfHeader->e_shoff >= loader->dataSize)
		return NULL;

	Elf64_Shdr* sectionHeader = (Elf64_Shdr*)((dataStart + elfHeader->e_shoff) + (sizeof(Elf64_Shdr) * index));

	// Validate that the offset is within bounds
	if (sectionHeader->sh_offset >= loader->dataSize)
		return NULL;

	// Return the address section header
	return sectionHeader;
}

Elf64_Shdr* elfloader_getSectionHeaderByName(ElfLoader_t* loader, const char* name)
{
	if (!loader)
		return NULL;

	if (!loader->data || loader->dataSize == 0)
		return NULL;

	if (!name)
		return NULL;

	Elf64_Ehdr* header = (Elf64_Ehdr*)loader->data;

	// Get the string table
	const char* stringTable = NULL;
	uint64_t stringTableSize = 0;

	uint8_t hasStringTable = elfloader_internalGetStringTable(loader, &stringTable, &stringTableSize);

	// Check that we have a string table
	if (!hasStringTable || !stringTable || stringTableSize == 0)
		return NULL;

	Elf64_Half headerCount = header->e_shnum;
	for (Elf64_Half headerIndex = 0; headerIndex < headerCount; ++headerIndex)
	{
		Elf64_Shdr* sectionHeader = elfloader_getSectionHeaderByIndex(loader, headerIndex);
		if (!sectionHeader)
			continue;

		// Bounds check the section header name offset
		if (sectionHeader->sh_name >= loader->dataSize)
			continue;

		// Verify that it is within bounds of the string table size
		if (sectionHeader->sh_name >= stringTableSize)
			continue;

		// This is index into string table
		const char* sectionName = stringTable + sectionHeader->sh_name;

		// Compare
		if (elfloader_strcmp(name, sectionName) != 0)
			continue;

		// We have a match

		// Verify that the offset is within bounds
		if (sectionHeader->sh_offset >= loader->dataSize)
			continue;

		return sectionHeader;
	}

	return NULL;
}

Elf64_Sym* elfloader_getSymbolByIndex(ElfLoader_t* loader, int32_t index)
{
	if (!loader)
		return NULL;

	if (!loader->data || loader->dataSize == 0)
		return NULL;

	Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)loader->data;

	// Iterate through each section
	Elf64_Half sectionCount = elfHeader->e_shnum;
	for (Elf64_Half sectionIndex = 0; sectionIndex < sectionCount; ++sectionIndex)
	{
		// Get the section header by index
		Elf64_Shdr* sectionHeader = elfloader_getSectionHeaderByIndex(loader, sectionIndex);
		if (!sectionHeader)
			continue;

		// We ignore everything execpt for symtab
		if (sectionHeader->sh_type != SHT_SYMTAB)
			continue;

		// Verify that we are within bounds
		if (sectionHeader->sh_offset >= loader->dataSize)
			continue;

		// Get the symbol count
		uint64_t symbolCount = sectionHeader->sh_size / sectionHeader->sh_entsize;

		// Bounds check the symbol index
		if (index < 0 || index >= symbolCount)
			return NULL;

		for (uint64_t symbolIndex = 0; symbolIndex < symbolCount; ++symbolIndex)
		{
			Elf64_Sym* symbolHeader = (Elf64_Sym*)(loader->data + sectionHeader->sh_offset + (sectionHeader->sh_entsize * symbolIndex));

			if (symbolIndex == index)
				return symbolHeader;
		}
	}

	return NULL;
}

uint8_t elfloader_internalGetStringTable(ElfLoader_t* loader, const char** outStringTable, uint64_t* outStringTableSize)
{
	if (!loader)
		return false;

	if (!loader->data || loader->dataSize == 0)
		return false;

	if (outStringTable == NULL)
		return false;

	*outStringTable = NULL;

	if (outStringTableSize)
		*outStringTableSize = 0;

	Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)loader->data;

	// Bounds check the section header offset
	if (elfHeader->e_shoff >= loader->dataSize)
	{
		if (loader->isKernel)
			WriteLog(LL_Debug, "section header is outside of bounds have (%llx) want <= (%llx)", elfHeader->e_shoff, loader->dataSize);
		else
			WriteNotificationLog("section header is outside of bounds");

		return false;
	}

	Elf64_Half sectionHeaderCount = elfHeader->e_shnum;
	for (Elf64_Half index = 0; index < sectionHeaderCount; index++)
	{
		Elf64_Shdr* sectionHeader = elfloader_getSectionHeaderByIndex(loader, index);
		if (!sectionHeader)
			continue;

		// We only want the string table offset
		if (sectionHeader->sh_type != SHT_STRTAB)
			continue;

		// Bounds check the section offset
		if (sectionHeader->sh_offset >= loader->dataSize)
			continue;

		// Set the output variable if it's set
		if (outStringTableSize)
			*outStringTableSize = sectionHeader->sh_size;

		*outStringTable = (const char*)(loader->data + sectionHeader->sh_offset);
		return true;
	}

	return false;
}

uint8_t elfloader_internalGetSymbolTable(ElfLoader_t* loader, uint8_t** outStringTable, uint64_t* outStringTableSize)
{
	if (!loader)
		return false;

	if (!loader->data || loader->dataSize == 0)
		return false;

	if (outStringTable == NULL)
		return false;

	*outStringTable = NULL;

	if (outStringTableSize)
		*outStringTableSize = 0;

	Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)loader->data;

	// Bounds check the section header offset
	if (elfHeader->e_shoff >= loader->dataSize)
	{
		if (loader->isKernel)
			WriteLog(LL_Debug, "section header is outside of bounds have (%llx) want <= (%llx)", elfHeader->e_shoff, loader->dataSize);
		else
			WriteNotificationLog("section header is outside of bounds");

		return false;
	}

	Elf64_Half sectionHeaderCount = elfHeader->e_shnum;
	for (Elf64_Half index = 0; index < sectionHeaderCount; index++)
	{
		Elf64_Shdr* sectionHeader = elfloader_getSectionHeaderByIndex(loader, index);
		if (!sectionHeader)
			continue;

		// We only want the string table offset
		if (sectionHeader->sh_type != SHT_SYMTAB)
			continue;

		// Bounds check the section offset
		if (sectionHeader->sh_offset >= loader->dataSize)
			continue;

		// Set the output variable if it's set
		if (outStringTableSize)
			*outStringTableSize = sectionHeader->sh_size;

		*outStringTable = (uint8_t*)(loader->data + sectionHeader->sh_offset);
		return true;
	}

	return false;
}

uint8_t elfloader_setProtection(ElfLoader_t* loader, uint8_t* data, uint64_t dataSize, int32_t protection)
{
	// Validate loader
	if (!loader)
		return false;

	// Validate data and size
	if (!data || dataSize == 0)
		return false;

#ifdef _WIN32
	DWORD oldProtection = 0;
	if (!VirtualProtect(data, dataSize, protection, &oldProtection))
		return false;
#else
	// TODO: use mprotect or some shit here
#endif

	return true;
}

uint8_t do_everything(ElfLoader_t* loader)
{
	if (!loader)
		return false;

	if (!loader->data)
		return false;

	// Update all of the headers virtual addresses
	Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)loader->data;
	Elf64_Half sectionHeaderCount = elfHeader->e_shnum;
	for (Elf64_Half sectionHeaderIndex = 0; sectionHeaderIndex < sectionHeaderCount; ++sectionHeaderIndex)
	{
		Elf64_Shdr* sectionHeader = elfloader_getSectionHeaderByIndex(loader, sectionHeaderIndex);
		if (!sectionHeader)
			continue;

		uint64_t sectionSize = sectionHeader->sh_size;

		// Verify that we are allocation and that the size is valid
		if ((sectionHeader->sh_flags & SHF_ALLOC) && sectionHeader->sh_size > 0)
		{
			// Allocate data
			uint8_t* sectionData = (uint8_t*)elfloader_allocate(loader, sectionSize);
			if (!sectionData)
			{
				if (loader->isKernel)
					WriteLog(LL_Error, "could not allocate section data.");

				return false;
			}

			if (sectionHeader->sh_type == SHT_PROGBITS)
			{
				elfloader_memset(loader, sectionData, 0, sectionSize);
				elfloader_memcpy(loader, sectionData, (loader->data + sectionHeader->sh_offset), sectionSize);

			}
			else if (sectionHeader->sh_type == SHT_NOBITS)
			{
				// Section is empty, fill with zeros
				elfloader_memset(loader, sectionData, 0, sectionSize);
			}

			sectionHeader->sh_addr = (Elf64_Addr)sectionData;
		}

		// Load symbol and string tables from the file
		if (sectionHeader->sh_type == SHT_SYMTAB || sectionHeader->sh_type == SHT_STRTAB)
		{
			Elf64_Sym* table = (Elf64_Sym*)elfloader_allocate(loader, sectionSize);
			if (!table)
			{
				WriteNotificationLog("could not allocate table memory");
				return false;
			}

			// Zero and copy out the section
			elfloader_memset(loader, table, 0, sectionSize);
			elfloader_memcpy(loader, (uint8_t*)table, (loader->data + sectionHeader->sh_offset), sectionSize);

			// Update the section header address
			sectionHeader->sh_addr = (Elf64_Addr)table;
		}
	}

	Elf64_Half programHeaderCount = elfHeader->e_phnum;
	for (Elf64_Half programHeaderIndex = 0; programHeaderIndex < programHeaderCount; ++programHeaderIndex)
	{
		Elf64_Phdr* programHeader = elfloader_getProgramHeaderByIndex(loader, programHeaderIndex);
		if (!programHeader)
			continue;

		programHeader->p_vaddr = (Elf64_Addr)(loader->data + programHeader->p_offset);
		programHeader->p_paddr = (Elf64_Addr)NULL;
	}

	// User update
	if (loader->isKernel)
		WriteLog(LL_Debug, "applying symbol relocations");
	else
		WriteNotificationLog("applying symbol relocations");

	for (Elf64_Half sectionHeaderIndex = 0; sectionHeaderIndex < sectionHeaderCount; ++sectionHeaderIndex)
	{
		Elf64_Shdr* sectionHeader = elfloader_getSectionHeaderByIndex(loader, sectionHeaderIndex);
		if (!sectionHeader)
			continue;

		if (sectionHeader->sh_type != SHT_RELA)
			continue;

		Elf64_Word infoSectionIndex = sectionHeader->sh_info;

		if (infoSectionIndex >= sectionHeaderCount)
			continue;

		Elf64_Word linkSectionIndex = sectionHeader->sh_link;
		if (linkSectionIndex >= sectionHeaderCount)
			continue;

		// Validate that the Elf64_Rela size is the same
		uint64_t entrySize = sectionHeader->sh_entsize;
		if (entrySize != sizeof(Elf64_Rela))
		{
			if (loader->isKernel)
				WriteLog(LL_Error, "Elf64_Rela entry size dont match");
			else
				WriteNotificationLog("Elf64_Rela entry size dont match");
			continue;
		}

		uint64_t sectionSize = sectionHeader->sh_size;
		uint64_t entryCount = entrySize / sectionSize;

		// Validate that we got valid memory
		Elf64_Rela* entries = (Elf64_Rela*)elfloader_allocate(loader, sectionSize);
		if (!entries)
		{
			if (loader->isKernel)
				WriteLog(LL_Error, "could not allocate entry memory");
			else
				WriteNotificationLog("could not allocate entry memory");

			return false;
		}

		sectionHeader->sh_addr = (Elf64_Addr)entries;

		elfloader_memset(loader, entries, 0, sectionSize);
		elfloader_memcpy(loader, entries, (loader->data + sectionHeader->sh_offset), sectionSize);

		Elf64_Shdr* relocationSection = elfloader_getSectionHeaderByIndex(loader, infoSectionIndex);
		if (!relocationSection)
		{
			if (loader->isKernel)
				WriteLog(LL_Error, "could not find relocation section");
			else
				WriteNotificationLog("could not find relocation section");

			return false;
		}

		uint8_t* relocationSectionData = (uint8_t*)relocationSection->sh_addr;
		if (!relocationSectionData)
			return false;

		Elf64_Shdr* symbolTableSection = elfloader_getSectionHeaderByIndex(loader, infoSectionIndex);
		if (!symbolTableSection)
			return false;

		Elf64_Sym* symbolTable = (Elf64_Sym*)(symbolTableSection->sh_addr);

		Elf64_Shdr* stringTableSection = elfloader_getSectionHeaderByIndex(loader, linkSectionIndex);
		if (!stringTableSection)
			return false;

		const char* stringTable = (const char*)(stringTableSection->sh_addr);

		// Relocate all entries
		for (uint64_t entryIndex = 0; entryIndex < entryCount; ++entryIndex)
		{
			Elf64_Rela* entry = (entries + entryIndex);

			// Find the symbol for this current entry
			int32_t symbolIndex = ELF64_R_SYM(entry->r_info);
			int32_t symbolType = ELF64_R_TYPE(entry->r_info);

			// TODO: Bounds check symbol index and type
			Elf64_Sym* symbol = (symbolTable + symbolIndex);
			const char* symbolName = (stringTable + symbol->st_name);

			// Validate that we have a section header index
			if (symbol->st_shndx <= 0)
			{
				// TODO: Do custom linking or error

				// We error always for now
				if (loader->isKernel)
					WriteLog(LL_Debug, "unknown symbol %s.", symbolName);

				return false;
			}

			// Otherwise we link
			if (symbol->st_name)
			{
				if (loader->isKernel)
					WriteLog(LL_Debug, "relocating symbol %s.", symbolName);
			}
			else
			{
				// TODO: Some other shit
				/*elf64_section_header_t *shstrtab = (elf_section_headers + elf_header->string_table_index);
				elf64_section_header_t *section = (elf_section_headers + symbol->section);
				printf("Relocating symbol: %s%+lld\n", ((char *)shstrtab->address) + section->name_index, entry->addend);*/

			}

			uint64_t* location = (uint64_t*)(relocationSectionData + entry->r_offset);
			Elf64_Shdr* symbolSection = elfloader_getSectionHeaderByIndex(loader, symbol->st_shndx);
			if (!symbolSection)
				return false;

			Elf64_Addr symbolSectionAddress = symbolSection->sh_addr;

			switch (symbolType)
			{
			case R_X86_64_64:
				*location = symbolSectionAddress + symbol->st_value + entry->r_addend;
				break;
			case R_X86_64_PC32:
				*location = symbolSectionAddress + entry->r_addend - entry->r_offset;
				break;
			case R_X86_64_32:
				*location = symbolSectionAddress + entry->r_addend;
				break;
			case R_X86_64_32S:
				*location = symbolSectionAddress + entry->r_addend;
				break;
			case R_X86_64_NONE:
				break;
			default:
				if (loader->isKernel)
					WriteLog(LL_Warn, "Unknown relocation!!!!");
				break;
			}
		}
	}

	// TODO: Fix memory protections

	if (loader->isKernel)
		WriteLog(LL_Debug, "did everything");

	return true;
}


uint8_t elfloader_handleRelocations(ElfLoader_t* loader)
{
	if (!loader)
		return false;

	if (!loader->data || loader->dataSize == 0)
		return false;

	if (!do_everything(loader))
		return false;

	Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)loader->data;

	Elf64_Shdr* textHeader = elfloader_getSectionHeaderByName(loader, ".text");
	if (!textHeader)
	{
		if (loader->isKernel)
			WriteLog(LL_Error, "could not get .text section");
		else
			WriteNotificationLog("could not get .text section");

		return false;
	}

	// Validate that the .text section is within bounds
	if (textHeader->sh_offset >= loader->dataSize)
		return false;

	// Calculate the entry point by (elf in memory) + (.text section header offset) + (elf entry point offset)
	uint8_t* calculatedEntryPoint = (textHeader->sh_addr + elfHeader->e_entry);

	// Save the elf's main entry point for later
	loader->elfMain = (void(*)())calculatedEntryPoint;

	// Updates the elf entry point
	elfHeader->e_entry = (Elf64_Addr)calculatedEntryPoint;

	// Write out our progress to the user
	if (loader->isKernel)
		WriteLog(LL_Debug, "EP: %p", calculatedEntryPoint);
	else
		WriteNotificationLog("got entry point");

	return elfloader_updateElfProtections(loader);
}

uint8_t elfloader_updateElfProtections(ElfLoader_t* loader)
{
	return true;
}

uint8_t elfloader_isElfValid(ElfLoader_t* loader)
{
	// Validate loader
	if (!loader)
		return false;

	// Validate that we have valid data and size
	if (!loader->data || loader->dataSize == 0)
		return false;

	// Get the header
	Elf64_Ehdr* header = (Elf64_Ehdr*)loader->data;

	// Validate the ELF header
	if (header->e_ident[EI_MAG0] != ELFMAG0 ||
		header->e_ident[EI_MAG1] != ELFMAG1 ||
		header->e_ident[EI_MAG2] != ELFMAG2 ||
		header->e_ident[EI_MAG3] != ELFMAG3)
		return false;

	// Validate the type is executable or relocatable
	if (header->e_type != ET_EXEC &&
		header->e_type != ET_REL &&
		header->e_type != ET_DYN)
		return false;

	// Only accept X64 binaries
	if (header->e_machine != EM_X86_64)
		return false;

	// Good nuff'
	return true;
}

void elfloader_memset(ElfLoader_t* loader, void* address, int32_t val, size_t len)
{
	if (!loader)
		return;

	if (loader->isKernel)
	{
#ifndef _WIN32
		void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
		memset(address, val, len);
#endif
	}
	else
	{
		volatile uint8_t c = (uint8_t)val;

		for (size_t i = 0; i < len; ++i)
			*(((uint8_t*)address) + i) = c;
	}
}

void elfloader_memcpy(ElfLoader_t* loader, void* dst, void* src, size_t cnt)
{
	if (!loader)
		return;

	if (loader->isKernel)
	{
#ifndef _WIN32
		void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);
		memcpy(dst, src, cnt);
#endif
	}
	else
	{
		for (size_t i = 0; i < cnt; ++i)
			((uint8_t*)dst)[i] = ((uint8_t*)src)[i];
	}
}

int32_t elfloader_strcmp(const char *s1, const char *s2)
{
	while (*s1 == *s2++)
		if (*s1++ == '\0')
			return (0);
	return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
}

void* elfloader_allocate(ElfLoader_t* loader, uint64_t size)
{
	if (!loader)
		return NULL;

	if (size == 0)
		return NULL;

	void* allocationData = NULL;

	if (loader->isKernel)
	{
#ifndef _WIN32
		vm_offset_t(*kmem_alloc)(vm_map_t map, vm_size_t size) = kdlsym(kmem_alloc);
		vm_map_t map = (vm_map_t)(*(uint64_t *)(kdlsym(kernel_map)));

		allocationData = (void*)kmem_alloc(map, size);
#endif
	}
	else
		allocationData = (Elf64_Rela*)_mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);

	return allocationData;
}