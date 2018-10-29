#pragma once

#ifndef NULL
#define NULL 0
#endif

#define true 1
#define false 0


typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef signed short int16_t;
typedef unsigned short uint16_t;

typedef signed long int32_t;
typedef unsigned long uint32_t;

typedef signed long long int64_t;
typedef unsigned long long uint64_t;

typedef uint64_t size_t;

#ifndef _SA_FAMILY_T_DECLARED
typedef	uint8_t	__sa_family_t;
typedef	__sa_family_t		sa_family_t;
#define	_SA_FAMILY_T_DECLARED
#endif

#ifndef _IN_PORT_T_DECLARED
typedef	uint16_t		in_port_t;
#define	_IN_PORT_T_DECLARED
#endif

#ifndef _IN_ADDR_T_DECLARED
typedef	uint32_t		in_addr_t;
#define	_IN_ADDR_T_DECLARED
#endif

#ifndef	_STRUCT_IN_ADDR_DECLARED
struct in_addr {
	in_addr_t s_addr;
};
#define	_STRUCT_IN_ADDR_DECLARED
#endif

struct sockaddr_in {
	uint8_t	sin_len;
	sa_family_t	sin_family;
	in_port_t	sin_port;
	struct	in_addr sin_addr;
	char	sin_zero[8];
};

#define INADDR_ANY 0x00000000

#define	AF_INET		2		/* internetwork: UDP, TCP, etc. */
#define	SOCK_STREAM	1		/* stream socket */

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))
#endif