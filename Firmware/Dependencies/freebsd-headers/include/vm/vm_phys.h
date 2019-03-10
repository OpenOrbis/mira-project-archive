/*-
 * Copyright (c) 2002-2006 Rice University
 * Copyright (c) 2007 Alan L. Cox <alc@cs.rice.edu>
 * All rights reserved.
 *
 * This software was developed for the FreeBSD Project by Alan L. Cox,
 * Olivier Crameri, Peter Druschel, Sitaram Iyer, and Juan Navarro.
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD: release/9.0.0/sys/vm/vm_phys.h 210550 2010-07-27 20:33:50Z jhb $
 */

/*
 *	Physical memory system definitions
 */

#ifndef	_VM_PHYS_H_
#define	_VM_PHYS_H_

#ifdef _KERNEL

/* Domains must be dense (non-sparse) and zero-based. */
struct mem_affinity {
	vm_paddr_t start;
	vm_paddr_t end;
	int domain;
};

extern struct mem_affinity *mem_affinity;

void vm_phys_add_page(vm_paddr_t pa);
vm_page_t vm_phys_alloc_contig(unsigned long npages,
    vm_paddr_t low, vm_paddr_t high,
    unsigned long alignment, unsigned long boundary);
vm_page_t vm_phys_alloc_freelist_pages(int flind, int pool, int order);
vm_page_t vm_phys_alloc_pages(int pool, int order);
vm_paddr_t vm_phys_bootstrap_alloc(vm_size_t size, unsigned long alignment);
void vm_phys_free_pages(vm_page_t m, int order);
void vm_phys_init(void);
void vm_phys_set_pool(int pool, vm_page_t m, int order);
boolean_t vm_phys_unfree_page(vm_page_t m);
boolean_t vm_phys_zero_pages_idle(void);

#endif	/* _KERNEL */
#endif	/* !_VM_PHYS_H_ */
