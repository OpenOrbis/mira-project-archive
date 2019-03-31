/*-
 * Copyright (c) 2009 Robert N. M. Watson
 * All rights reserved.
 *
 * This software was developed at the University of Cambridge Computer
 * Laboratory with support from a grant from Google, Inc.
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
 * $FreeBSD: release/9.0.0/sys/sys/procdesc.h 224987 2011-08-18 22:51:30Z jonathan $
 */

#ifndef _SYS_PROCDESC_H_
#define	_SYS_PROCDESC_H_

#ifdef _KERNEL
#include <sys/selinfo.h>	/* struct selinfo */
#include <sys/_lock.h>
#include <sys/_mutex.h>

/*-
 * struct procdesc describes a process descriptor, and essentially consists
 * of two pointers -- one to the file descriptor, and one to the process.
 * When both become NULL, the process descriptor will be freed.  An important
 * invariant is that there is only ever one process descriptor for a process,
 * so a single file pointer will suffice.
 *
 * Locking key:
 * (c) - Constant after initial setup.
 * (p) - Protected by the process descriptor mutex.
 * (r) - Atomic eference count.
 * (s) - Protected by selinfo.
 * (t) - Protected by the proctree_lock
 */
struct proc;
struct sigio;
struct procdesc {
	/*
	 * Basic process descriptor state: the process, a cache of its pid to
	 * satisfy queries after the process exits, and process descriptor
	 * refcount.
	 */
	struct proc	*pd_proc;		/* (t) Process. */
	pid_t		 pd_pid;		/* (c) Cached pid. */
	u_int		 pd_refcount;		/* (r) Reference count. */

	/*
	 * In-flight data and notification of events.
	 */
	int		 pd_flags;		/* (p) PD_ flags. */
	struct selinfo	 pd_selinfo;		/* (p) Event notification. */
	struct mtx	 pd_lock;		/* Protect data + events. */
};

/*
 * Locking macros for the procdesc itself.
 */
#define	PROCDESC_LOCK_DESTROY(pd)	mtx_destroy(&(pd)->pd_lock)
#define	PROCDESC_LOCK_INIT(pd)	mtx_init(&(pd)->pd_lock, "procdesc", NULL, \
				    MTX_DEF)
#define	PROCDESC_LOCK(pd)	mtx_lock(&(pd)->pd_lock)
#define	PROCDESC_UNLOCK(pd)	mtx_unlock(&(pd)->pd_lock)

/*
 * Flags for the pd_flags field.
 */
#define	PDF_CLOSED	0x00000001	/* Descriptor has closed. */
#define	PDF_SELECTED	0x00000002	/* Issue selwakeup(). */
#define	PDF_EXITED	0x00000004	/* Process exited. */
#define	PDF_DAEMON	0x00000008	/* Don't exit when procdesc closes. */

/*
 * In-kernel interfaces to process descriptors.
 */
int	 procdesc_exit(struct proc *);
int	 procdesc_find(struct thread *, int fd, cap_rights_t, struct proc **);
int	 kern_pdgetpid(struct thread *, int fd, cap_rights_t, pid_t *pidp);
void	 procdesc_new(struct proc *, int);
void	 procdesc_finit(struct procdesc *, struct file *);
pid_t	 procdesc_pid(struct file *);
void	 procdesc_reap(struct proc *);

#else /* !_KERNEL */

/*
 * Process descriptor system calls.
 */
struct rusage;
int	 pdfork(int *, int);
int	 pdkill(int, int);
int	 pdgetpid(int, pid_t *);

#endif /* _KERNEL */

/*
 * Flags which can be passed to pdfork(2).
 */
#define	PD_DAEMON	0x00000001	/* Don't exit when procdesc closes. */

#endif /* !_SYS_PROCDESC_H_ */
