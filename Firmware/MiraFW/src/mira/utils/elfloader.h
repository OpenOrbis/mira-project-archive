#pragma once
#include <oni/utils/types.h>

enum LoaderType
{
	LT_Trainer,
	LT_UserExecutable,
	LT_KernelExecutable,
	LT_COUNT
};

char* LoaderTypeStrings[] =
{
	"Trainer",
	"User Executable",
	"Kernel Executable",
	""
};

enum TrainerOptionType
{
	TT_String,
	TT_Int,
	TT_Int64,
	TT_Float,
	TT_Double,
	TT_COUNT
};

#define OPTION_MAXSTRINGLEN	32
#define TRAINER_TITLEIDMAX	6
#define TRAINER_TITLEIDLEN	10

union elf_optionData_t
{
	char stringOption[OPTION_MAXSTRINGLEN];
	int32_t intOption;
	int64_t int64Option;
	float floatOption;
	double doubleOption;
};

struct elf_trainerOption_t
{
	char* title;
	char* description;

	enum TrainerOptionType type;
	union elf_optionData_t data;
};

struct elf_trainer_t
{
	char* title;
	char* author;

	char titleIds[TRAINER_TITLEIDLEN][TRAINER_TITLEIDMAX];

	uint64_t optionCount;
	struct elf_trainerOption_t options[];
};

struct elfdata_t
{

};

struct mira_loader_header_t
{
	// The type that this module is
	enum LoaderType type;

	// Versioning
	uint16_t majorVersion;
	uint16_t minorVersion;
};

struct elfloader_t
{
	// The kernel space ELF copy
	uint8_t* elfData;

	// The size of the elf itself
	uint64_t elfSize;

	// Injected process id, -1 if new process
	int32_t processId;

	// The type that this is will determine how it is loaded
	enum LoaderType type;

	// The address that this ELF is loaded
	uint8_t* loadedElfAddress;

	// The total allocated size for this elf (page aligned)
	uint64_t loadedElfSize;
};