/*-
 * Copyright (c) 1993 The Regents of the University of California.
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
 * $FreeBSD: release/9.0.0/sys/amd64/include/asmacros.h 209483 2010-06-23 20:44:07Z kib $
 */

#ifndef _MACHINE_ASMACROS_H_
#define _MACHINE_ASMACROS_H_

#include <sys/cdefs.h>

/* XXX too much duplication in various asm*.h's. */

/*
 * CNAME is used to manage the relationship between symbol names in C
 * and the equivalent assembly language names.  CNAME is given a name as
 * it would be used in a C program.  It expands to the equivalent assembly
 * language name.
 */
#define CNAME(csym)		csym

#define ALIGN_DATA	.p2align 3	/* 8 byte alignment, zero filled */
#ifdef GPROF
#define ALIGN_TEXT	.p2align 4,0x90	/* 16-byte alignment, nop filled */
#else
#define ALIGN_TEXT	.p2align 4,0x90	/* 16-byte alignment, nop filled */
#endif
#define SUPERALIGN_TEXT	.p2align 4,0x90	/* 16-byte alignment, nop filled */

#define GEN_ENTRY(name)		ALIGN_TEXT; .globl CNAME(name); \
				.type CNAME(name),@function; CNAME(name):
#define NON_GPROF_ENTRY(name)	GEN_ENTRY(name)
#define NON_GPROF_RET		.byte 0xc3	/* opcode for `ret' */

#define	END(name)		.size name, . - name

#ifdef GPROF
/*
 * __mcount is like [.]mcount except that doesn't require its caller to set
 * up a frame pointer.  It must be called before pushing anything onto the
 * stack.  gcc should eventually generate code to call __mcount in most
 * cases.  This would make -pg in combination with -fomit-frame-pointer
 * useful.  gcc has a configuration variable PROFILE_BEFORE_PROLOGUE to
 * allow profiling before setting up the frame pointer, but this is
 * inadequate for good handling of special cases, e.g., -fpic works best
 * with profiling after the prologue.
 *
 * [.]mexitcount is a new function to support non-statistical profiling if an
 * accurate clock is available.  For C sources, calls to it are generated
 * by the FreeBSD extension `-mprofiler-epilogue' to gcc.  It is best to
 * call [.]mexitcount at the end of a function like the MEXITCOUNT macro does,
 * but gcc currently generates calls to it at the start of the epilogue to
 * avoid problems with -fpic.
 *
 * [.]mcount and __mcount may clobber the call-used registers and %ef.
 * [.]mexitcount may clobber %ecx and %ef.
 *
 * Cross-jumping makes non-statistical profiling timing more complicated.
 * It is handled in many cases by calling [.]mexitcount before jumping.  It
 * is handled for conditional jumps using CROSSJUMP() and CROSSJUMP_LABEL().
 * It is handled for some fault-handling jumps by not sharing the exit
 * routine.
 *
 * ALTENTRY() must be before a corresponding ENTRY() so that it can jump to
 * the main entry point.  Note that alt entries are counted twice.  They
 * have to be counted as ordinary entries for gprof to get the call times
 * right for the ordinary entries.
 *
 * High local labels are used in macros to avoid clashes with local labels
 * in functions.
 *
 * Ordinary `ret' is used instead of a macro `RET' because there are a lot
 * of `ret's.  0xc3 is the opcode for `ret' (`#define ret ... ret' can't
 * be used because this file is sometimes preprocessed in traditional mode).
 * `ret' clobbers eflags but this doesn't matter.
 */
#define ALTENTRY(name)		GEN_ENTRY(name) ; MCOUNT ; MEXITCOUNT ; jmp 9f
#define	CROSSJUMP(jtrue, label, jfalse) \
	jfalse 8f; MEXITCOUNT; jmp __CONCAT(to,label); 8:
#define CROSSJUMPTARGET(label) \
	ALIGN_TEXT; __CONCAT(to,label): ; MCOUNT; jmp label
#define ENTRY(name)		GEN_ENTRY(name) ; 9: ; MCOUNT
#define FAKE_MCOUNT(caller)	pushq caller ; call __mcount ; popq %rcx
#define MCOUNT			call __mcount
#define MCOUNT_LABEL(name)	GEN_ENTRY(name) ; nop ; ALIGN_TEXT
#ifdef GUPROF
#define MEXITCOUNT		call .mexitcount
#define ret			MEXITCOUNT ; NON_GPROF_RET
#else
#define MEXITCOUNT
#endif

#else /* !GPROF */
/*
 * ALTENTRY() has to align because it is before a corresponding ENTRY().
 * ENTRY() has to align to because there may be no ALTENTRY() before it.
 * If there is a previous ALTENTRY() then the alignment code for ENTRY()
 * is empty.
 */
#define ALTENTRY(name)		GEN_ENTRY(name)
#define	CROSSJUMP(jtrue, label, jfalse)	jtrue label
#define	CROSSJUMPTARGET(label)
#define ENTRY(name)		GEN_ENTRY(name)
#define FAKE_MCOUNT(caller)
#define MCOUNT
#define MCOUNT_LABEL(name)
#define MEXITCOUNT
#endif /* GPROF */

#ifdef LOCORE
/*
 * Convenience macro for declaring interrupt entry points.
 */
#define	IDTVEC(name)	ALIGN_TEXT; .globl __CONCAT(X,name); \
			.type __CONCAT(X,name),@function; __CONCAT(X,name):

/*
 * Macros to create and destroy a trap frame.
 */
#define PUSH_FRAME							\
	subq	$TF_RIP,%rsp ;	/* skip dummy tf_err and tf_trapno */	\
	testb	$SEL_RPL_MASK,TF_CS(%rsp) ; /* come from kernel? */	\
	jz	1f ;		/* Yes, dont swapgs again */		\
	swapgs ;							\
1:	movq	%rdi,TF_RDI(%rsp) ;					\
	movq	%rsi,TF_RSI(%rsp) ;					\
	movq	%rdx,TF_RDX(%rsp) ;					\
	movq	%rcx,TF_RCX(%rsp) ;					\
	movq	%r8,TF_R8(%rsp) ;					\
	movq	%r9,TF_R9(%rsp) ;					\
	movq	%rax,TF_RAX(%rsp) ;					\
	movq	%rbx,TF_RBX(%rsp) ;					\
	movq	%rbp,TF_RBP(%rsp) ;					\
	movq	%r10,TF_R10(%rsp) ;					\
	movq	%r11,TF_R11(%rsp) ;					\
	movq	%r12,TF_R12(%rsp) ;					\
	movq	%r13,TF_R13(%rsp) ;					\
	movq	%r14,TF_R14(%rsp) ;					\
	movq	%r15,TF_R15(%rsp) ;					\
	movw	%fs,TF_FS(%rsp) ;					\
	movw	%gs,TF_GS(%rsp) ;					\
	movw	%es,TF_ES(%rsp) ;					\
	movw	%ds,TF_DS(%rsp) ;					\
	movl	$TF_HASSEGS,TF_FLAGS(%rsp) ;				\
	cld

#define POP_FRAME							\
	movq	TF_RDI(%rsp),%rdi ;					\
	movq	TF_RSI(%rsp),%rsi ;					\
	movq	TF_RDX(%rsp),%rdx ;					\
	movq	TF_RCX(%rsp),%rcx ;					\
	movq	TF_R8(%rsp),%r8 ;					\
	movq	TF_R9(%rsp),%r9 ;					\
	movq	TF_RAX(%rsp),%rax ;					\
	movq	TF_RBX(%rsp),%rbx ;					\
	movq	TF_RBP(%rsp),%rbp ;					\
	movq	TF_R10(%rsp),%r10 ;					\
	movq	TF_R11(%rsp),%r11 ;					\
	movq	TF_R12(%rsp),%r12 ;					\
	movq	TF_R13(%rsp),%r13 ;					\
	movq	TF_R14(%rsp),%r14 ;					\
	movq	TF_R15(%rsp),%r15 ;					\
	testb	$SEL_RPL_MASK,TF_CS(%rsp) ; /* come from kernel? */	\
	jz	1f ;		/* keep kernel GS.base */		\
	cli ;								\
	swapgs ;							\
1:	addq	$TF_RIP,%rsp	/* skip over tf_err, tf_trapno */

/*
 * Access per-CPU data.
 */
#define	PCPU(member)	%gs:PC_ ## member
#define	PCPU_ADDR(member, reg)					\
	movq %gs:PC_PRVSPACE, reg ;				\
	addq $PC_ ## member, reg

#endif /* LOCORE */

#endif /* !_MACHINE_ASMACROS_H_ */
