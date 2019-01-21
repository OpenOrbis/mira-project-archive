/*-
 * Copyright (c) 2002 Poul-Henning Kamp
 * Copyright (c) 2002 Networks Associates Technology, Inc.
 * All rights reserved.
 *
 * This software was developed for the FreeBSD Project by Poul-Henning Kamp
 * and NAI Labs, the Security Research Division of Network Associates, Inc.
 * under DARPA/SPAWAR contract N66001-01-C-8035 ("CBOSS"), as part of the
 * DARPA CHATS research program.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the authors may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
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
 * $FreeBSD: release/9.0.0/sys/sys/kerneldump.h 214903 2010-11-07 03:09:02Z gonzo $
 */

#ifndef _SYS_KERNELDUMP_H
#define _SYS_KERNELDUMP_H

#include <machine/endian.h>

#if BYTE_ORDER == LITTLE_ENDIAN
#define	dtoh32(x)	__bswap32(x)
#define	dtoh64(x)	__bswap64(x)
#define	htod32(x)	__bswap32(x)
#define	htod64(x)	__bswap64(x)
#elif BYTE_ORDER == BIG_ENDIAN
#define	dtoh32(x)	(x)
#define	dtoh64(x)	(x)
#define	htod32(x)	(x)
#define	htod64(x)	(x)
#endif

/*
 * All uintX_t fields are in dump byte order, which is the same as
 * network byte order. Use the macros defined above to read or
 * write the fields.
 */
struct kerneldumpheader {
	char		magic[20];
#define	KERNELDUMPMAGIC		"FreeBSD Kernel Dump"
#define	TEXTDUMPMAGIC		"FreeBSD Text Dump"
#define	KERNELDUMPMAGIC_CLEARED	"Cleared Kernel Dump"
	char		architecture[12];
	uint32_t	version;
#define	KERNELDUMPVERSION	1
	uint32_t	architectureversion;
#define	KERNELDUMP_ALPHA_VERSION	1
#define	KERNELDUMP_AMD64_VERSION	2
#define	KERNELDUMP_ARM_VERSION		1
#define	KERNELDUMP_I386_VERSION		2
#define	KERNELDUMP_IA64_VERSION		1
#define	KERNELDUMP_MIPS_VERSION		1
#define	KERNELDUMP_POWERPC_VERSION	1
#define	KERNELDUMP_SPARC64_VERSION	1
#define	KERNELDUMP_TEXT_VERSION		1
	uint64_t	dumplength;		/* excl headers */
	uint64_t	dumptime;
	uint32_t	blocksize;
	char		hostname[64];
	char		versionstring[192];
	char		panicstring[192];
	uint32_t	parity;
};

/*
 * Parity calculation is endian insensitive.
 */
static __inline u_int32_t
kerneldump_parity(struct kerneldumpheader *kdhp)
{
	uint32_t *up, parity;
	u_int i;

	up = (uint32_t *)kdhp;
	parity = 0;
	for (i = 0; i < sizeof *kdhp; i += sizeof *up)
		parity ^= *up++;
	return (parity);
}

#ifdef _KERNEL
void mkdumpheader(struct kerneldumpheader *kdh, char *magic, uint32_t archver,
    uint64_t dumplen, uint32_t blksz);
#endif

#endif /* _SYS_KERNELDUMP_H */
