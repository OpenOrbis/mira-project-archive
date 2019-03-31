#pragma once
#include <oni/utils/types.h>

enum OptionValueType
{
	VT_STRING,
	VT_INT,
	VT_BOOL,
	VT_COUNT
};

#define OPTION_MAXVALUELEN	64
#define TRAINER_HASHLEN		20

struct traineroption_t
{
	const enum OptionValueType valueType;
	const char* name;
	const char* description;
};

struct traineroption_string_t
{
	struct traineroption_t option;
	
	const char defaultValue[OPTION_MAXVALUELEN];
	char value[OPTION_MAXVALUELEN];
};

struct traineroption_int_t
{
	struct traineroption_t option;

	const uint64_t defaultValue;
	uint64_t value;
};

struct traineroption_bool_t
{
	struct traineroption_t option;

	const uint8_t defaultValue;
	uint8_t value;
};

struct trainer_t
{
	const char* name;
	const char* titleId;

	const uint32_t optionCount;
	struct traineroption_t** options;
};

struct traineroption_bool_t noclipOption =
{
	{
		VT_BOOL,
		"Noclip",
		"Generic noclip mode"
	},
	false,
	false
};

struct traineroption_bool_t bottomlessClipOption =
{
	{
		VT_BOOL,
		"Bottomless Clip",
		"Unlimited ammo"
	},
	false,
	false
};

struct traineroption_string_t playerNameOption =
{
	{
		VT_STRING,
		"Player Name",
		"Overrides player name"
	},
	"Player1",
	{ 0 }
};

struct traineroption_t* myOptions[] =
{
	(struct traineroption_t*)&noclipOption,
	(struct traineroption_t*)&bottomlessClipOption,
	(struct traineroption_t*)&playerNameOption,
};

struct trainer_t myTrainer =
{
	// Display name of the trainer
	"Example Trainer",

	// TitleID for the trainer
	"CUSA01447",

	// Set the optionCount
	ARRAYSIZE(myOptions),

	// Set the options
	myOptions
};