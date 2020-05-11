/*	$FreeBSD: release/9.0.0/sys/netipsec/keysock.h 195727 2009-07-16 21:13:04Z rwatson $	*/
/*	$KAME: keysock.h,v 1.8 2000/03/27 05:11:06 sumikawa Exp $	*/

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

#ifndef _NETIPSEC_KEYSOCK_H_
#define _NETIPSEC_KEYSOCK_H_

/* statistics for pfkey socket */
struct pfkeystat {
	/* kernel -> userland */
	u_quad_t out_total;		/* # of total calls */
	u_quad_t out_bytes;		/* total bytecount */
	u_quad_t out_msgtype[256];	/* message type histogram */
	u_quad_t out_invlen;		/* invalid length field */
	u_quad_t out_invver;		/* invalid version field */
	u_quad_t out_invmsgtype;	/* invalid message type field */
	u_quad_t out_tooshort;		/* msg too short */
	u_quad_t out_nomem;		/* memory allocation failure */
	u_quad_t out_dupext;		/* duplicate extension */
	u_quad_t out_invexttype;	/* invalid extension type */
	u_quad_t out_invsatype;		/* invalid sa type */
	u_quad_t out_invaddr;		/* invalid address extension */
	/* userland -> kernel */
	u_quad_t in_total;		/* # of total calls */
	u_quad_t in_bytes;		/* total bytecount */
	u_quad_t in_msgtype[256];	/* message type histogram */
	u_quad_t in_msgtarget[3];	/* one/all/registered */
	u_quad_t in_nomem;		/* memory allocation failure */
	/* others */
	u_quad_t sockerr;		/* # of socket related errors */
};

#define KEY_SENDUP_ONE		0
#define KEY_SENDUP_ALL		1
#define KEY_SENDUP_REGISTERED	2

#ifdef _KERNEL
struct keycb {
	struct rawcb kp_raw;	/* rawcb */
	int kp_promisc;		/* promiscuous mode */
	int kp_registered;	/* registered socket */
};

VNET_DECLARE(struct pfkeystat, pfkeystat);
#define	V_pfkeystat		VNET(pfkeystat)

extern int key_output(struct mbuf *m, struct socket *so);
extern int key_usrreq __P((struct socket *,
	int, struct mbuf *, struct mbuf *, struct mbuf *));

extern int key_sendup __P((struct socket *, struct sadb_msg *, u_int, int));
extern int key_sendup_mbuf __P((struct socket *, struct mbuf *, int));
#endif /* _KERNEL */

#endif /*_NETIPSEC_KEYSOCK_H_*/
