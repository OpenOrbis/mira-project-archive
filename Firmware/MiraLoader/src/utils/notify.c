#include "notify.h"
#include <oni/utils/types.h>
#include <oni/utils/dynlib.h>
#include <stdarg.h>



void loader_displayNotification(int id, char* text)
{
	if (!text)
		return;

	// Load the sysutil module, needs to be rooted for this to work
	int32_t moduleId = -1;
	sys_dynlib_load_prx("/system/common/lib/libSceSysUtil.sprx", &moduleId);

	// Validate that the module loaded properly
	if (moduleId == -1)
		return;

	int(*sceSysUtilSendSystemNotificationWithText)(int messageType, char* message) = NULL;

	// Resolve the symbol
	sys_dynlib_dlsym(moduleId, "sceSysUtilSendSystemNotificationWithText", &sceSysUtilSendSystemNotificationWithText);

	if (sceSysUtilSendSystemNotificationWithText)
		sceSysUtilSendSystemNotificationWithText(id, text);

	sys_dynlib_unload_prx(moduleId);
}

void loader_writelog(enum LogLevels level, const char* function, int line, const char* fmt, ...)
{
	int(*snprintf)(char *str, size_t size, const char *format, ...) = NULL;
	int(*vsnprintf)(char *str, size_t size, const char *format, va_list ap) = NULL;

	// Load the libc module
	int32_t libcModuleId = -1;
	sys_dynlib_load_prx("libSceLibcInternal.sprx", &libcModuleId);

	// Validate that our module loaded correctly
	if (libcModuleId == -1)
		return;

	// Resolve the functions that we need
	sys_dynlib_dlsym(libcModuleId, "snprintf", &snprintf);
	sys_dynlib_dlsym(libcModuleId, "vsnprintf", &vsnprintf);

	// Declare some buffers
	char buffer[64];
	char finalBuffer[64];

	// Zero out our buffers
	for (size_t i = 0; i < sizeof(buffer); ++i)
		buffer[i] = 0;

	for (size_t i = 0; i < sizeof(finalBuffer); ++i)
		finalBuffer[i] = 0;

	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	snprintf(finalBuffer, sizeof(finalBuffer), "%s:%d : %s", function, line, buffer);

	// Unload the libc modules we had previously loaded
	sys_dynlib_unload_prx(libcModuleId);

	// Display the notification
	loader_displayNotification(222, finalBuffer);
}