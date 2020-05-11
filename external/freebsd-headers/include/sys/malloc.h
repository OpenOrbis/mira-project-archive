/*-
 * Copyright (c) 1987, 1993
 *	The Regents of the University of California.
 * Copyright (c) 2005, 2009 Robert N. M. Watson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)malloc.h	8.5 (Berkeley) 5/3/95
 * $FreeBSD: release/9.0.0/sys/sys/malloc.h 210564 2010-07-28 15:36:12Z mdf $
 */

#ifndef _SYS_MALLOC_H_
#define	_SYS_MALLOC_H_

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/_lock.h>
#include <sys/_mutex.h>

#define	MINALLOCSIZE	UMA_SMALLEST_UNIT

/*
 * flags to malloc.
 */
#define	M_NOWAIT	0x0001		/* do not block */
#define	M_WAITOK	0x0002		/* ok to block */
#define	M_ZERO		0x0100		/* bzero the allocation */
#define	M_NOVM		0x0200		/* don't ask VM for pages */
#define	M_EXEC	0x4000	/* allocate executable space */
#define	M_USE_RESERVE	0x0400		/* can alloc out of reserve memory */
#define	M_NODUMP	0x0800	

#define	M_MAGIC		877983977	/* time when first defined :-) */

/*
 * Two malloc type structures are present: malloc_type, which is used by a
 * type owner to declare the type, and malloc_type_internal, which holds
 * malloc-owned statistics and other ABI-sensitive fields, such as the set of
 * malloc statistics indexed by the compile-time MAXCPU constant.
 * Applications should avoid introducing dependence on the allocator private
 * data layout and size.
 *
 * The malloc_type ks_next field is protected by malloc_mtx.  Other fields in
 * malloc_type are static after initialization so unsynchronized.
 *
 * Statistics in malloc_type_stats are written only when holding a critical
 * section and running on the CPU associated with the index into the stat
 * array, but read lock-free resulting in possible (minor) races, which the
 * monitoring app should take into account.
 */
struct malloc_type_stats {
	uint64_t	mts_memalloced;	/* Bytes allocated on CPU. */
	uint64_t	mts_memfreed;	/* Bytes freed on CPU. */
	uint64_t	mts_numallocs;	/* Number of allocates on CPU. */
	uint64_t	mts_numfrees;	/* number of frees on CPU. */
	uint64_t	mts_size;	/* Bitmask of sizes allocated on CPU. */
	uint64_t	_mts_reserved1;	/* Reserved field. */
	uint64_t	_mts_reserved2;	/* Reserved field. */
	uint64_t	_mts_reserved3;	/* Reserved field. */
};

/*
 * Index definitions for the mti_probes[] array.
 */
#define DTMALLOC_PROBE_MALLOC		0
#define DTMALLOC_PROBE_FREE		1
#define DTMALLOC_PROBE_MAX		2

struct malloc_type_internal {
	uint32_t	mti_probes[DTMALLOC_PROBE_MAX];
					/* DTrace probe ID array. */
	u_char		mti_zone;
	struct malloc_type_stats	mti_stats[MAXCPU];
};

/*
 * Public data structure describing a malloc type.  Private data is hung off
 * of ks_handle to avoid encoding internal malloc(9) data structures in
 * modules, which will statically allocate struct malloc_type.
 */
struct malloc_type {
	struct malloc_type *ks_next;	/* Next in global chain. */
	u_long		 ks_magic;	/* Detect programmer error. */
	const char	*ks_shortdesc;	/* Printable type name. */
	void		*ks_handle;	/* Priv. data, was lo_class. */
};

/*
 * Statistics structure headers for user space.  The kern.malloc sysctl
 * exposes a structure stream consisting of a stream header, then a series of
 * malloc type headers and statistics structures (quantity maxcpus).  For
 * convenience, the kernel will provide the current value of maxcpus at the
 * head of the stream.
 */
#define	MALLOC_TYPE_STREAM_VERSION	0x00000001
struct malloc_type_stream_header {
	uint32_t	mtsh_version;	/* Stream format version. */
	uint32_t	mtsh_maxcpus;	/* Value of MAXCPU for stream. */
	uint32_t	mtsh_count;	/* Number of records. */
	uint32_t	_mtsh_pad;	/* Pad/reserved field. */
};

#define	MALLOC_MAX_NAME	32
struct malloc_type_header {
	char				mth_name[MALLOC_MAX_NAME];
};

#ifdef _KERNEL
#define	MALLOC_DEFINE(type, shortdesc, longdesc)			\
	struct malloc_type type[1] = {					\
		{ NULL, M_MAGIC, shortdesc, NULL }			\
	};								\
	SYSINIT(type##_init, SI_SUB_KMEM, SI_ORDER_SECOND, malloc_init,	\
	    type);							\
	SYSUNINIT(type##_uninit, SI_SUB_KMEM, SI_ORDER_ANY,		\
	    malloc_uninit, type)

#define	MALLOC_DECLARE(type) \
	extern struct malloc_type type[1]

MALLOC_DECLARE(M_CACHE);
MALLOC_DECLARE(M_DEVBUF);
MALLOC_DECLARE(M_TEMP);

MALLOC_DECLARE(M_IP6OPT); /* for INET6 */
MALLOC_DECLARE(M_IP6NDP); /* for INET6 */

/*
 * Deprecated macro versions of not-quite-malloc() and free().
 */
#define	MALLOC(space, cast, size, type, flags) \
	((space) = (cast)malloc((u_long)(size), (type), (flags)))
#define	FREE(addr, type) free((addr), (type))

/*
 * XXX this should be declared in <sys/uio.h>, but that tends to fail
 * because <sys/uio.h> is included in a header before the source file
 * has a chance to include <sys/malloc.h> to get MALLOC_DECLARE() defined.
 */
MALLOC_DECLARE(M_IOV);

extern struct mtx malloc_mtx;

/*
 * Function type used when iterating over the list of malloc types.
 */
typedef void malloc_type_list_func_t(struct malloc_type *, void *);

void	contigfree(void *addr, unsigned long size, struct malloc_type *type);
void	*contigmalloc(unsigned long size, struct malloc_type *type, int flags,
	    vm_paddr_t low, vm_paddr_t high, unsigned long alignment,
	    unsigned long boundary) __malloc_like;
void	free(void *addr, struct malloc_type *type);
void	*malloc(unsigned long size, struct malloc_type *type, int flags) __malloc_like;
void	malloc_init(void *);
int	malloc_last_fail(void);
void	malloc_type_allocated(struct malloc_type *type, unsigned long size);
void	malloc_type_freed(struct malloc_type *type, unsigned long size);
void	malloc_type_list(malloc_type_list_func_t *, void *);
void	malloc_uninit(void *);
void	*realloc(void *addr, unsigned long size, struct malloc_type *type,
	    int flags);
void	*reallocf(void *addr, unsigned long size, struct malloc_type *type,
	    int flags);

struct malloc_type *malloc_desc2type(const char *desc);
#endif /* _KERNEL */

#endif /* !_SYS_MALLOC_H_ */
