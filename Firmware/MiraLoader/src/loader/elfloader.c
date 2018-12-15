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

#define WriteLog(x, y, ...) fprintf((x == LL_Error) ? stderr : stdout, y, __VA_ARGS__)
#define WriteNotificationLog(x) fprintf(stdout, x)

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
#include <sys/fcntl.h>
#include <sys/unistd.h>

#include <oni/utils/sys_wrappers.h>
#include <oni/utils/escape.h>
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

	//if (loader->isKernel)
	//	WriteLog(LL_Debug, "allocationAddress: %p");

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

		if (sectionHeader->sh_type == SHN_UNDEF)
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

	// Get the string header index, and bounds check it
	Elf64_Half stringHeaderIndex = elfHeader->e_shstrndx;
	if (stringHeaderIndex > elfHeader->e_shnum)
		return false;

	Elf64_Shdr* stringTableHeader = elfloader_getSectionHeaderByIndex(loader, stringHeaderIndex);
	if (!stringTableHeader)
		return false;

	// Set the output variable if it's set
	if (outStringTableSize)
		*outStringTableSize = stringTableHeader->sh_size;

	*outStringTable = (const char*)(loader->data + stringTableHeader->sh_offset);

	return true;
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

Elf64_Phdr* elfloader_getProgramHeaderByVirtualAddress(ElfLoader_t* loader, Elf64_Addr p_VirtualAddress)
{
	if (!loader)
		return NULL;

	if (!loader->data || loader->dataSize == 0)
		return NULL;

	Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)loader->data;
	Elf64_Half programHeaderCount = elfHeader->e_phnum;
	for (Elf64_Half programHeaderIndex = 0; programHeaderIndex < programHeaderCount; ++programHeaderIndex)
	{
		Elf64_Phdr* header = elfloader_getProgramHeaderByIndex(loader, programHeaderIndex);
		if (!header)
			continue;

		if (p_VirtualAddress >= header->p_paddr && p_VirtualAddress < header->p_paddr + header->p_memsz)
			return header;
	}

	return NULL;
}

uint8_t elfloader_getSymbolAddress(ElfLoader_t* loader, const char* symbolLookup, void** outAddress)
{
	if (!loader)
		return false;

	if (!loader->data || loader->dataSize == 0)
		return false;

	if (!symbolLookup)
		return false;

	if (!outAddress)
		return false;

	// Set our output address to null
	*outAddress = NULL;

	Elf64_Ehdr* header = (Elf64_Ehdr*)loader->data;

	// Get the string tbale
	const char* stringTable = NULL;
	uint64_t stringTableSize = 0;
	if (!elfloader_internalGetStringTable(loader, &stringTable, &stringTableSize))
	{
		if (loader->isKernel)
			WriteLog(LL_Debug, "could not get string table");
		else
			WriteNotificationLog("could not get string table");

		return false;
	}

	// Iterate through all of the section headers
	Elf64_Half sectionCount = header->e_shnum;
	for (Elf64_Half sectionIndex = 0; sectionIndex < sectionCount; ++sectionIndex)
	{
		Elf64_Shdr* sectionHeader = elfloader_getSectionHeaderByIndex(loader, sectionIndex);
		if (!sectionHeader)
			continue;

		// We only want the symbol table
		if (sectionHeader->sh_type != SHT_SYMTAB)
			continue;

		// Get the symbol table in memory
		Elf64_Sym* symbolTable = (Elf64_Sym*)sectionHeader->sh_addr;
		if (!symbolTable)
			continue;

		// Iterate through all of the symbols
		Elf64_Xword symbolCount = sectionHeader->sh_size / sectionHeader->sh_entsize;
		for (Elf64_Xword symbolIndex = 0; symbolIndex < symbolCount; ++symbolIndex)
		{
			// Get symbol
			Elf64_Sym* symbol = elfloader_getSymbolByIndex(loader, (int32_t)symbolIndex);
			if (!symbol)
				continue;

			Elf64_Shdr* stringTableHeader = elfloader_getSectionHeaderByIndex(loader, sectionHeader->sh_link);
			if (!stringTableHeader)
				continue;

			// TODO: Bounds check the st_name offset
			const char* symbolName = (((const char*)stringTableHeader->sh_addr) + symbol->st_name);

			// Compare the symbol name to the one we are looking for
			if (elfloader_strcmp(symbolLookup, symbolName) != 0)
				continue;

			// Get the section header by index
			Elf64_Shdr* symbolSectionHeader = elfloader_getSectionHeaderByIndex(loader, symbol->st_shndx);
			if (!symbolSectionHeader)
				continue;

			// Assign the symbol value
			*outAddress = (void*)(((uint8_t*)symbolSectionHeader->sh_addr) + symbol->st_value);
			return true;
		}
	}

	if (loader->isKernel)
		WriteLog(LL_Debug, "didn't find anything");
	else
		WriteNotificationLog("didn't find anything");

	// We didn't find anything :(
	return false;
}

void* elfloader_resolve(ElfLoader_t* loader, const char* symbol)
{
	// We do not support resolving of kernel
	if (loader->isKernel)
		return NULL;

	// TODO: Load sce modules and dlsym against them
	return NULL;
}

void elfloader_relocate(ElfLoader_t* loader, Elf64_Shdr* sectionHeader, const Elf64_Sym* symbols, const char* strings, const uint8_t* source, uint8_t* destination)
{
	if (!loader)
		return;

	if (!loader->data || loader->dataSize == 0)
		return;

	Elf64_Rela* rela = (Elf64_Rela*)(source + sectionHeader->sh_offset);

	if (sectionHeader->sh_entsize != sizeof(Elf64_Rela))
		return;

	Elf64_Xword entryCount = sectionHeader->sh_size / sectionHeader->sh_entsize;
	for (Elf64_Xword entryIndex = 0; entryIndex < entryCount; ++entryIndex)
	{
		Elf64_Word symbolIndex = ELF64_R_SYM(rela[entryIndex].r_info);
		Elf64_Word symbolType = ELF64_R_TYPE(rela[entryIndex].r_info);

		Elf64_Sym* symbol = &symbols[symbolIndex];

		const char* symbolName = strings + symbol->st_name;

		switch (symbolType)
		{
		//case R_X86_64_64:
		//	*(Elf64_Addr*)(destination + rela[entryIndex].r_offset) = symbolSectionAddress + symbol->st_value + entry->r_addend;
		//	break;
		//case R_X86_64_PC32:
		//	*location = symbolSectionAddress + entry->r_addend - entry->r_offset;
		//	break;
		//case R_X86_64_32:
		//	*location = symbolSectionAddress + entry->r_addend;
		//	break;
		//case R_X86_64_32S:
		//	*location = symbolSectionAddress + entry->r_addend;
		//	break;
		case R_X86_64_JMP_SLOT:
		case R_X86_64_GLOB_DAT:
			*(Elf64_Addr*)(destination + rela[entryIndex].r_offset) = (Elf64_Addr)elfloader_resolve(loader, symbolName);
			break;
		case R_X86_64_RELATIVE:
			*(Elf64_Addr*)(destination + rela[entryIndex].r_offset) = (Elf64_Addr)(destination + rela[entryIndex].r_addend);
			break;
		}
	}
}

void* elfloader_findSymbol(const char* name, Elf64_Shdr* sectionHeader, const char* strings, uint8_t* source, uint8_t* destination)
{
	Elf64_Sym* symbols = (Elf64_Sym*)(source + sectionHeader->sh_offset);

	Elf64_Xword entrySize = sectionHeader->sh_entsize;

	if (entrySize != sizeof(Elf64_Rela))
		return NULL;

	Elf64_Xword symbolCount = sectionHeader->sh_size / entrySize;
	for (uint32_t symbolIndex = 0; symbolIndex < symbolCount; ++symbolIndex)
	{
		const char* symbolName = strings + symbols[symbolIndex].st_name;
		if (elfloader_strcmp(name, symbolName) == 0)
			return destination + symbols[symbolIndex].st_value;
	}

	return NULL;
}

uint8_t elfloader_handleRelocations(ElfLoader_t* loader)
{
	if (!loader)
		return false;

	if (!loader->data || loader->dataSize == 0)
		return false;

	Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)loader->data;

	// Bounds check the program header list offset
	if (elfHeader->e_phoff >= loader->dataSize)
		return false;

	uint64_t dataSize = loader->dataSize;
	uint8_t* exec = elfloader_allocate(loader, ALLOC_5MB);
	if (!exec)
		return false;

	elfloader_memset(loader, exec, 0, dataSize);

	// Iterate through all of the program headers and allocate new data for them
	for (Elf64_Half programHeaderIndex = 0; programHeaderIndex < elfHeader->e_phnum; ++programHeaderIndex)
	{
		Elf64_Phdr* programHeader = elfloader_getProgramHeaderByIndex(loader, programHeaderIndex);
		if (!programHeader)
			continue;

		// We only want to allocate memory for loadable segments
		if (programHeader->p_type != PT_LOAD)
			continue;

		// The memory size should always be >= file size
		if (programHeader->p_filesz > programHeader->p_memsz)
			continue;

		// If there is nothing to be allocated, skip
		if (programHeader->p_filesz == 0)
			continue;

		// Bounds check the program header offset
		if (programHeader->p_offset >= loader->dataSize)
			continue;

		uint8_t* fileDataStart = loader->data + programHeader->p_offset;
		Elf64_Xword fileDataSize = programHeader->p_filesz;

		Elf64_Xword dataMemorySize = programHeader->p_memsz;

		uint8_t* execDataOffset = programHeader->p_vaddr + exec;

		elfloader_memcpy(loader, execDataOffset, fileDataStart, fileDataSize);

		// Calculate data protection starting from ---
		int32_t dataProtection = 0;

		if (programHeader->p_flags & PF_R)
			dataProtection |= PF_R;
		if (programHeader->p_flags & PF_W)
			dataProtection |= PF_W;
		if (programHeader->p_flags & PF_X)
			dataProtection |= PF_X;

		// Update the protection on this crap
		if (!elfloader_setProtection(loader, execDataOffset, dataMemorySize, dataProtection))
		{
			if (loader->isKernel)
				WriteLog(LL_Error, "could not set protection");
			else
				WriteNotificationLog("could not set protection");
		}
	}

	Elf64_Sym* symbolTable = NULL;
	const char* stringTable = NULL;
	//int(*_main)(int argc, char* argv[], char* env[]) = NULL;
	void(*_main)(void* args) = NULL;

	Elf64_Half sectionHeaderCount = elfHeader->e_shnum;
	for (Elf64_Half sectionHeaderIndex = 0; sectionHeaderIndex < sectionHeaderCount; ++sectionHeaderIndex)
	{
		Elf64_Shdr* sectionHeader = elfloader_getSectionHeaderByIndex(loader, sectionHeaderIndex);
		if (!sectionHeader)
			continue;

		if (sectionHeader->sh_offset >= loader->dataSize)
			continue;

		if (sectionHeader->sh_type != SHT_SYMTAB)
			continue;

		symbolTable = (Elf64_Sym*)(loader->data + sectionHeader->sh_offset);

		Elf64_Word stringSectionIndex = sectionHeader->sh_link;
		if (stringSectionIndex > sectionHeaderCount)
			continue;

		Elf64_Shdr* stringTableHeader = elfloader_getSectionHeaderByIndex(loader, stringSectionIndex);
		if (!stringTableHeader)
			continue;

		stringTable = (const char*)(loader->data + stringTableHeader->sh_offset);

		_main = elfloader_findSymbol("mira_entry", sectionHeader, stringTable, loader->data, exec);
		if (_main != NULL)
			break;
	}

	for (Elf64_Half sectionHeaderIndex = 0; sectionHeaderIndex < sectionHeaderCount; ++sectionHeaderIndex)
	{
		Elf64_Shdr* sectionHeader = elfloader_getSectionHeaderByIndex(loader, sectionHeaderIndex);
		if (!sectionHeader)
			continue;

		if (sectionHeader->sh_type != SHT_RELA)
			continue;

		elfloader_relocate(loader, sectionHeader, symbolTable, stringTable, loader->data, exec);
	}

	// Write out our progress to the user
	if (loader->isKernel)
		WriteLog(LL_Debug, "EP: %p", _main);
	else
		WriteNotificationLog("got entry point");

	loader->elfMain = _main;

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

	// Validate the type is dynamic lib or relocatable
	if (header->e_type != ET_REL &&
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

uint8_t elfloader_isProgramCool(ElfLoader_t* loader, int32_t index)
{
	if (index < 0)
		return false;

	Elf64_Phdr* header = elfloader_getProgramHeaderByIndex(loader, index);
	if (!header)
		return false;

	Elf64_Addr programHeaderAddress = header->p_vaddr;
	if (!programHeaderAddress)
		return false;

	Elf64_Xword programHeaderSize = header->p_memsz;
	if (programHeaderSize == 0)
		return false;

	// Validate that the old program header was in-bounds
	if (header->p_offset > loader->dataSize)
		return false;

	return true;
}

uint8_t elfloader_isSectionCool(ElfLoader_t* loader, int32_t index)
{
	if (index < 0)
		return false;

	Elf64_Shdr* sectionHeader = elfloader_getSectionHeaderByIndex(loader, index);
	if (!sectionHeader)
		return false;

	Elf64_Addr sectionHeaderAddress = sectionHeader->sh_addr;
	if (!sectionHeaderAddress)
		return false;

	Elf64_Xword sectionHeaderSize = sectionHeader->sh_size;
	if (sectionHeaderSize == 0)
		return false;

	// Bounds check the old offset before we update it
	if (sectionHeader->sh_offset > loader->dataSize)
		return false;

	return true;
}

uint8_t elfloader_dumpElf(ElfLoader_t* loader, char* filePath)
{
#ifdef _WIN32
	return false;
#else
	if (!loader)
		return false;

	if (!loader->data || loader->dataSize == 0)
		return false;

	uint8_t dumpSuccessful = false;

	// Escape the thread
	struct thread_info_t threadInfo;
	elfloader_memset(loader, &threadInfo, 0, sizeof(threadInfo));
	oni_threadEscape(curthread, &threadInfo);

	// Open up the file for writing
	int32_t fd = kopen(filePath, O_WRONLY | O_CREAT | O_TRUNC, 0777);
	if (fd < 0)
	{
		WriteLog(LL_Error, "could not open file for writing (%d).", fd);
		dumpSuccessful = false;
		goto error;
	}

	// TODO: Make this not modify shit, right now it will clobber the original shit, but shouldn't matter for the purpose of this
	Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)loader->data;

	// We start at the end of the elf header offset
	uint64_t programHeaderStart = sizeof(Elf64_Ehdr);

	// Update where we are starting our program headers in file
	uint64_t fileProgramHeaderCount = 0;
	uint64_t fileSectionHeaderCount = 0;

	// Count how many program headers that we have to write out (filtered)
	Elf64_Half programHeaderCount = elfHeader->e_phnum;
	for (Elf64_Half programHeaderIndex = 0; programHeaderIndex < programHeaderCount; programHeaderIndex++)
	{
		// Skip uncool programs
		if (!elfloader_isProgramCool(loader, programHeaderIndex))
			continue;

		fileProgramHeaderCount++;
	}

	// Iterate through the section headers
	Elf64_Half sectionHeaderCount = elfHeader->e_shnum;
	for (Elf64_Half sectionHeaderIndex = 0; sectionHeaderIndex < sectionHeaderCount; sectionHeaderIndex++)
	{
		// Skip uncool sections
		if (!elfloader_isSectionCool(loader, sectionHeaderIndex))
			continue;

		fileSectionHeaderCount++;
	}

	// Calculate the total size of the headers part
	uint64_t programHeadersSize = sizeof(Elf64_Phdr) * fileProgramHeaderCount;
	uint64_t sectionHeadersSize = sizeof(Elf64_Shdr) * fileSectionHeaderCount;

	// Calculate the program header data start, after the Program Headers + Section Headers
	uint64_t programDataStart = programHeaderStart + programHeadersSize + sectionHeadersSize;
	uint64_t programDataSize = 0;

	// Go through all of the program headers again and write all of the program headers while updating them
	for (Elf64_Half programHeaderIndex = 0; programHeaderIndex < programHeaderCount; programHeaderIndex++)
	{
		if (!elfloader_isProgramCool(loader, programHeaderIndex))
			continue;

		Elf64_Phdr* programHeader = elfloader_getProgramHeaderByIndex(loader, programHeaderIndex);
		if (!programHeader)
			continue;

		// Update the program header with the new in-file offset
		programHeader->p_offset = programDataStart + programDataSize;

		// Write out the program header
		if (kwrite(fd, (const void*)programHeader, sizeof(*programHeader)) < 0)
		{
			WriteLog(LL_Error, "could not write program header");
			goto error;
		}

		programDataSize += programHeader->p_memsz;
	}

	uint64_t sectionDataStart = programDataStart + programDataSize;
	uint64_t sectionDataSize = 0;

	// Iterate through the section headers
	for (Elf64_Half sectionHeaderIndex = 0; sectionHeaderIndex < sectionHeaderCount; sectionHeaderIndex++)
	{
		if (!elfloader_isSectionCool(loader, sectionHeaderIndex))
			continue;

		Elf64_Shdr* sectionHeader = elfloader_getSectionHeaderByIndex(loader, sectionHeaderIndex);
		if (!sectionHeader)
			continue;

		// Update the section header
		sectionHeader->sh_offset = sectionDataStart + sectionDataSize;

		// Update the file offset
		sectionDataSize += sectionHeader->sh_size;
	}

	// ===================================================================
	// DO THE WRITES
	// ===================================================================
	//uint64_t fileProgramHeaderSize = fileProgramHeaderCount * sizeof(Elf64_Phdr);
	//uint64_t fileSectionHeaderSize = fileSectionHeaderCount * sizeof(Elf64_Shdr);

	elfHeader->e_phoff = programHeaderStart;

	// Update the section header start offset
	elfHeader->e_shoff = programHeaderStart;

	uint8_t* currentAddress = loader->data;

	// Write out the elf header
	if (kwrite(fd, currentAddress, sizeof(Elf64_Ehdr)) < 0)
	{
		if (loader->isKernel)
			WriteLog(LL_Error, "could not write elf header to file");

		dumpSuccessful = false;
		goto error;
	}

	// Iterate and write all program headers
	for (Elf64_Half programHeaderIndex = 0; programHeaderIndex < programHeaderCount; programHeaderIndex++)
	{
		Elf64_Phdr* programHeader = elfloader_getProgramHeaderByIndex(loader, programHeaderIndex);
		if (!programHeader)
			continue;

		Elf64_Addr programHeaderAddress = programHeader->p_vaddr;
		if (!programHeaderAddress)
			continue;

		Elf64_Xword programHeaderSize = programHeader->p_memsz;
		if (programHeaderSize == 0)
			continue;

		if (kwrite(fd, (const void*)programHeaderAddress, programHeaderSize) < 0)
		{
			if (loader->isKernel)
				WriteLog(LL_Error, "could not write program header index (%d).", programHeaderIndex);

			dumpSuccessful = false;
			goto error;
		}

		// Update the file offset with the memory size of this
		programHeaderStart += programHeaderSize;
	}

	// Update the section header start offset
	elfHeader->e_shoff = programHeaderStart;

	// Iterate through the section headers
	for (Elf64_Half sectionHeaderIndex = 0; sectionHeaderIndex < sectionHeaderCount; sectionHeaderIndex++)
	{
		Elf64_Shdr* sectionHeader = elfloader_getSectionHeaderByIndex(loader, sectionHeaderIndex);
		if (!sectionHeader)
			continue;

		Elf64_Addr sectionHeaderAddress = sectionHeader->sh_addr;
		if (!sectionHeaderAddress)
			continue;

		Elf64_Xword sectionHeaderSize = sectionHeader->sh_size;
		if (sectionHeaderSize == 0)
			continue;

		// Update the section header
		sectionHeader->sh_offset = programHeaderStart;

		// Update the file offset
		programHeaderStart += sectionHeaderSize;
	}

	dumpSuccessful = true;

error:
	if (fd > 0)
		kclose(fd);

	oni_threadRestore(curthread, &threadInfo);

	return dumpSuccessful;
#endif
}