/*-
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * The Mach Operating System project at Carnegie-Mellon University.
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
 *	from: @(#)vm_object.h	8.3 (Berkeley) 1/12/94
 *
 *
 * Copyright (c) 1987, 1990 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Authors: Avadis Tevanian, Jr., Michael Wayne Young
 *
 * Permission to use, copy, modify and distribute this software and
 * its documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 *
 * $FreeBSD: release/9.0.0/sys/vm/vm_object.h 223677 2011-06-29 16:40:41Z alc $
 */

/*
 *	Virtual memory object module definitions.
 */

#ifndef	_VM_OBJECT_
#define	_VM_OBJECT_

#include <sys/queue.h>
#include <sys/_lock.h>
#include <sys/_mutex.h>

/*
 *	Types defined:
 *
 *	vm_object_t		Virtual memory object.
 *
 * List of locks
 *	(c)	const until freed
 *
 */

struct vm_object {
	struct mtx mtx;
	TAILQ_ENTRY(vm_object) object_list; /* list of all objects */
	LIST_HEAD(, vm_object) shadow_head; /* objects that this is a shadow for */
	LIST_ENTRY(vm_object) shadow_list; /* chain of shadow objects */
	TAILQ_HEAD(, vm_page) memq;	/* list of resident pages */
	vm_page_t root;			/* root of the resident page splay tree */
	vm_pindex_t size;		/* Object size */
	int generation;			/* generation ID */
	int ref_count;			/* How many refs?? */
	int shadow_count;		/* how many objects that this is a shadow for */
	vm_memattr_t memattr;		/* default memory attribute for pages */
	objtype_t type;			/* type of pager */
	u_short flags;			/* see below */
	u_short pg_color;		/* (c) color of first page in obj */
	u_short paging_in_progress;	/* Paging (in or out) so don't collapse or destroy */
	int resident_page_count;	/* number of resident pages */
	struct vm_object *backing_object; /* object that I'm a shadow of */
	vm_ooffset_t backing_object_offset;/* Offset in backing object */
	TAILQ_ENTRY(vm_object) pager_object_list; /* list of all objects of this pager type */
	LIST_HEAD(, vm_reserv) rvq;	/* list of reservations */
	vm_page_t cache;		/* root of the cache page splay tree */
	void *handle;
	union {
		/*
		 * VNode pager
		 *
		 *	vnp_size - current size of file
		 */
		struct {
			off_t vnp_size;
		} vnp;

		/*
		 * Device pager
		 *
		 *	devp_pglist - list of allocated pages
		 */
		struct {
			TAILQ_HEAD(, vm_page) devp_pglist;
		} devp;

		/*
		 * SG pager
		 *
		 *	sgp_pglist - list of allocated pages
		 */
		struct {
			TAILQ_HEAD(, vm_page) sgp_pglist;
		} sgp;

		/*
		 * Swap pager
		 *
		 *	swp_bcount - number of swap 'swblock' metablocks, each
		 *		     contains up to 16 swapblk assignments.
		 *		     see vm/swap_pager.h
		 */
		struct {
			int swp_bcount;
		} swp;
	} un_pager;
	struct ucred *cred;
	vm_ooffset_t charge;
};

/*
 * Flags
 */
#define OBJ_ACTIVE	0x0004		/* active objects */
#define OBJ_DEAD	0x0008		/* dead objects (during rundown) */
#define	OBJ_NOSPLIT	0x0010		/* dont split this object */
#define OBJ_PIPWNT	0x0040		/* paging in progress wanted */
#define OBJ_MIGHTBEDIRTY 0x0100		/* object might be dirty, only for vnode */
#define	OBJ_COLORED	0x1000		/* pg_color is defined */
#define	OBJ_ONEMAPPING	0x2000		/* One USE (a single, non-forked) mapping flag */
#define	OBJ_DISCONNECTWNT 0x4000	/* disconnect from vnode wanted */

#define IDX_TO_OFF(idx) (((vm_ooffset_t)(idx)) << PAGE_SHIFT)
#define OFF_TO_IDX(off) ((vm_pindex_t)(((vm_ooffset_t)(off)) >> PAGE_SHIFT))

#ifdef	_KERNEL

#define OBJPC_SYNC	0x1			/* sync I/O */
#define OBJPC_INVAL	0x2			/* invalidate */
#define OBJPC_NOSYNC	0x4			/* skip if PG_NOSYNC */

/*
 * The following options are supported by vm_object_page_remove().
 */
#define	OBJPR_CLEANONLY	0x1		/* Don't remove dirty pages. */
#define	OBJPR_NOTMAPPED	0x2		/* Don't unmap pages. */

TAILQ_HEAD(object_q, vm_object);

extern struct object_q vm_object_list;	/* list of allocated objects */
extern struct mtx vm_object_list_mtx;	/* lock for object list and count */

extern struct vm_object kernel_object_store;
extern struct vm_object kmem_object_store;

#define	kernel_object	(&kernel_object_store)
#define	kmem_object	(&kmem_object_store)

#define	VM_OBJECT_LOCK(object)		mtx_lock(&(object)->mtx)
#define	VM_OBJECT_LOCK_ASSERT(object, type) \
					mtx_assert(&(object)->mtx, (type))
#define	VM_OBJECT_LOCK_INIT(object, type) \
					mtx_init(&(object)->mtx, "vm object", \
					    (type), MTX_DEF | MTX_DUPOK)
#define	VM_OBJECT_LOCKED(object)	mtx_owned(&(object)->mtx)
#define	VM_OBJECT_MTX(object)		(&(object)->mtx)
#define	VM_OBJECT_TRYLOCK(object)	mtx_trylock(&(object)->mtx)
#define	VM_OBJECT_UNLOCK(object)	mtx_unlock(&(object)->mtx)

/*
 *	The object must be locked or thread private.
 */
static __inline void
vm_object_set_flag(vm_object_t object, u_short bits)
{

	object->flags |= bits;
}

void vm_object_clear_flag(vm_object_t object, u_short bits);
void vm_object_pip_add(vm_object_t object, short i);
void vm_object_pip_subtract(vm_object_t object, short i);
void vm_object_pip_wakeup(vm_object_t object);
void vm_object_pip_wakeupn(vm_object_t object, short i);
void vm_object_pip_wait(vm_object_t object, char *waitid);

vm_object_t vm_object_allocate (objtype_t, vm_pindex_t);
void _vm_object_allocate (objtype_t, vm_pindex_t, vm_object_t);
boolean_t vm_object_coalesce(vm_object_t, vm_ooffset_t, vm_size_t, vm_size_t,
   boolean_t);
void vm_object_collapse (vm_object_t);
void vm_object_deallocate (vm_object_t);
void vm_object_destroy (vm_object_t);
void vm_object_terminate (vm_object_t);
void vm_object_set_writeable_dirty (vm_object_t);
void vm_object_init (void);
void vm_object_page_clean(vm_object_t object, vm_ooffset_t start,
    vm_ooffset_t end, int flags);
void vm_object_page_remove(vm_object_t object, vm_pindex_t start,
    vm_pindex_t end, int options);
boolean_t vm_object_populate(vm_object_t, vm_pindex_t, vm_pindex_t);
void vm_object_print(long addr, boolean_t have_addr, long count, char *modif);
void vm_object_reference (vm_object_t);
void vm_object_reference_locked(vm_object_t);
int  vm_object_set_memattr(vm_object_t object, vm_memattr_t memattr);
void vm_object_shadow (vm_object_t *, vm_ooffset_t *, vm_size_t);
void vm_object_split(vm_map_entry_t);
void vm_object_sync(vm_object_t, vm_ooffset_t, vm_size_t, boolean_t,
    boolean_t);
void vm_object_madvise (vm_object_t, vm_pindex_t, int, int);
#endif				/* _KERNEL */

#endif				/* _VM_OBJECT_ */
