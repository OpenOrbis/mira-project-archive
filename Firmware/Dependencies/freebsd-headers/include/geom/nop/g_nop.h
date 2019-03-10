/*-
 * Copyright (c) 2004-2006 Pawel Jakub Dawidek <pjd@FreeBSD.org>
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: release/9.0.0/sys/geom/nop/g_nop.h 162834 2006-09-30 08:16:49Z pjd $
 */

#ifndef	_G_NOP_H_
#define	_G_NOP_H_

#define	G_NOP_CLASS_NAME	"NOP"
#define	G_NOP_VERSION		4
#define	G_NOP_SUFFIX		".nop"

#ifdef _KERNEL
#define	G_NOP_DEBUG(lvl, ...)	do {					\
	if (g_nop_debug >= (lvl)) {					\
		printf("GEOM_NOP");					\
		if (g_nop_debug > 0)					\
			printf("[%u]", lvl);				\
		printf(": ");						\
		printf(__VA_ARGS__);					\
		printf("\n");						\
	}								\
} while (0)
#define	G_NOP_LOGREQ(bp, ...)	do {					\
	if (g_nop_debug >= 2) {						\
		printf("GEOM_NOP[2]: ");				\
		printf(__VA_ARGS__);					\
		printf(" ");						\
		g_print_bio(bp);					\
		printf("\n");						\
	}								\
} while (0)

struct g_nop_softc {
	int		sc_error;
	off_t		sc_offset;
	u_int		sc_rfailprob;
	u_int		sc_wfailprob;
	uintmax_t	sc_reads;
	uintmax_t	sc_writes;
	uintmax_t	sc_readbytes;
	uintmax_t	sc_wrotebytes;
};
#endif	/* _KERNEL */

#endif	/* _G_NOP_H_ */
