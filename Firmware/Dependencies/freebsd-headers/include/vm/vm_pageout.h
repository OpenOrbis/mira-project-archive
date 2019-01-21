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
 *	from: @(#)vm_pageout.h	8.2 (Berkeley) 1/12/94
 *
 *
 * Copyright (c) 1987, 1990 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Author: Avadis Tevanian, Jr.
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
 * $FreeBSD: release/9.0.0/sys/vm/vm_pageout.h 215471 2010-11-18 21:09:02Z kib $
 */

#ifndef _VM_VM_PAGEOUT_H_
#define _VM_VM_PAGEOUT_H_

/*
 *	Header file for pageout daemon.
 */

/*
 *	Exported data structures.
 */

extern int vm_page_max_wired;
extern int vm_pages_needed;	/* should be some "event" structure */
extern int vm_pageout_pages_needed;
extern int vm_pageout_deficit;
extern int vm_pageout_page_count;

/*
 * Swap out requests
 */
#define VM_SWAP_NORMAL 1
#define VM_SWAP_IDLE 2

#define	VM_OOM_MEM	1
#define	VM_OOM_SWAPZ	2

/*
 *	Exported routines.
 */

/*
 *	Signal pageout-daemon and wait for it.
 */

extern void pagedaemon_wakeup(void);
#define VM_WAIT vm_wait()
#define VM_WAITPFAULT vm_waitpfault()
extern void vm_wait(void);
extern void vm_waitpfault(void);

#ifdef _KERNEL
boolean_t vm_pageout_fallback_object_lock(vm_page_t, vm_page_t *);
int vm_pageout_flush(vm_page_t *, int, int, int, int *);
void vm_pageout_oom(int shortage);
boolean_t vm_pageout_page_lock(vm_page_t, vm_page_t *);
void vm_contig_grow_cache(int, vm_paddr_t, vm_paddr_t);
#endif
#endif	/* _VM_VM_PAGEOUT_H_ */
