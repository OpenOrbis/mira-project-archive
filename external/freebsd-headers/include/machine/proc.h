/*-
 * Copyright (c) 1991 Regents of the University of California.
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
 *	from: @(#)proc.h	7.1 (Berkeley) 5/15/91
 * $FreeBSD: release/9.0.0/sys/amd64/include/proc.h 208453 2010-05-23 18:32:02Z kib $
 */

#ifndef _MACHINE_PROC_H_
#define	_MACHINE_PROC_H_

#include <machine/segments.h>

struct proc_ldt {
	caddr_t ldt_base;
	int     ldt_refcnt;
};

/*
 * Machine-dependent part of the proc structure for AMD64.
 */
struct mdthread {
	int	md_spinlock_count;	/* (k) */
	register_t md_saved_flags;	/* (k) */
};

struct mdproc {
	struct proc_ldt *md_ldt;	/* (t) per-process ldt */
	struct system_segment_descriptor md_ldt_sd;
};

#define	KINFO_PROC_SIZE 1088
#define	KINFO_PROC32_SIZE 768

#ifdef	_KERNEL

/* Get the current kernel thread stack usage. */
#define GET_STACK_USAGE(total, used) do {				\
	struct thread	*td = curthread;				\
	(total) = td->td_kstack_pages * PAGE_SIZE;			\
	(used) = (char *)td->td_kstack +				\
	    td->td_kstack_pages * PAGE_SIZE -				\
	    (char *)&td;						\
} while (0)

void set_user_ldt(struct mdproc *);
struct proc_ldt *user_ldt_alloc(struct proc *, int);
void user_ldt_free(struct thread *);
void user_ldt_deref(struct proc_ldt *);
struct sysarch_args;
int sysarch_ldt(struct thread *td, struct sysarch_args *uap, int uap_space);
int amd64_set_ldt_data(struct thread *td, int start, int num,
    struct user_segment_descriptor *descs);

extern struct mtx dt_lock;
extern int max_ldt_segment;

struct syscall_args {
	u_int code;
	struct sysent *callp;
	register_t args[8];
	int narg;
};
#define	HAVE_SYSCALL_ARGS_DEF 1

#endif  /* _KERNEL */

#endif /* !_MACHINE_PROC_H_ */
