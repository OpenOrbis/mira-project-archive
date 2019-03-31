#include <oni/utils/logger.h>
#include <stdarg.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/sys_wrappers.h>

void logger_init(struct logger_t* logger)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	if (!logger)
		return;

#ifdef _DEBUG
	logger->logLevel = LL_Debug;
#else
	logger->logLevel = LL_Error;
#endif
	logger->logHandle = -1;

	memset(logger->buffer, 0, sizeof(logger->buffer));
	memset(logger->finalBuffer, 0, sizeof(logger->finalBuffer));

	void(*mtx_init)(struct mtx *m, const char *name, const char *type, int opts) = kdlsym(mtx_init);
	mtx_init(&logger->mutex, "logger", NULL, 0);
}

void logger_writelog(struct logger_t* logger, enum LogLevels level, const char* function, int line, const char* fmt, ...)
{
	if (!logger)
		return;

	// If the logger is set not to output anything, skip everything
	if (logger->logLevel <= LL_None)
		return;

	if (level > logger->logLevel)
		return;

	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	int(*snprintf)(char *str, size_t size, const char *format, ...) = kdlsym(snprintf);
	int(*vsnprintf)(char *str, size_t size, const char *format, va_list ap) = kdlsym(vsnprintf);
	void(*printf)(char *format, ...) = kdlsym(printf);
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);

	_mtx_lock_flags(&logger->mutex, 0, __FILE__, __LINE__);

	// Zero out the buffer
	memset(logger->buffer, 0, sizeof(logger->buffer));
	memset(logger->finalBuffer, 0, sizeof(logger->finalBuffer));

	va_list args;
	va_start(args, fmt);
	vsnprintf(logger->buffer, sizeof(logger->buffer), fmt, args);
	va_end(args);

	const char* levelString = "None";
	const char* levelColor = KNRM;
	switch (level)
	{
	case LL_Info:
		levelString = "Info";
		levelColor = KGRN;
		break;
	case LL_Warn:
		levelString = "Warn";
		levelColor = KYEL;
		break;
	case LL_Error:
		levelString = "Error";
		levelColor = KRED;
		break;
	case LL_Debug:
		levelString = "Debug";
		levelColor = KMAG;
		break;
	case LL_None:
	default:
		levelString = "None";
		levelColor = KNRM;
		break;
	}

	snprintf(logger->finalBuffer, sizeof(logger->finalBuffer), "%s[%s] %s:%d : %s %s\n", levelColor, levelString, function, line, logger->buffer, KNRM);
	printf(logger->finalBuffer);

	_mtx_unlock_flags(&logger->mutex, 0, __FILE__, __LINE__);
}