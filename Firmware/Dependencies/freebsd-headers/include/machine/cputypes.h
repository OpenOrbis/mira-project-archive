/*-
 * Copyright (c) 1993 Christopher G. Demetriou
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
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD: release/9.0.0/sys/amd64/include/cputypes.h 186797 2009-01-05 21:51:49Z jkim $
 */

#ifndef _MACHINE_CPUTYPES_H_
#define	_MACHINE_CPUTYPES_H_

/*
 * Classes of processor.
 */
#define	CPUCLASS_X86		0	/* X86 */
#define	CPUCLASS_K8		1	/* K8 AMD64 class */

/*
 * Kinds of processor.
 */
#define	CPU_X86			0	/* Intel */
#define	CPU_CLAWHAMMER		1	/* AMD Clawhammer */
#define	CPU_SLEDGEHAMMER	2	/* AMD Sledgehammer */

/*
 * Vendors of processor.
 */
#define	CPU_VENDOR_AMD		0x1022		/* AMD */
#define	CPU_VENDOR_IDT		0x111d		/* Centaur/IDT/VIA */
#define	CPU_VENDOR_INTEL	0x8086		/* Intel */
#define	CPU_VENDOR_CENTAUR	CPU_VENDOR_IDT

#ifndef LOCORE
extern int	cpu;
extern int	cpu_class;
#endif

#endif /* !_MACHINE_CPUTYPES_H_ */
