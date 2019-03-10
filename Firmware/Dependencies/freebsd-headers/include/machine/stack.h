/*-
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
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
 * $FreeBSD: release/9.0.0/sys/amd64/include/stack.h 179886 2008-06-20 05:22:09Z alc $
 */

#ifndef _MACHINE_STACK_H_
#define	_MACHINE_STACK_H_

/*
 * Stack trace.
 */
#define	INKERNEL(va) (((va) >= DMAP_MIN_ADDRESS && (va) < DMAP_MAX_ADDRESS) \
	    || ((va) >= VM_MIN_KERNEL_ADDRESS && (va) < VM_MAX_KERNEL_ADDRESS))

struct amd64_frame {
	struct amd64_frame	*f_frame;
	long			f_retaddr;
	long			f_arg0;
};

#endif /* !_MACHINE_STACK_H_ */
