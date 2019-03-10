/*-
 * Copyright (c) 1990, 1991 Regents of The University of Michigan.
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
 * $FreeBSD: release/9.0.0/sys/netatalk/ddp.h 171078 2007-06-28 12:54:58Z rwatson $
 */

#ifndef _NETATALK_DDP_H_
#define	_NETATALK_DDP_H_

/*-
 * <-1byte(8bits) ->
 * +---------------+
 * | 0 | hopc  |len|
 * +---------------+
 * | len (cont)    |
 * +---------------+
 * |               |
 * +- DDP csum    -+
 * |               |
 * +---------------+
 * |               |
 * +- Dest NET    -+
 * |               |
 * +---------------+
 * |               |
 * +- Src NET     -+
 * |               |
 * +---------------+
 * | Dest NODE     |
 * +---------------+
 * | Src NODE      |
 * +---------------+
 * | Dest PORT     |
 * +---------------+
 * | Src PORT      |
 * +---------------+
 *
 * On Apples, there is also a ddp_type field, after src_port.  However, under
 * this unix implementation, user level processes need to be able to set the
 * ddp_type.  In later revisions, the ddp_type may only be available in a
 * raw_appletalk interface.
 */

struct elaphdr {
	u_char	el_dnode;
	u_char	el_snode;
	u_char	el_type;
} __packed;

#define	SZ_ELAPHDR	3

#define	ELAP_DDPSHORT	0x01
#define	ELAP_DDPEXTEND	0x02

/*
 * Extended DDP header. Includes sickness for dealing with arbitrary
 * bitfields on a little-endian arch.
 */
struct ddpehdr {
	union {
		struct {
#if BYTE_ORDER == BIG_ENDIAN
			unsigned	dub_pad:2;
			unsigned	dub_hops:4;
			unsigned	dub_len:10;
			unsigned	dub_sum:16;
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
			unsigned	dub_sum:16;
			unsigned	dub_len:10;
			unsigned	dub_hops:4;
			unsigned	dub_pad:2;
#endif
		} __packed du_bits;
		unsigned	du_bytes;
	} deh_u;
	u_short	deh_dnet;
	u_short	deh_snet;
	u_char	deh_dnode;
	u_char	deh_snode;
	u_char	deh_dport;
	u_char	deh_sport;
} __packed;
#define	deh_pad		deh_u.du_bits.dub_pad
#define	deh_hops	deh_u.du_bits.dub_hops
#define	deh_len		deh_u.du_bits.dub_len
#define	deh_sum		deh_u.du_bits.dub_sum
#define	deh_bytes	deh_u.du_bytes

#define	DDP_MAXHOPS	15

struct ddpshdr {
	union {
		struct {
#if BYTE_ORDER == BIG_ENDIAN
			unsigned	dub_pad:6;
			unsigned	dub_len:10;
			unsigned	dub_dport:8;
			unsigned	dub_sport:8;
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
			unsigned	dub_sport:8;
			unsigned	dub_dport:8;
			unsigned	dub_len:10;
			unsigned	dub_pad:6;
#endif
		} __packed du_bits;
		unsigned	du_bytes;
	} dsh_u;
} __packed;

#define	dsh_pad		dsh_u.du_bits.dub_pad
#define	dsh_len		dsh_u.du_bits.dub_len
#define	dsh_dport	dsh_u.du_bits.dub_dport
#define	dsh_sport	dsh_u.du_bits.dub_sport
#define	dsh_bytes	dsh_u.du_bytes

#endif /* _NETATALK_DDP_H_ */
