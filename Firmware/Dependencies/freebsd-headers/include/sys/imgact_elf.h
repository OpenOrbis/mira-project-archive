/*-
 * Copyright (c) 1995-1996 S�ren Schmidt
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
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
 * $FreeBSD: release/9.0.0/sys/sys/imgact_elf.h 210446 2010-07-24 14:23:06Z kib $
 */

#ifndef _SYS_IMGACT_ELF_H_
#define	_SYS_IMGACT_ELF_H_

#include <machine/elf.h>

#ifdef _KERNEL

#define	AUXARGS_ENTRY(pos, id, val) {suword(pos++, id); suword(pos++, val);}

struct thread;

/*
 * Structure used to pass infomation from the loader to the
 * stack fixup routine.
 */
typedef struct {
	Elf_Ssize	execfd;
	Elf_Size	phdr;
	Elf_Size	phent;
	Elf_Size	phnum;
	Elf_Size	pagesz;
	Elf_Size	base;
	Elf_Size	flags;
	Elf_Size	entry;
} __ElfN(Auxargs);

typedef struct {
	Elf_Note	hdr;
	const char *	vendor;
	int		flags;
	boolean_t	(*trans_osrel)(const Elf_Note *, int32_t *);
#define	BN_CAN_FETCH_OSREL	0x0001	/* Deprecated. */
#define	BN_TRANSLATE_OSREL	0x0002	/* Use trans_osrel to fetch osrel */
		/* after checking the image ABI specification, if needed. */
} Elf_Brandnote;

typedef struct {
	int brand;
	int machine;
	const char *compat_3_brand;	/* pre Binutils 2.10 method (FBSD 3) */
	const char *emul_path;
	const char *interp_path;
	struct sysentvec *sysvec;
	const char *interp_newpath;
	int flags;
	Elf_Brandnote *brand_note;
#define	BI_CAN_EXEC_DYN		0x0001
#define	BI_BRAND_NOTE		0x0002	/* May have note.ABI-tag section. */
#define	BI_BRAND_NOTE_MANDATORY	0x0004	/* Must have note.ABI-tag section. */
} __ElfN(Brandinfo);

__ElfType(Auxargs);
__ElfType(Brandinfo);

#define	MAX_BRANDS	8

int	__elfN(brand_inuse)(Elf_Brandinfo *entry);
int	__elfN(insert_brand_entry)(Elf_Brandinfo *entry);
int	__elfN(remove_brand_entry)(Elf_Brandinfo *entry);
int	__elfN(freebsd_fixup)(register_t **, struct image_params *);
int	__elfN(coredump)(struct thread *, struct vnode *, off_t, int);

/* Machine specific function to dump per-thread information. */
void	__elfN(dump_thread)(struct thread *, void *, size_t *);

extern int __elfN(fallback_brand);
extern Elf_Brandnote __elfN(freebsd_brandnote);
extern Elf_Brandnote __elfN(kfreebsd_brandnote);
#endif /* _KERNEL */

#endif /* !_SYS_IMGACT_ELF_H_ */
