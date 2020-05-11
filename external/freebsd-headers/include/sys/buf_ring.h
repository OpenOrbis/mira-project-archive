/*-
 * Copyright (c) 2007-2009 Kip Macy <kmacy@freebsd.org>
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
 * $FreeBSD: release/9.0.0/sys/sys/buf_ring.h 207673 2010-05-05 20:39:02Z joel $
 *
 */

#ifndef	_SYS_BUF_RING_H_
#define	_SYS_BUF_RING_H_

#include <machine/cpu.h>

#if defined(INVARIANTS) && !defined(DEBUG_BUFRING)
#define DEBUG_BUFRING 1
#endif

#ifdef DEBUG_BUFRING
#include <sys/lock.h>
#include <sys/mutex.h>
#endif

struct buf_ring {
	volatile uint32_t	br_prod_head;
	volatile uint32_t	br_prod_tail;	
	int              	br_prod_size;
	int              	br_prod_mask;
	uint64_t		br_drops;
	uint64_t		br_prod_bufs;
	uint64_t		br_prod_bytes;
	/*
	 * Pad out to next L2 cache line
	 */
	uint64_t	  	_pad0[11];

	volatile uint32_t	br_cons_head;
	volatile uint32_t	br_cons_tail;
	int		 	br_cons_size;
	int              	br_cons_mask;
	
	/*
	 * Pad out to next L2 cache line
	 */
	uint64_t	  	_pad1[14];
#ifdef DEBUG_BUFRING
	struct mtx		*br_lock;
#endif	
	void			*br_ring[0];
};

/*
 * multi-producer safe lock-free ring buffer enqueue
 *
 */
static __inline int
buf_ring_enqueue_bytes(struct buf_ring *br, void *buf, int nbytes)
{
	uint32_t prod_head, prod_next;
	uint32_t cons_tail;
	int success;
#ifdef DEBUG_BUFRING
	int i;
	for (i = br->br_cons_head; i != br->br_prod_head;
	     i = ((i + 1) & br->br_cons_mask))
		if(br->br_ring[i] == buf)
			panic("buf=%p already enqueue at %d prod=%d cons=%d",
			    buf, i, br->br_prod_tail, br->br_cons_tail);
#endif	
	critical_enter();
	do {
		prod_head = br->br_prod_head;
		cons_tail = br->br_cons_tail;

		prod_next = (prod_head + 1) & br->br_prod_mask;
		
		if (prod_next == cons_tail) {
			critical_exit();
			return (ENOBUFS);
		}
		
		success = atomic_cmpset_int(&br->br_prod_head, prod_head,
		    prod_next);
	} while (success == 0);
#ifdef DEBUG_BUFRING
	if (br->br_ring[prod_head] != NULL)
		panic("dangling value in enqueue");
#endif	
	br->br_ring[prod_head] = buf;
	wmb();

	/*
	 * If there are other enqueues in progress
	 * that preceeded us, we need to wait for them
	 * to complete 
	 */   
	while (br->br_prod_tail != prod_head)
		cpu_spinwait();
	br->br_prod_bufs++;
	br->br_prod_bytes += nbytes;
	br->br_prod_tail = prod_next;
	critical_exit();
	return (0);
}

static __inline int
buf_ring_enqueue(struct buf_ring *br, void *buf)
{

	return (buf_ring_enqueue_bytes(br, buf, 0));
}

/*
 * multi-consumer safe dequeue 
 *
 */
static __inline void *
buf_ring_dequeue_mc(struct buf_ring *br)
{
	uint32_t cons_head, cons_next;
	uint32_t prod_tail;
	void *buf;
	int success;

	critical_enter();
	do {
		cons_head = br->br_cons_head;
		prod_tail = br->br_prod_tail;

		cons_next = (cons_head + 1) & br->br_cons_mask;
		
		if (cons_head == prod_tail) {
			critical_exit();
			return (NULL);
		}
		
		success = atomic_cmpset_int(&br->br_cons_head, cons_head,
		    cons_next);
	} while (success == 0);		

	buf = br->br_ring[cons_head];
#ifdef DEBUG_BUFRING
	br->br_ring[cons_head] = NULL;
#endif
	rmb();
	
	/*
	 * If there are other dequeues in progress
	 * that preceeded us, we need to wait for them
	 * to complete 
	 */   
	while (br->br_cons_tail != cons_head)
		cpu_spinwait();

	br->br_cons_tail = cons_next;
	critical_exit();

	return (buf);
}

/*
 * single-consumer dequeue 
 * use where dequeue is protected by a lock
 * e.g. a network driver's tx queue lock
 */
static __inline void *
buf_ring_dequeue_sc(struct buf_ring *br)
{
	uint32_t cons_head, cons_next, cons_next_next;
	uint32_t prod_tail;
	void *buf;
	
	cons_head = br->br_cons_head;
	prod_tail = br->br_prod_tail;
	
	cons_next = (cons_head + 1) & br->br_cons_mask;
	cons_next_next = (cons_head + 2) & br->br_cons_mask;
	
	if (cons_head == prod_tail) 
		return (NULL);

#ifdef PREFETCH_DEFINED	
	if (cons_next != prod_tail) {		
		prefetch(br->br_ring[cons_next]);
		if (cons_next_next != prod_tail) 
			prefetch(br->br_ring[cons_next_next]);
	}
#endif
	br->br_cons_head = cons_next;
	buf = br->br_ring[cons_head];

#ifdef DEBUG_BUFRING
	br->br_ring[cons_head] = NULL;
	if (!mtx_owned(br->br_lock))
		panic("lock not held on single consumer dequeue");
	if (br->br_cons_tail != cons_head)
		panic("inconsistent list cons_tail=%d cons_head=%d",
		    br->br_cons_tail, cons_head);
#endif
	br->br_cons_tail = cons_next;
	return (buf);
}

/*
 * return a pointer to the first entry in the ring
 * without modifying it, or NULL if the ring is empty
 * race-prone if not protected by a lock
 */
static __inline void *
buf_ring_peek(struct buf_ring *br)
{

#ifdef DEBUG_BUFRING
	if ((br->br_lock != NULL) && !mtx_owned(br->br_lock))
		panic("lock not held on single consumer dequeue");
#endif	
	/*
	 * I believe it is safe to not have a memory barrier
	 * here because we control cons and tail is worst case
	 * a lagging indicator so we worst case we might
	 * return NULL immediately after a buffer has been enqueued
	 */
	if (br->br_cons_head == br->br_prod_tail)
		return (NULL);
	
	return (br->br_ring[br->br_cons_head]);
}

static __inline int
buf_ring_full(struct buf_ring *br)
{

	return (((br->br_prod_head + 1) & br->br_prod_mask) == br->br_cons_tail);
}

static __inline int
buf_ring_empty(struct buf_ring *br)
{

	return (br->br_cons_head == br->br_prod_tail);
}

static __inline int
buf_ring_count(struct buf_ring *br)
{

	return ((br->br_prod_size + br->br_prod_tail - br->br_cons_tail)
	    & br->br_prod_mask);
}

struct buf_ring *buf_ring_alloc(int count, struct malloc_type *type, int flags,
    struct mtx *);
void buf_ring_free(struct buf_ring *br, struct malloc_type *type);



#endif
