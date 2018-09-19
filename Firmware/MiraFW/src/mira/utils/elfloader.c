#include "elfloader.h"
#include <sys/elf.h>
#include <sys/elf64.h>
#include <oni/utils/logger.h>
#include <oni/utils/kdlsym.h>

/*
	THIS CODE IS WIP, DONT TOUCH PLS THANKS
	ITS NOT REQUIRED FOR CLEAN BUILD OR EVEN ENABLED
*/

#define PT_MIRAHEADER	0x70000001

uint8_t elfloader_loadTrainer() { return false; }
uint8_t elfloader_loadKernelExecutable() { return false; }
uint8_t elfloader_loadUserExecutable() { return false; }


void* elfloader_parseMiraProgramHeader(Elf64_Phdr* programHeader, uint8_t* elfData)
{
	// Verify our program header and data
	if (!programHeader || !elfData)
	{
		WriteLog(LL_Error, "invalid program header or elf data");
		return NULL;
	}

	// Get the loader header
	struct mira_loader_header_t* loaderHeader = (struct mira_loader_header_t*)(elfData + programHeader->p_offset);
	if (!loaderHeader)
	{
		WriteLog(LL_Error, "could not load loader header");
		return NULL;
	}

	// Validate the type
	if (loaderHeader->type > LT_COUNT)
	{
		WriteLog(LL_Error, "loader type is invalid (%d).", loaderHeader->type);
		return NULL;
	}

	WriteLog(LL_Info, "attempting to load %s module version %02d.%02d", LoaderTypeStrings[loaderHeader->type], loaderHeader->majorVersion, loaderHeader->minorVersion);

	uint8_t loadSuccessful = false;
	// We will load depending on the type
	switch (loaderHeader->type)
	{
	case LT_Trainer:
		loadSuccessful = elfloader_loadTrainer();
		break;
	case LT_KernelExecutable:
		loadSuccessful = elfloader_loadKernelExecutable();
		break;
	case LT_UserExecutable:
		loadSuccessful = elfloader_loadUserExecutable();
		break;
	default:
		WriteLog(LL_Warn, "loader type (%d) unsupported.", loaderHeader->type);
		break;
	}

	if (!loadSuccessful)
	{
		WriteLog(LL_Error, "could not load");
		return NULL;
	}

	// TODO: Implement returning some tracking object for this elf
	return NULL;
}

void load_elf(uint8_t* data, uint64_t size)
{
	int(*memcmp)(const void* s1, const void* s2, size_t n) = kdlsym(memcmp);

	if (!data || size == 0)
	{
		WriteLog(LL_Error, "data or size is invalid");
		return;
	}

	if (size < sizeof(Elf64_Ehdr))
	{
		WriteLog(LL_Error, "elf size is too small have (%llx) want (%llx).", size, sizeof(Elf64_Ehdr));
		return;
	}

	Elf64_Ehdr* header = (Elf64_Ehdr*)data;

	// Verify the elf header
	if (memcmp(header->e_ident, ELFMAG, SELFMAG))
	{
		WriteLog(LL_Error, "elf header check failed");
		return;
	}

	// Verify the elf type
	if (header->e_type != ET_EXEC || header->e_machine != EM_X86_64)
	{
		WriteLog(LL_Error, "elf not an executable, or not compiled for x86_64");
		return;
	}

	// Get the entry point where we want to start running code
	Elf64_Addr entryPoint = header->e_entry;

	WriteLog(LL_Debug, "executable entry point %p", entryPoint);

	// Iterate through each of the elf segments to find the one that we want
	Elf64_Half programHeaderCount = header->e_phnum;
	Elf64_Half programHeaderStart = header->e_phoff;

	for (size_t headerIndex = 0; headerIndex < programHeaderCount; ++headerIndex)
	{
		Elf64_Phdr* programHeader = (Elf64_Phdr*)((data + programHeaderStart) + (headerIndex * sizeof(Elf64_Phdr)));
		if (!programHeader)
			continue;

		switch (programHeader->p_type)
		{
		case PT_MIRAHEADER:
			elfloader_parseMiraProgramHeader(programHeader, data);
			break;
		default:
			break;
		}
	}

	// Test change
} 