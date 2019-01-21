#pragma once
#include <oni/utils/types.h>

void* syscall1(
	uint64_t number,
	void* arg1
);

void* syscall2(
	uint64_t number,
	void* arg1,
	void* arg2
);

void* syscall3(
	uint64_t number,
	void* arg1,
	void* arg2,
	void* arg3
);

void* syscall4(
	uint64_t number,
	void* arg1,
	void* arg2,
	void* arg3,
	void* arg4
);

void* syscall5(
	uint64_t number,
	void* arg1,
	void* arg2,
	void* arg3,
	void* arg4,
	void* arg5
);

caddr_t _mmap(caddr_t addr, size_t len, int prot, int flags, int fd, off_t pos);
//caddr_t _mmap(void);
caddr_t _Allocate3MB(void);
caddr_t _Allocate5MB(void);