#pragma once
enum LogLevels
{
	LogLevel_WhoGivesAFuck = 1337,
};

#define WriteLizog(fmt, ...) loader_writelog(LogLevel_WhoGivesAFuck, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

void loader_displayNotification(int id, char* text);

void loader_writelog(enum LogLevels level, const char* function, int line, const char* fmt, ...);