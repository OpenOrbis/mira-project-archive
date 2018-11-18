#include "elfloader.h"

#ifdef _WIN32
#include <Windows.h>
#include <stdio.h>

#define PROT_READ	0x1     /* Page can be read.  */
#define PROT_WRITE	0x2     /* Page can be written.  */
#define PROT_EXEC	0x4     /* Page can be executed.  */
#define PROT_NONE	0x0     /* Page can not be accessed.  */

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
#define ALLOC_5MB	0x500000


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
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

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

	// Allocate user memory
	if (!loader->isKernel)
		allocationData = _Allocate5MB();
	else
	{
		void* M_LINKER = kdlsym(M_LINKER);

		void * (*contigmalloc)(unsigned long	size, struct malloc_type *type, int flags,
			vm_paddr_t low, vm_paddr_t high, unsigned long	alignment,
			vm_paddr_t boundary) = kdlsym(contigmalloc);

		// Allocate some memory
		allocationData = contigmalloc(allocationSize, M_LINKER, M_NOWAIT | M_ZERO, 0, __UINT64_MAX__, PAGE_SIZE, 0);
	}

	if (!allocationData)
		return false;

	if (loader->isKernel)
		WriteLog(LL_Debug, "allocationData: %p", allocationData);

	uint8_t* allocationData2 = (uint8_t*)allocationData;

	// Zero out the allocaiton
	if (loader->isKernel)
		memset(allocationData, 0, allocationSize);
	else
		elfloader_memset(allocationData2, 0, allocationSize);

	if (loader->isKernel)
		WriteLog(LL_Debug, "memset allocation data\n");

	// Copy over the elf data
	if (loader->isKernel)
		memcpy(allocationData, data, dataLength);
	else
	{
		for (size_t i = 0; i < dataLength; ++i)
			allocationData2[i] = data[i];
	}

	if (loader->isKernel)
		WriteLog(LL_Debug, "copied elf data\n");

	loader->data = (uint8_t*)allocationData2;
	loader->dataSize = allocationSize;
	loader->elfSize = elfSize;

	if (loader->isKernel)
		WriteLog(LL_Debug, "loader success\n");

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

	if (loader->isKernel)
		WriteLog(LL_Debug, "dataStart: %p elfHeader: %p programHeaderCount: %d", dataStart, elfHeader, programHeaderCount);

	// Validate the index bounds
	if (index < 0 || index >= programHeaderCount)
		return NULL;

	// Bounds check the program header offset
	if (elfHeader->e_phoff >= loader->dataSize)
		return NULL;

	Elf64_Phdr* programHeader = (Elf64_Phdr*)((dataStart + elfHeader->e_phoff) + (sizeof(Elf64_Phdr) * index));

	if (loader->isKernel)
		WriteLog(LL_Debug, "programHeader: %p", programHeader);

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

	if (loader->isKernel)
		WriteLog(LL_Debug, "hasStringTable: %s", hasStringTable ? "true" : "false");

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

uint8_t elfloader_handleRelocations(ElfLoader_t* loader)
{
	if (!loader)
		return false;

	if (!loader->data || loader->dataSize == 0)
		return false;

	Elf64_Ehdr* elfHeader = (Elf64_Ehdr*)loader->data;

	uint64_t stringTableSize = 0;
	const char* stringTable = NULL;
	if (!elfloader_internalGetStringTable(loader, &stringTable, &stringTableSize))
	{
		//loader_displayNotification(222, "could not find string table");
		//return false;
	}

	// Check to see if the main has been resolved already
	if (loader->elfMain == NULL)
	{
		Elf64_Shdr* textHeader = elfloader_getSectionHeaderByName(loader, ".text");


		if (textHeader)
		{
			if (loader->isKernel)
				WriteLog(LL_Debug, "got .text section");
			else
				WriteNotificationLog("got .text");

			if (textHeader->sh_offset >= loader->dataSize)
				return false;

			uint8_t* entryPoint = loader->data + textHeader->sh_offset + elfHeader->e_entry;

			loader->elfMain = (void(*)())entryPoint;

			if (loader->isKernel)
				WriteLog(LL_Debug, "EP: %p", entryPoint);
			else
				WriteNotificationLog("got entry point");
		}
	}

	// Update all of the program headers
	Elf64_Half programHeaderCount = elfHeader->e_phnum;
	for (Elf64_Half programIndex = 0; programIndex < programHeaderCount; ++programIndex)
	{
		Elf64_Phdr* programHeader = elfloader_getProgramHeaderByIndex(loader, programIndex);
		if (!programHeader)
			continue;

		programHeader->p_vaddr = ((Elf64_Addr)loader->data + programHeader->p_offset);
		programHeader->p_memsz = programHeader->p_filesz;
	}

	// Update all of the section headers
	Elf64_Half sectionHeaderCount = elfHeader->e_shnum;
	for (Elf64_Half sectionIndex = 0; sectionIndex < sectionHeaderCount; ++sectionIndex)
	{
		// Get the section header
		Elf64_Shdr* sectionHeader = elfloader_getSectionHeaderByIndex(loader, sectionIndex);

		// Update current address
		sectionHeader->sh_addr = ((Elf64_Addr)loader->data + sectionHeader->sh_offset);

		// If the section header is not a relocatable section skip it
		if (sectionHeader->sh_type != SHT_RELA)
			continue;

		// Get the relocation count
		uint32_t relCount = sectionHeader->sh_size / sectionHeader->sh_entsize;

		// Iterate over all of the relocations
		for (uint32_t relIndex = 0; relIndex < relCount; ++relIndex)
		{
			Elf64_Rela* relocationTab = (Elf64_Rela*)(loader->data + sectionHeader->sh_offset + (sectionHeader->sh_entsize * relIndex));

			Elf64_Shdr* target = elfloader_getSectionHeaderByIndex(loader, sectionHeader->sh_info);

			// Location where the relocation has to happen
			uint8_t* address = loader->data + target->sh_offset;

			uint8_t** ref = (uint8_t**)(loader->data + relocationTab->r_offset);

			int32_t symbolSectionIndex = ELF64_R_SYM(relocationTab->r_info);
			if (symbolSectionIndex != SHN_UNDEF && stringTable != NULL && stringTableSize != 0)
			{
				//// TODO: Handle symbols
				//Elf64_Shdr* symbolTableHeader = elfloader_getSectionHeaderByIndex(loader, sectionHeader->sh_info);

				//// TODO: Get symbol entry
				//if (!symbolTableHeader)
				//	continue;

				//Elf64_Sym* symbol = (Elf64_Sym*)(loader->data + symbolTableHeader->sh_offset);
				//if (!symbol)
				//	continue;

				//
				///*if (symbol->st_name >= stringTableSize)
				//	continue;*/

				//const char* symbolName = stringTable + symbol->st_name;

				//printf("got symbol %s\n", symbolName);
			}

			// No idea if this is correct or not, seems so
			int32_t relocationType = ELF64_R_TYPE(relocationTab->r_info);
			switch (relocationType)
			{
			case R_X86_64_64:
				*ref = address + relocationTab->r_addend;
				break;
			}
		}
	}

#ifdef _WIN32
	// Debugging code to dump the elf back to file so we can inspect it in IDA
	FILE* file = NULL;
	errno_t error = fopen_s(&file, "dump.elf", "wb");
	fwrite(loader->data, sizeof(uint8_t), loader->elfSize, file);
	fclose(file);
#endif

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
		header->e_type != ET_REL)
		return false;

	// Only accept X64 binaries
	if (header->e_machine != EM_X86_64)
		return false;

	// Good nuff'
	return true;
}

void elfloader_memset(void* address, int32_t val, size_t len)
{
	volatile uint8_t c = (uint8_t)val;

	for (size_t i = 0; i < len; ++i)
		*(((uint8_t*)address) + i) = c;
}

int32_t elfloader_strcmp(const char *s1, const char *s2)
{
	while (*s1 == *s2++)
		if (*s1++ == '\0')
			return (0);
	return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
}