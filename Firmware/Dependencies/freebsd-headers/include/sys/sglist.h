/*-
 * Copyright (c) 2008 Yahoo!, Inc.
 * All rights reserved.
 * Written by: John Baldwin <jhb@FreeBSD.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: release/9.0.0/sys/sys/sglist.h 196404 2009-08-20 19:23:58Z jhb $
 */

/*
 * A scatter/gather list describes a group of physical address ranges.
 * Each physical address range consists of a starting address and a
 * length.
 */

#ifndef __SGLIST_H__
#define	__SGLIST_H__

#include <sys/refcount.h>

struct sglist_seg {
	vm_paddr_t	ss_paddr;
	size_t		ss_len;
};

struct sglist {
	struct sglist_seg *sg_segs;
	int		sg_refs;
	u_short		sg_nseg;
	u_short		sg_maxseg;
};

struct mbuf;
struct uio;

static __inline void
sglist_init(struct sglist *sg, u_short maxsegs, struct sglist_seg *segs)
{

	sg->sg_segs = segs;
	sg->sg_nseg = 0;
	sg->sg_maxseg = maxsegs;
	refcount_init(&sg->sg_refs, 1);
}

static __inline void
sglist_reset(struct sglist *sg)
{

	sg->sg_nseg = 0;
}

static __inline struct sglist *
sglist_hold(struct sglist *sg)
{

	refcount_acquire(&sg->sg_refs);
	return (sg);
}

struct sglist *sglist_alloc(int nsegs, int mflags);
int	sglist_append(struct sglist *sg, void *buf, size_t len);
int	sglist_append_mbuf(struct sglist *sg, struct mbuf *m0);
int	sglist_append_phys(struct sglist *sg, vm_paddr_t paddr,
	    size_t len);
int	sglist_append_uio(struct sglist *sg, struct uio *uio);
int	sglist_append_user(struct sglist *sg, void *buf, size_t len,
	    struct thread *td);
struct sglist *sglist_build(void *buf, size_t len, int mflags);
struct sglist *sglist_clone(struct sglist *sg, int mflags);
int	sglist_consume_uio(struct sglist *sg, struct uio *uio, size_t resid);
int	sglist_count(void *buf, size_t len);
void	sglist_free(struct sglist *sg);
int	sglist_join(struct sglist *first, struct sglist *second);
size_t	sglist_length(struct sglist *sg);
int	sglist_slice(struct sglist *original, struct sglist **slice,
	    size_t offset, size_t length, int mflags);
int	sglist_split(struct sglist *original, struct sglist **head,
	    size_t length, int mflags);

#endif	/* !__SGLIST_H__ */
