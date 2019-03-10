/*	$FreeBSD: release/9.0.0/sys/net/if_gif.h 207369 2010-04-29 11:52:42Z bz $	*/
/*	$KAME: if_gif.h,v 1.17 2000/09/11 11:36:41 sumikawa Exp $	*/

/*-
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
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
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * if_gif.h
 */

#ifndef _NET_IF_GIF_H_
#define _NET_IF_GIF_H_


#ifdef _KERNEL
#include "opt_inet.h"
#include "opt_inet6.h"

#include <netinet/in.h>
/* xxx sigh, why route have struct route instead of pointer? */

struct encaptab;

extern	void (*ng_gif_input_p)(struct ifnet *ifp, struct mbuf **mp,
		int af);
extern	void (*ng_gif_input_orphan_p)(struct ifnet *ifp, struct mbuf *m,
		int af);
extern	int  (*ng_gif_output_p)(struct ifnet *ifp, struct mbuf **mp);
extern	void (*ng_gif_attach_p)(struct ifnet *ifp);
extern	void (*ng_gif_detach_p)(struct ifnet *ifp);

struct gif_softc {
	struct ifnet	*gif_ifp;
	struct mtx	gif_mtx;
	struct sockaddr	*gif_psrc; /* Physical src addr */
	struct sockaddr	*gif_pdst; /* Physical dst addr */
	union {
		struct route  gifscr_ro;    /* xxx */
#ifdef INET6
		struct route_in6 gifscr_ro6; /* xxx */
#endif
	} gifsc_gifscr;
	int		gif_flags;
	u_int		gif_fibnum;
	const struct encaptab *encap_cookie4;
	const struct encaptab *encap_cookie6;
	void		*gif_netgraph;	/* ng_gif(4) netgraph node info */
	u_int		gif_options;
	LIST_ENTRY(gif_softc) gif_list; /* all gif's are linked */
};
#define	GIF2IFP(sc)	((sc)->gif_ifp)
#define	GIF_LOCK_INIT(sc)	mtx_init(&(sc)->gif_mtx, "gif softc",	\
				     NULL, MTX_DEF)
#define	GIF_LOCK_DESTROY(sc)	mtx_destroy(&(sc)->gif_mtx)
#define	GIF_LOCK(sc)		mtx_lock(&(sc)->gif_mtx)
#define	GIF_UNLOCK(sc)		mtx_unlock(&(sc)->gif_mtx)
#define	GIF_LOCK_ASSERT(sc)	mtx_assert(&(sc)->gif_mtx, MA_OWNED)

#define gif_ro gifsc_gifscr.gifscr_ro
#ifdef INET6
#define gif_ro6 gifsc_gifscr.gifscr_ro6
#endif

#define GIF_MTU		(1280)	/* Default MTU */
#define	GIF_MTU_MIN	(1280)	/* Minimum MTU */
#define	GIF_MTU_MAX	(8192)	/* Maximum MTU */

#define	MTAG_GIF	1080679712
#define	MTAG_GIF_CALLED	0

struct etherip_header {
#if BYTE_ORDER == LITTLE_ENDIAN
	u_int	eip_resvl:4,	/* reserved */
		eip_ver:4;	/* version */
#endif
#if BYTE_ORDER == BIG_ENDIAN
	u_int	eip_ver:4,	/* version */
		eip_resvl:4;	/* reserved */
#endif
	u_int8_t eip_resvh;	/* reserved */
} __packed;

#define ETHERIP_VERSION			0x3
/* mbuf adjust factor to force 32-bit alignment of IP header */
#define	ETHERIP_ALIGN		2

/* Prototypes */
void gif_input(struct mbuf *, int, struct ifnet *);
int gif_output(struct ifnet *, struct mbuf *, struct sockaddr *,
	       struct route *);
int gif_ioctl(struct ifnet *, u_long, caddr_t);
int gif_set_tunnel(struct ifnet *, struct sockaddr *, struct sockaddr *);
void gif_delete_tunnel(struct ifnet *);
int gif_encapcheck(const struct mbuf *, int, int, void *);
#endif /* _KERNEL */

#define GIFGOPTS	_IOWR('i', 150, struct ifreq)
#define GIFSOPTS	_IOW('i', 151, struct ifreq)

#define	GIF_ACCEPT_REVETHIP	0x0001
#define	GIF_SEND_REVETHIP	0x0010
#define	GIF_OPTMASK		(GIF_ACCEPT_REVETHIP|GIF_SEND_REVETHIP)

#endif /* _NET_IF_GIF_H_ */
