/*-
 * Copyright (c) 1990,1991 Regents of The University of Michigan.
 * All Rights Reserved.
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appears in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation, and that the name of The University
 * of Michigan not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. This software is supplied as is without expressed or
 * implied warranties of any kind.
 *
 *	Research Systems Unix Group
 *	The University of Michigan
 *	c/o Mike Clark
 *	535 W. William Street
 *	Ann Arbor, Michigan
 *	+1-313-763-0525
 *	netatalk@itd.umich.edu
 *
 * $FreeBSD: release/9.0.0/sys/netatalk/at.h 165972 2007-01-12 13:18:08Z rwatson $
 */

#ifndef _NETATALK_AT_H_
#define	_NETATALK_AT_H_

/*
 * Supported protocols
 */
#define	ATPROTO_DDP	0
#define	ATPROTO_AARP	254

#define	DDP_MAXSZ	587

/*
 * If ATPORT_FIRST <= Port < ATPORT_RESERVED, the port was created by a
 * privileged process.
 *
 * If ATPORT_RESERVED <= Port < ATPORT_LAST, the port was not necessarily
 * created by a privileged process.
 */
#define	ATPORT_FIRST	1
#define	ATPORT_RESERVED	128
#define	ATPORT_LAST	255

/*
 * AppleTalk address.
 */
struct at_addr {
	u_short	s_net;
	u_char	s_node;
};

#define	ATADDR_ANYNET	(u_short)0x0000
#define	ATADDR_ANYNODE	(u_char)0x00
#define	ATADDR_ANYPORT	(u_char)0x00
#define	ATADDR_BCAST	(u_char)0xff	/* There is no BCAST for NET. */

struct netrange {
	u_char	nr_phase;
	u_short	nr_firstnet;
	u_short	nr_lastnet;
};

/*
 * Socket address, AppleTalk style.  We keep magic information in the zero
 * bytes.  There are three types, NONE, CONFIG which has the phase and a net
 * range, and IFACE which has the network address of an interface.  IFACE may
 * be filled in by the client, and is filled in by the kernel.
 */
struct sockaddr_at {
	u_char		sat_len;
	u_char		sat_family;
	u_char		sat_port;
	struct at_addr	sat_addr;
	union {
		struct netrange	r_netrange;
		char		r_zero[8];	/* Hide struct netrange here. */
	} sat_range;
};

#define	sat_zero	sat_range.r_zero

#endif /* !_NETATALK_AT_H_ */
