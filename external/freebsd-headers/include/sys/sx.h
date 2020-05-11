/*-
 * Copyright (c) 2007 Attilio Rao <attilio@freebsd.org>
 * Copyright (c) 2001 Jason Evans <jasone@freebsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice(s), this list of conditions and the following disclaimer as
 *    the first lines of this file unmodified other than the possible 
 *    addition of one or more copyright notices.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice(s), this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * $FreeBSD: release/9.0.0/sys/sys/sx.h 219819 2011-03-21 09:40:01Z jeff $
 */

#ifndef	_SYS_SX_H_
#define	_SYS_SX_H_

#include <sys/_lock.h>
#include <sys/_sx.h>

#ifdef	_KERNEL
#include <sys/pcpu.h>
#include <sys/lock_profile.h>
#include <sys/lockstat.h>
#include <machine/atomic.h>
#endif

/*
 * In general, the sx locks and rwlocks use very similar algorithms.
 * The main difference in the implementations is how threads are
 * blocked when a lock is unavailable.  For this, sx locks use sleep
 * queues which do not support priority propagation, and rwlocks use
 * turnstiles which do.
 *
 * The sx_lock field consists of several fields.  The low bit
 * indicates if the lock is locked with a shared or exclusive lock.  A
 * value of 0 indicates an exclusive lock, and a value of 1 indicates
 * a shared lock.  Bit 1 is a boolean indicating if there are any
 * threads waiting for a shared lock.  Bit 2 is a boolean indicating
 * if there are any threads waiting for an exclusive lock.  Bit 3 is a
 * boolean indicating if an exclusive lock is recursively held.  The
 * rest of the variable's definition is dependent on the value of the
 * first bit.  For an exclusive lock, it is a pointer to the thread
 * holding the lock, similar to the mtx_lock field of mutexes.  For
 * shared locks, it is a count of read locks that are held.
 *
 * When the lock is not locked by any thread, it is encoded as a
 * shared lock with zero waiters.
 */

#define	SX_LOCK_SHARED			0x01
#define	SX_LOCK_SHARED_WAITERS		0x02
#define	SX_LOCK_EXCLUSIVE_WAITERS	0x04
#define	SX_LOCK_RECURSED		0x08
#define	SX_LOCK_FLAGMASK						\
	(SX_LOCK_SHARED | SX_LOCK_SHARED_WAITERS |			\
	SX_LOCK_EXCLUSIVE_WAITERS | SX_LOCK_RECURSED)

#define	SX_OWNER(x)			((x) & ~SX_LOCK_FLAGMASK)
#define	SX_SHARERS_SHIFT		4
#define	SX_SHARERS(x)			(SX_OWNER(x) >> SX_SHARERS_SHIFT)
#define	SX_SHARERS_LOCK(x)						\
	((x) << SX_SHARERS_SHIFT | SX_LOCK_SHARED)
#define	SX_ONE_SHARER			(1 << SX_SHARERS_SHIFT)

#define	SX_LOCK_UNLOCKED		SX_SHARERS_LOCK(0)
#define	SX_LOCK_DESTROYED						\
	(SX_LOCK_SHARED_WAITERS | SX_LOCK_EXCLUSIVE_WAITERS)

#ifdef _KERNEL

/*
 * Function prototipes.  Routines that start with an underscore are not part
 * of the public interface and are wrappered with a macro.
 */
void	sx_sysinit(void *arg);
#define	sx_init(sx, desc)	sx_init_flags((sx), (desc), 0)
void	sx_init_flags(struct sx *sx, const char *description, int opts);
void	sx_destroy(struct sx *sx);
int	_sx_slock(struct sx *sx, int opts, const char *file, int line);
int	_sx_xlock(struct sx *sx, int opts, const char *file, int line);
int	_sx_try_slock(struct sx *sx, const char *file, int line);
int	_sx_try_xlock(struct sx *sx, const char *file, int line);
void	_sx_sunlock(struct sx *sx, const char *file, int line);
void	_sx_xunlock(struct sx *sx, const char *file, int line);
int	_sx_try_upgrade(struct sx *sx, const char *file, int line);
void	_sx_downgrade(struct sx *sx, const char *file, int line);
int	_sx_xlock_hard(struct sx *sx, uintptr_t tid, int opts,
	    const char *file, int line);
int	_sx_slock_hard(struct sx *sx, int opts, const char *file, int line);
void	_sx_xunlock_hard(struct sx *sx, uintptr_t tid, const char *file, int
	    line);
void	_sx_sunlock_hard(struct sx *sx, const char *file, int line);
#if defined(INVARIANTS) || defined(INVARIANT_SUPPORT)
void	_sx_assert(struct sx *sx, int what, const char *file, int line);
#endif
#ifdef DDB
int	sx_chain(struct thread *td, struct thread **ownerp);
#endif

struct sx_args {
	struct sx 	*sa_sx;
	const char	*sa_desc;
	int		sa_flags;
};

#define	SX_SYSINIT_FLAGS(name, sxa, desc, flags)			\
	static struct sx_args name##_args = {				\
		(sxa),							\
		(desc),							\
		(flags)							\
	};								\
	SYSINIT(name##_sx_sysinit, SI_SUB_LOCK, SI_ORDER_MIDDLE,	\
	    sx_sysinit, &name##_args);					\
	SYSUNINIT(name##_sx_sysuninit, SI_SUB_LOCK, SI_ORDER_MIDDLE,	\
	    sx_destroy, (sxa))

#define	SX_SYSINIT(name, sxa, desc)	SX_SYSINIT_FLAGS(name, sxa, desc, 0)

/*
 * Full lock operations that are suitable to be inlined in non-debug kernels.
 * If the lock can't be acquired or released trivially then the work is
 * deferred to 'tougher' functions.
 */

///* Acquire an exclusive lock. */
//static __inline int
//__sx_xlock(struct sx *sx, struct thread *td, int opts, const char *file,
//	int line);
//
///* Release an exclusive lock. */
//static __inline void
//__sx_xunlock(struct sx *sx, struct thread *td, const char *file, int line);
//
///* Acquire a shared lock. */
//static __inline int
//__sx_slock(struct sx *sx, int opts, const char *file, int line);

/*
 * Release a shared lock.  We can just drop a single shared lock so
 * long as we aren't trying to drop the last shared lock when other
 * threads are waiting for an exclusive lock.  This takes advantage of
 * the fact that an unlocked lock is encoded as a shared lock with a
 * count of 0.
 */
//static __inline void
//__sx_sunlock(struct sx *sx, const char *file, int line);

/*
 * Public interface for lock operations.
 */
#ifndef LOCK_DEBUG
#error	"LOCK_DEBUG not defined, include <sys/lock.h> before <sys/sx.h>"
#endif
#if	(LOCK_DEBUG > 0) || defined(SX_NOINLINE)
#define	sx_xlock(sx)		(void)_sx_xlock((sx), 0, LOCK_FILE, LOCK_LINE)
#define	sx_xlock_sig(sx)						\
	_sx_xlock((sx), SX_INTERRUPTIBLE, LOCK_FILE, LOCK_LINE)
#define	sx_xunlock(sx)		_sx_xunlock((sx), LOCK_FILE, LOCK_LINE)
#define	sx_slock(sx)		(void)(*_sx_slock)((sx), 0, LOCK_FILE, LOCK_LINE)
#define	sx_slock_sig(sx)						\
	_sx_slock((sx), SX_INTERRUPTIBLE, LOCK_FILE, LOCK_LINE)
#define	sx_sunlock(sx)		(*_sx_sunlock)((sx), LOCK_FILE, LOCK_LINE)
#else
#define	sx_xlock(sx)							\
	(void)__sx_xlock((sx), curthread, 0, LOCK_FILE, LOCK_LINE)
#define	sx_xlock_sig(sx)						\
	__sx_xlock((sx), curthread, SX_INTERRUPTIBLE, LOCK_FILE, LOCK_LINE)
#define	sx_xunlock(sx)							\
	(*__sx_xunlock)((sx), curthread, LOCK_FILE, LOCK_LINE)
#define	sx_slock(sx)		(void)__sx_slock((sx), 0, LOCK_FILE, LOCK_LINE)
#define	sx_slock_sig(sx)						\
	__sx_slock((sx), SX_INTERRUPTIBLE, LOCK_FILE, LOCK_LINE)
#define	sx_sunlock(sx)		__sx_sunlock((sx), LOCK_FILE, LOCK_LINE)
#endif	/* LOCK_DEBUG > 0 || SX_NOINLINE */
#define	sx_try_slock(sx)	_sx_try_slock((sx), LOCK_FILE, LOCK_LINE)
#define	sx_try_xlock(sx)	_sx_try_xlock((sx), LOCK_FILE, LOCK_LINE)
#define	sx_try_upgrade(sx)	_sx_try_upgrade((sx), LOCK_FILE, LOCK_LINE)
#define	sx_downgrade(sx)	_sx_downgrade((sx), LOCK_FILE, LOCK_LINE)

/*
 * Return a pointer to the owning thread if the lock is exclusively
 * locked.
 */
#define	sx_xholder(sx)							\
	((sx)->sx_lock & SX_LOCK_SHARED ? NULL :			\
	(struct thread *)SX_OWNER((sx)->sx_lock))

#define	sx_xlocked(sx)							\
	(((sx)->sx_lock & ~(SX_LOCK_FLAGMASK & ~SX_LOCK_SHARED)) ==	\
	    (uintptr_t)curthread)

#define	sx_unlock(sx) do {						\
	if (sx_xlocked(sx))						\
		sx_xunlock(sx);						\
	else								\
		sx_sunlock(sx);						\
} while (0)

#define	sx_sleep(chan, sx, pri, wmesg, timo)				\
	_sleep((chan), &(sx)->lock_object, (pri), (wmesg), (timo))

/*
 * Options passed to sx_init_flags().
 */
#define	SX_DUPOK		0x01
#define	SX_NOPROFILE		0x02
#define	SX_NOWITNESS		0x04
#define	SX_QUIET		0x08
#define	SX_NOADAPTIVE		0x10
#define	SX_RECURSE		0x20

/*
 * Options passed to sx_*lock_hard().
 */
#define	SX_INTERRUPTIBLE	0x40

//#if defined(INVARIANTS) || defined(INVARIANT_SUPPORT)
//#define	SA_LOCKED		LA_LOCKED
//#define	SA_SLOCKED		LA_SLOCKED
//#define	SA_XLOCKED		LA_XLOCKED
//#define	SA_UNLOCKED		LA_UNLOCKED
//#define	SA_RECURSED		LA_RECURSED
//#define	SA_NOTRECURSED		LA_NOTRECURSED
//
///* Backwards compatability. */
//#define	SX_LOCKED		LA_LOCKED
//#define	SX_SLOCKED		LA_SLOCKED
//#define	SX_XLOCKED		LA_XLOCKED
//#define	SX_UNLOCKED		LA_UNLOCKED
//#define	SX_RECURSED		LA_RECURSED
//#define	SX_NOTRECURSED		LA_NOTRECURSED
//#endif

//#ifdef INVARIANTS
//#define	sx_assert(sx, what)	_sx_assert((sx), (what), LOCK_FILE, LOCK_LINE)
//#else
//#define	sx_assert(sx, what)	(void)0
//#endif

#endif /* _KERNEL */

#endif /* !_SYS_SX_H_ */
