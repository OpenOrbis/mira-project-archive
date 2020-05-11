/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * William Jolitz.
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
 *	from: @(#)npx.h	5.3 (Berkeley) 1/18/91
 * $FreeBSD: release/9.0.0/sys/amd64/include/fpu.h 215865 2010-11-26 14:50:42Z kib $
 */

/*
 * Floating Point Data Structures and Constants
 * W. Jolitz 1/90
 */

#ifndef _MACHINE_FPU_H_
#define	_MACHINE_FPU_H_

/* Contents of each x87 floating point accumulator */
struct fpacc87 {
	u_char	fp_bytes[10];
};

/* Contents of each SSE extended accumulator */
struct  xmmacc {
	u_char	xmm_bytes[16];
};

struct  envxmm {
	u_int16_t	en_cw;		/* control word (16bits) */
	u_int16_t	en_sw;		/* status word (16bits) */
	u_int8_t	en_tw;		/* tag word (8bits) */
	u_int8_t	en_zero;
	u_int16_t	en_opcode;	/* opcode last executed (11 bits ) */
	u_int64_t	en_rip;		/* floating point instruction pointer */
	u_int64_t	en_rdp;		/* floating operand pointer */
	u_int32_t	en_mxcsr;	/* SSE sontorol/status register */
	u_int32_t	en_mxcsr_mask;	/* valid bits in mxcsr */
};

struct  savefpu {
	struct	envxmm	sv_env;
	struct {
		struct fpacc87	fp_acc;
		u_char		fp_pad[6];      /* padding */
	} sv_fp[8];
	struct xmmacc	sv_xmm[16];
	u_char sv_pad[96];
} __aligned(16);

#ifdef _KERNEL
struct fpu_kern_ctx {
	struct savefpu hwstate;
	struct savefpu *prev;
	uint32_t flags;
};
#define	FPU_KERN_CTX_FPUINITDONE 0x01

#define	PCB_USER_FPU(pcb) (((pcb)->pcb_flags & PCB_KERNFPU) == 0)
#endif

/*
 * The hardware default control word for i387's and later coprocessors is
 * 0x37F, giving:
 *
 *	round to nearest
 *	64-bit precision
 *	all exceptions masked.
 *
 * FreeBSD/i386 uses 53 bit precision for things like fadd/fsub/fsqrt etc
 * because of the difference between memory and fpu register stack arguments.
 * If its using an intermediate fpu register, it has 80/64 bits to work
 * with.  If it uses memory, it has 64/53 bits to work with.  However,
 * gcc is aware of this and goes to a fair bit of trouble to make the
 * best use of it.
 *
 * This is mostly academic for AMD64, because the ABI prefers the use
 * SSE2 based math.  For FreeBSD/amd64, we go with the default settings.
 */
#define	__INITIAL_FPUCW__	0x037F
#define	__INITIAL_FPUCW_I386__	0x127F
#define	__INITIAL_MXCSR__	0x1F80
#define	__INITIAL_MXCSR_MASK__	0xFFBF

#ifdef _KERNEL
void	fpudna(void);
void	fpudrop(void);
void	fpuexit(struct thread *td);
int	fpuformat(void);
int	fpugetregs(struct thread *td);
void	fpuinit(void);
void	fpusetregs(struct thread *td, struct savefpu *addr);
int	fputrap(void);
void	fpuuserinited(struct thread *td);
int	fpu_kern_enter(struct thread *td, struct fpu_kern_ctx *ctx,
	    u_int flags);
int	fpu_kern_leave(struct thread *td, struct fpu_kern_ctx *ctx);
int	fpu_kern_thread(u_int flags);
int	is_fpu_kern_thread(u_int flags);

/*
 * Flags for fpu_kern_enter() and fpu_kern_thread().
 */
#define	FPU_KERN_NORMAL	0x0000

#endif

#endif /* !_MACHINE_FPU_H_ */
