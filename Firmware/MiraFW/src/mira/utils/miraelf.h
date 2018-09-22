#pragma once
#include <oni/utils/types.h>

#define MODULE_NULL "(null)"

#define OPTION_MAXSTRINGLEN 16

#define TRAINER_TITLEIDMAX	4
#define TRAINER_TITLEIDLEN	10

#define COMPILE_ASSERT(x) extern int __dummy[(int)x]

typedef enum _mira_module_type_t
{
	ModuleType_Trainer,
	ModuleType_UserExecutable,
	ModuleType_KernelExecutable,
	ModuleType_COUNT
} mira_module_type_t;

extern char* _mira_module_type_strings_t[]; /* = 
{
	"Trainer",
	"User Executable",
	"Kernel Executable",
	""
};*/

typedef struct _mira_module_header_t
{
	// The type of this module
	mira_module_type_t moduleType;

	// Versioning
	uint16_t majorVersion;
	uint16_t minorVersion;
} mira_module_header_t;
COMPILE_ASSERT(sizeof(mira_module_header_t) == 8);

enum mira_trainer_option_type_t
{
	OptionType_String,
	OptionType_Int,
	OptionType_Int64,
	OptionType_Float,
	OptionType_Double,
	OptionType_COUNT
};

union mira_option_data_t
{
	char stringOption[OPTION_MAXSTRINGLEN];
	int32_t intOption;
	int64_t int64Option;
	float floatOption;
	double doubleOption;
};

struct mira_trainer_option_t
{
	// The name of this option to be displayed
	char* name;

	// The description of this option
	char* description;

	// The option type (determines how data is read)
	enum mira_trainer_option_type_t type;

	// The option data
	union mira_option_data_t data;

};

struct mira_trainer_module_t
{
	mira_module_header_t header;

	// Title of this trainer
	char* title;

	// Description of this trainer
	char* description;

	// Supported title id count
	uint16_t titleIdCount;

	// Number of options to read
	uint16_t optionCount;

	// Title id's of games to check
	char titleIds[TRAINER_TITLEIDLEN][TRAINER_TITLEIDMAX];

	struct mira_trainer_option_t options[];
};