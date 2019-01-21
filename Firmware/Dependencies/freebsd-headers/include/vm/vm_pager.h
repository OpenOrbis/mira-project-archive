/*-
 * Copyright (c) 1990 University of Utah.
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
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
 *	@(#)vm_pager.h	8.4 (Berkeley) 1/12/94
 * $FreeBSD: release/9.0.0/sys/vm/vm_pager.h 217529 2011-01-18 04:54:43Z alc $
 */

/*
 * Pager routine interface definition.
 */

#ifndef	_VM_PAGER_
#define	_VM_PAGER_

#include <sys/queue.h>

TAILQ_HEAD(pagerlst, vm_object);

typedef void pgo_init_t(void);
typedef vm_object_t pgo_alloc_t(void *, vm_ooffset_t, vm_prot_t, vm_ooffset_t,
    struct ucred *);
typedef void pgo_dealloc_t(vm_object_t);
typedef int pgo_getpages_t(vm_object_t, vm_page_t *, int, int);
typedef void pgo_putpages_t(vm_object_t, vm_page_t *, int, int, int *);
typedef boolean_t pgo_haspage_t(vm_object_t, vm_pindex_t, int *, int *);
typedef void pgo_pageunswapped_t(vm_page_t);

struct pagerops {
	pgo_init_t	*pgo_init;		/* Initialize pager. */
	pgo_alloc_t	*pgo_alloc;		/* Allocate pager. */
	pgo_dealloc_t	*pgo_dealloc;		/* Disassociate. */
	pgo_getpages_t	*pgo_getpages;		/* Get (read) page. */
	pgo_putpages_t	*pgo_putpages;		/* Put (write) page. */
	pgo_haspage_t	*pgo_haspage;		/* Does pager have page? */
	pgo_pageunswapped_t *pgo_pageunswapped;
};

extern struct pagerops defaultpagerops;
extern struct pagerops swappagerops;
extern struct pagerops vnodepagerops;
extern struct pagerops devicepagerops;
extern struct pagerops physpagerops;
extern struct pagerops sgpagerops;

/*
 * get/put return values
 * OK	 operation was successful
 * BAD	 specified data was out of the accepted range
 * FAIL	 specified data was in range, but doesn't exist
 * PEND	 operations was initiated but not completed
 * ERROR error while accessing data that is in range and exists
 * AGAIN temporary resource shortage prevented operation from happening
 */
#define	VM_PAGER_OK	0
#define	VM_PAGER_BAD	1
#define	VM_PAGER_FAIL	2
#define	VM_PAGER_PEND	3
#define	VM_PAGER_ERROR	4
#define VM_PAGER_AGAIN	5

#define	VM_PAGER_PUT_SYNC		0x0001
#define	VM_PAGER_PUT_INVAL		0x0002
#define VM_PAGER_CLUSTER_OK		0x0008

#ifdef _KERNEL

extern vm_map_t pager_map;
extern struct pagerops *pagertab[];
extern struct mtx pbuf_mtx;

vm_object_t vm_pager_allocate(objtype_t, void *, vm_ooffset_t, vm_prot_t,
    vm_ooffset_t, struct ucred *);
void vm_pager_bufferinit(void);
void vm_pager_deallocate(vm_object_t);
static __inline int vm_pager_get_pages(vm_object_t, vm_page_t *, int, int);
static __inline boolean_t vm_pager_has_page(vm_object_t, vm_pindex_t, int *, int *);
void vm_pager_init(void);
vm_object_t vm_pager_object_lookup(struct pagerlst *, void *);

/*
 *	vm_page_get_pages:
 *
 *	Retrieve pages from the VM system in order to map them into an object
 *	( or into VM space somewhere ).  If the pagein was successful, we
 *	must fully validate it.
 */
static __inline int
vm_pager_get_pages(
	vm_object_t object,
	vm_page_t *m,
	int count,
	int reqpage
) {
	int r;

	VM_OBJECT_LOCK_ASSERT(object, MA_OWNED);
	r = (*pagertab[object->type]->pgo_getpages)(object, m, count, reqpage);
	if (r == VM_PAGER_OK && m[reqpage]->valid != VM_PAGE_BITS_ALL) {
		vm_page_zero_invalid(m[reqpage], TRUE);
	}
	return (r);
}

static __inline void
vm_pager_put_pages(
	vm_object_t object,
	vm_page_t *m,
	int count,
	int flags,
	int *rtvals
) {

	VM_OBJECT_LOCK_ASSERT(object, MA_OWNED);
	(*pagertab[object->type]->pgo_putpages)
	    (object, m, count, flags, rtvals);
}

/*
 *	vm_pager_haspage
 *
 *	Check to see if an object's pager has the requested page.  The
 *	object's pager will also set before and after to give the caller
 *	some idea of the number of pages before and after the requested
 *	page can be I/O'd efficiently.
 *
 *	The object must be locked.
 */
static __inline boolean_t
vm_pager_has_page(
	vm_object_t object,
	vm_pindex_t offset, 
	int *before,
	int *after
) {
	boolean_t ret;

	VM_OBJECT_LOCK_ASSERT(object, MA_OWNED);
	ret = (*pagertab[object->type]->pgo_haspage)
	    (object, offset, before, after);
	return (ret);
} 

/* 
 *      vm_pager_page_unswapped
 * 
 *	Destroy swap associated with the page.
 * 
 *	The object containing the page must be locked.
 *      This function may not block.
 *
 *	XXX: A much better name would be "vm_pager_page_dirtied()"
 *	XXX: It is not obvious if this could be profitably used by any
 *	XXX: pagers besides the swap_pager or if it should even be a
 *	XXX: generic pager_op in the first place.
 */
static __inline void
vm_pager_page_unswapped(vm_page_t m)
{

	VM_OBJECT_LOCK_ASSERT(m->object, MA_OWNED);
	if (pagertab[m->object->type]->pgo_pageunswapped)
		(*pagertab[m->object->type]->pgo_pageunswapped)(m);
}

#endif				/* _KERNEL */
#endif				/* _VM_PAGER_ */
