#pragma once
#include <oni/utils/types.h>

struct sockaddr;

void loader_memset(void* address, int32_t val, size_t len);
int loader_strcmp(const char *s1, const char *s2);

int(*sceNetSocket)(const char *, int, int, int);
int(*sceNetSocketClose)(int);
int(*sceNetBind)(int, struct sockaddr *, int);
int(*sceNetListen)(int, int);
int(*sceNetAccept)(int, struct sockaddr *, unsigned int *);
int(*sceNetRecv)(int, void *, size_t, int);

int(*snprintf)(char *str, size_t size, const char *format, ...);