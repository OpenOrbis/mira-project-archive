#include "utils.h"

int(*sceNetSocket)(const char *, int, int, int) = NULL;
int(*sceNetSocketClose)(int) = NULL;
int(*sceNetBind)(int, struct sockaddr *, int) = NULL;
int(*sceNetListen)(int, int) = NULL;
int(*sceNetAccept)(int, struct sockaddr *, unsigned int *) = NULL;
int(*sceNetRecv)(int, void *, size_t, int) = NULL;

int(*snprintf)(char *str, size_t size, const char *format, ...) = NULL;

void loader_memset(void* address, int32_t val, size_t len)
{
	for (size_t i = 0; i < len; ++i)
		*(((uint8_t*)address) + i) = 0;
}

int
loader_strcmp(const char *s1, const char *s2)
{
	while (*s1 == *s2++)
		if (*s1++ == '\0')
			return (0);
	return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
}