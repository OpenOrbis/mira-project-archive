/*-
 * Copyright (c) 2001 Atsushi Onoe
 * Copyright (c) 2002-2008 Sam Leffler, Errno Consulting
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
 * $FreeBSD: release/9.0.0/sys/net80211/_ieee80211.h 220935 2011-04-22 00:44:27Z adrian $
 */
#ifndef _NET80211__IEEE80211_H_
#define _NET80211__IEEE80211_H_

/*
 * 802.11 implementation definitions.
 *
 * NB: this file is used by applications.
 */

/*
 * PHY type; mostly used to identify FH phys.
 */
enum ieee80211_phytype {
	IEEE80211_T_DS,			/* direct sequence spread spectrum */
	IEEE80211_T_FH,			/* frequency hopping */
	IEEE80211_T_OFDM,		/* frequency division multiplexing */
	IEEE80211_T_TURBO,		/* high rate OFDM, aka turbo mode */
	IEEE80211_T_HT,			/* high throughput */
	IEEE80211_T_OFDM_HALF,		/* 1/2 rate OFDM */
	IEEE80211_T_OFDM_QUARTER,	/* 1/4 rate OFDM */
};
#define	IEEE80211_T_CCK	IEEE80211_T_DS	/* more common nomenclature */

/*
 * PHY mode; this is not really a mode as multi-mode devices
 * have multiple PHY's.  Mode is mostly used as a shorthand
 * for constraining which channels to consider in setting up
 * operation.  Modes used to be used more extensively when
 * channels were identified as IEEE channel numbers.
 */
enum ieee80211_phymode {
	IEEE80211_MODE_AUTO	= 0,	/* autoselect */
	IEEE80211_MODE_11A	= 1,	/* 5GHz, OFDM */
	IEEE80211_MODE_11B	= 2,	/* 2GHz, CCK */
	IEEE80211_MODE_11G	= 3,	/* 2GHz, OFDM */
	IEEE80211_MODE_FH	= 4,	/* 2GHz, GFSK */
	IEEE80211_MODE_TURBO_A	= 5,	/* 5GHz, OFDM, 2x clock */
	IEEE80211_MODE_TURBO_G	= 6,	/* 2GHz, OFDM, 2x clock */
	IEEE80211_MODE_STURBO_A	= 7,	/* 5GHz, OFDM, 2x clock, static */
	IEEE80211_MODE_11NA	= 8,	/* 5GHz, w/ HT */
	IEEE80211_MODE_11NG	= 9,	/* 2GHz, w/ HT */
	IEEE80211_MODE_HALF	= 10,	/* OFDM, 1/2x clock */
	IEEE80211_MODE_QUARTER	= 11,	/* OFDM, 1/4x clock */
};
#define	IEEE80211_MODE_MAX	(IEEE80211_MODE_QUARTER+1)

/*
 * Operating mode.  Devices do not necessarily support
 * all modes; they indicate which are supported in their
 * capabilities.
 */
enum ieee80211_opmode {
	IEEE80211_M_IBSS 	= 0,	/* IBSS (adhoc) station */
	IEEE80211_M_STA		= 1,	/* infrastructure station */
	IEEE80211_M_WDS		= 2,	/* WDS link */
	IEEE80211_M_AHDEMO	= 3,	/* Old lucent compatible adhoc demo */
	IEEE80211_M_HOSTAP	= 4,	/* Software Access Point */
	IEEE80211_M_MONITOR	= 5,	/* Monitor mode */
	IEEE80211_M_MBSS	= 6,	/* MBSS (Mesh Point) link */
};
#define	IEEE80211_OPMODE_MAX	(IEEE80211_M_MBSS+1)

/*
 * 802.11g/802.11n protection mode.
 */
enum ieee80211_protmode {
	IEEE80211_PROT_NONE	= 0,	/* no protection */
	IEEE80211_PROT_CTSONLY	= 1,	/* CTS to self */
	IEEE80211_PROT_RTSCTS	= 2,	/* RTS-CTS */
};

/*
 * Authentication mode.  The open and shared key authentication
 * modes are implemented within the 802.11 layer.  802.1x and
 * WPA/802.11i are implemented in user mode by setting the
 * 802.11 layer into IEEE80211_AUTH_8021X and deferring
 * authentication to user space programs.
 */
enum ieee80211_authmode {
	IEEE80211_AUTH_NONE	= 0,
	IEEE80211_AUTH_OPEN	= 1,		/* open */
	IEEE80211_AUTH_SHARED	= 2,		/* shared-key */
	IEEE80211_AUTH_8021X	= 3,		/* 802.1x */
	IEEE80211_AUTH_AUTO	= 4,		/* auto-select/accept */
	/* NB: these are used only for ioctls */
	IEEE80211_AUTH_WPA	= 5,		/* WPA/RSN w/ 802.1x/PSK */
};

/*
 * Roaming mode is effectively who controls the operation
 * of the 802.11 state machine when operating as a station.
 * State transitions are controlled either by the driver
 * (typically when management frames are processed by the
 * hardware/firmware), the host (auto/normal operation of
 * the 802.11 layer), or explicitly through ioctl requests
 * when applications like wpa_supplicant want control.
 */
enum ieee80211_roamingmode {
	IEEE80211_ROAMING_DEVICE= 0,	/* driver/hardware control */
	IEEE80211_ROAMING_AUTO	= 1,	/* 802.11 layer control */
	IEEE80211_ROAMING_MANUAL= 2,	/* application control */
};

/*
 * Channels are specified by frequency and attributes.
 */
struct ieee80211_channel {
	uint32_t	ic_flags;	/* see below */
	uint16_t	ic_freq;	/* setting in MHz */
	uint8_t		ic_ieee;	/* IEEE channel number */
	int8_t		ic_maxregpower;	/* maximum regulatory tx power in dBm */
	int8_t		ic_maxpower;	/* maximum tx power in .5 dBm */
	int8_t		ic_minpower;	/* minimum tx power in .5 dBm */
	uint8_t		ic_state;	/* dynamic state */
	uint8_t		ic_extieee;	/* HT40 extension channel number */
	int8_t		ic_maxantgain;	/* maximum antenna gain in .5 dBm */
	uint8_t		ic_pad;
	uint16_t	ic_devdata;	/* opaque device/driver data */
};

#define	IEEE80211_CHAN_MAX	256
#define	IEEE80211_CHAN_BYTES	32	/* howmany(IEEE80211_CHAN_MAX, NBBY) */
#define	IEEE80211_CHAN_ANY	0xffff	/* token for ``any channel'' */
#define	IEEE80211_CHAN_ANYC \
	((struct ieee80211_channel *) IEEE80211_CHAN_ANY)

/* channel attributes */
#define	IEEE80211_CHAN_PRIV0	0x00000001 /* driver private bit 0 */
#define	IEEE80211_CHAN_PRIV1	0x00000002 /* driver private bit 1 */
#define	IEEE80211_CHAN_PRIV2	0x00000004 /* driver private bit 2 */
#define	IEEE80211_CHAN_PRIV3	0x00000008 /* driver private bit 3 */
#define	IEEE80211_CHAN_TURBO	0x00000010 /* Turbo channel */
#define	IEEE80211_CHAN_CCK	0x00000020 /* CCK channel */
#define	IEEE80211_CHAN_OFDM	0x00000040 /* OFDM channel */
#define	IEEE80211_CHAN_2GHZ	0x00000080 /* 2 GHz spectrum channel. */
#define	IEEE80211_CHAN_5GHZ	0x00000100 /* 5 GHz spectrum channel */
#define	IEEE80211_CHAN_PASSIVE	0x00000200 /* Only passive scan allowed */
#define	IEEE80211_CHAN_DYN	0x00000400 /* Dynamic CCK-OFDM channel */
#define	IEEE80211_CHAN_GFSK	0x00000800 /* GFSK channel (FHSS PHY) */
#define	IEEE80211_CHAN_GSM	0x00001000 /* 900 MHz spectrum channel */
#define	IEEE80211_CHAN_STURBO	0x00002000 /* 11a static turbo channel only */
#define	IEEE80211_CHAN_HALF	0x00004000 /* Half rate channel */
#define	IEEE80211_CHAN_QUARTER	0x00008000 /* Quarter rate channel */
#define	IEEE80211_CHAN_HT20	0x00010000 /* HT 20 channel */
#define	IEEE80211_CHAN_HT40U	0x00020000 /* HT 40 channel w/ ext above */
#define	IEEE80211_CHAN_HT40D	0x00040000 /* HT 40 channel w/ ext below */
#define	IEEE80211_CHAN_DFS	0x00080000 /* DFS required */
#define	IEEE80211_CHAN_4MSXMIT	0x00100000 /* 4ms limit on frame length */
#define	IEEE80211_CHAN_NOADHOC	0x00200000 /* adhoc mode not allowed */
#define	IEEE80211_CHAN_NOHOSTAP	0x00400000 /* hostap mode not allowed */
#define	IEEE80211_CHAN_11D	0x00800000 /* 802.11d required */

#define	IEEE80211_CHAN_HT40	(IEEE80211_CHAN_HT40U | IEEE80211_CHAN_HT40D)
#define	IEEE80211_CHAN_HT	(IEEE80211_CHAN_HT20 | IEEE80211_CHAN_HT40)

#define	IEEE80211_CHAN_BITS \
	"\20\1PRIV0\2PRIV2\3PRIV3\4PRIV4\5TURBO\6CCK\7OFDM\0102GHZ\0115GHZ" \
	"\12PASSIVE\13DYN\14GFSK\15GSM\16STURBO\17HALF\20QUARTER\21HT20" \
	"\22HT40U\23HT40D\24DFS\0254MSXMIT\26NOADHOC\27NOHOSTAP\03011D"

/*
 * Useful combinations of channel characteristics.
 */
#define	IEEE80211_CHAN_FHSS \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_GFSK)
#define	IEEE80211_CHAN_A \
	(IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_OFDM)
#define	IEEE80211_CHAN_B \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_CCK)
#define	IEEE80211_CHAN_PUREG \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_OFDM)
#define	IEEE80211_CHAN_G \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_DYN)
#define IEEE80211_CHAN_108A \
	(IEEE80211_CHAN_A | IEEE80211_CHAN_TURBO)
#define	IEEE80211_CHAN_108G \
	(IEEE80211_CHAN_PUREG | IEEE80211_CHAN_TURBO)
#define	IEEE80211_CHAN_ST \
	(IEEE80211_CHAN_108A | IEEE80211_CHAN_STURBO)

#define	IEEE80211_CHAN_ALL \
	(IEEE80211_CHAN_2GHZ | IEEE80211_CHAN_5GHZ | IEEE80211_CHAN_GFSK | \
	 IEEE80211_CHAN_CCK | IEEE80211_CHAN_OFDM | IEEE80211_CHAN_DYN | \
	 IEEE80211_CHAN_HALF | IEEE80211_CHAN_QUARTER | \
	 IEEE80211_CHAN_HT)
#define	IEEE80211_CHAN_ALLTURBO \
	(IEEE80211_CHAN_ALL | IEEE80211_CHAN_TURBO | IEEE80211_CHAN_STURBO)

#define	IEEE80211_IS_CHAN_FHSS(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_FHSS) == IEEE80211_CHAN_FHSS)
#define	IEEE80211_IS_CHAN_A(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_A) == IEEE80211_CHAN_A)
#define	IEEE80211_IS_CHAN_B(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_B) == IEEE80211_CHAN_B)
#define	IEEE80211_IS_CHAN_PUREG(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_PUREG) == IEEE80211_CHAN_PUREG)
#define	IEEE80211_IS_CHAN_G(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_G) == IEEE80211_CHAN_G)
#define	IEEE80211_IS_CHAN_ANYG(_c) \
	(IEEE80211_IS_CHAN_PUREG(_c) || IEEE80211_IS_CHAN_G(_c))
#define	IEEE80211_IS_CHAN_ST(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_ST) == IEEE80211_CHAN_ST)
#define	IEEE80211_IS_CHAN_108A(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_108A) == IEEE80211_CHAN_108A)
#define	IEEE80211_IS_CHAN_108G(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_108G) == IEEE80211_CHAN_108G)

#define	IEEE80211_IS_CHAN_2GHZ(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_2GHZ) != 0)
#define	IEEE80211_IS_CHAN_5GHZ(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_5GHZ) != 0)
#define	IEEE80211_IS_CHAN_PASSIVE(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_PASSIVE) != 0)
#define	IEEE80211_IS_CHAN_OFDM(_c) \
	(((_c)->ic_flags & (IEEE80211_CHAN_OFDM | IEEE80211_CHAN_DYN)) != 0)
#define	IEEE80211_IS_CHAN_CCK(_c) \
	(((_c)->ic_flags & (IEEE80211_CHAN_CCK | IEEE80211_CHAN_DYN)) != 0)
#define	IEEE80211_IS_CHAN_GFSK(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_GFSK) != 0)
#define	IEEE80211_IS_CHAN_TURBO(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_TURBO) != 0)
#define	IEEE80211_IS_CHAN_STURBO(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_STURBO) != 0)
#define	IEEE80211_IS_CHAN_DTURBO(_c) \
	(((_c)->ic_flags & \
	(IEEE80211_CHAN_TURBO | IEEE80211_CHAN_STURBO)) == IEEE80211_CHAN_TURBO)
#define	IEEE80211_IS_CHAN_HALF(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_HALF) != 0)
#define	IEEE80211_IS_CHAN_QUARTER(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_QUARTER) != 0)
#define	IEEE80211_IS_CHAN_FULL(_c) \
	(((_c)->ic_flags & (IEEE80211_CHAN_QUARTER | IEEE80211_CHAN_HALF)) == 0)
#define	IEEE80211_IS_CHAN_GSM(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_GSM) != 0)
#define	IEEE80211_IS_CHAN_HT(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_HT) != 0)
#define	IEEE80211_IS_CHAN_HT20(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_HT20) != 0)
#define	IEEE80211_IS_CHAN_HT40(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_HT40) != 0)
#define	IEEE80211_IS_CHAN_HT40U(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_HT40U) != 0)
#define	IEEE80211_IS_CHAN_HT40D(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_HT40D) != 0)
#define	IEEE80211_IS_CHAN_HTA(_c) \
	(IEEE80211_IS_CHAN_5GHZ(_c) && \
	 ((_c)->ic_flags & IEEE80211_CHAN_HT) != 0)
#define	IEEE80211_IS_CHAN_HTG(_c) \
	(IEEE80211_IS_CHAN_2GHZ(_c) && \
	 ((_c)->ic_flags & IEEE80211_CHAN_HT) != 0)
#define	IEEE80211_IS_CHAN_DFS(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_DFS) != 0)
#define	IEEE80211_IS_CHAN_NOADHOC(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_NOADHOC) != 0)
#define	IEEE80211_IS_CHAN_NOHOSTAP(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_NOHOSTAP) != 0)
#define	IEEE80211_IS_CHAN_11D(_c) \
	(((_c)->ic_flags & IEEE80211_CHAN_11D) != 0)

#define	IEEE80211_CHAN2IEEE(_c)		(_c)->ic_ieee

/* dynamic state */
#define	IEEE80211_CHANSTATE_RADAR	0x01	/* radar detected */
#define	IEEE80211_CHANSTATE_CACDONE	0x02	/* CAC completed */
#define	IEEE80211_CHANSTATE_CWINT	0x04	/* interference detected */
#define	IEEE80211_CHANSTATE_NORADAR	0x10	/* post notify on radar clear */

#define	IEEE80211_IS_CHAN_RADAR(_c) \
	(((_c)->ic_state & IEEE80211_CHANSTATE_RADAR) != 0)
#define	IEEE80211_IS_CHAN_CACDONE(_c) \
	(((_c)->ic_state & IEEE80211_CHANSTATE_CACDONE) != 0)
#define	IEEE80211_IS_CHAN_CWINT(_c) \
	(((_c)->ic_state & IEEE80211_CHANSTATE_CWINT) != 0)

/* ni_chan encoding for FH phy */
#define	IEEE80211_FH_CHANMOD	80
#define	IEEE80211_FH_CHAN(set,pat)	(((set)-1)*IEEE80211_FH_CHANMOD+(pat))
#define	IEEE80211_FH_CHANSET(chan)	((chan)/IEEE80211_FH_CHANMOD+1)
#define	IEEE80211_FH_CHANPAT(chan)	((chan)%IEEE80211_FH_CHANMOD)

#define	IEEE80211_TID_SIZE	(WME_NUM_TID+1)	/* WME TID's +1 for non-QoS */
#define	IEEE80211_NONQOS_TID	WME_NUM_TID	/* index for non-QoS sta */

/*
 * The 802.11 spec says at most 2007 stations may be
 * associated at once.  For most AP's this is way more
 * than is feasible so we use a default of 128.  This
 * number may be overridden by the driver and/or by
 * user configuration but may not be less than IEEE80211_AID_MIN.
 */
#define	IEEE80211_AID_DEF		128
#define	IEEE80211_AID_MIN		16

/*
 * 802.11 rate set.
 */
#define	IEEE80211_RATE_SIZE	8		/* 802.11 standard */
#define	IEEE80211_RATE_MAXSIZE	15		/* max rates we'll handle */

struct ieee80211_rateset {
	uint8_t		rs_nrates;
	uint8_t		rs_rates[IEEE80211_RATE_MAXSIZE];
};

/*
 * 802.11n variant of ieee80211_rateset.  Instead of
 * legacy rates the entries are MCS rates.  We define
 * the structure such that it can be used interchangeably
 * with an ieee80211_rateset (modulo structure size).
 */
#define	IEEE80211_HTRATE_MAXSIZE	77

struct ieee80211_htrateset {
	uint8_t		rs_nrates;
	uint8_t		rs_rates[IEEE80211_HTRATE_MAXSIZE];
};

#define	IEEE80211_RATE_MCS	0x80

/*
 * Per-mode transmit parameters/controls visible to user space.
 * These can be used to set fixed transmit rate for all operating
 * modes or on a per-client basis according to the capabilities
 * of the client (e.g. an 11b client associated to an 11g ap).
 *
 * MCS are distinguished from legacy rates by or'ing in 0x80.
 */
struct ieee80211_txparam {
	uint8_t		ucastrate;	/* ucast data rate (legacy/MCS|0x80) */
	uint8_t		mgmtrate;	/* mgmt frame rate (legacy/MCS|0x80) */
	uint8_t		mcastrate;	/* multicast rate (legacy/MCS|0x80) */
	uint8_t		maxretry;	/* max unicast data retry count */
};

/*
 * Per-mode roaming state visible to user space.  There are two
 * thresholds that control whether roaming is considered; when
 * either is exceeded the 802.11 layer will check the scan cache
 * for another AP.  If the cache is stale then a scan may be
 * triggered.
 */
struct ieee80211_roamparam {
	int8_t		rssi;		/* rssi thresh (.5 dBm) */
	uint8_t		rate;		/* tx rate thresh (.5 Mb/s or MCS) */
	uint16_t	pad;		/* reserve */
};

/*
 * Regulatory Information.
 */
struct ieee80211_regdomain {
	uint16_t	regdomain;	/* SKU */
	uint16_t	country;	/* ISO country code */
	uint8_t		location;	/* I (indoor), O (outdoor), other */
	uint8_t		ecm;		/* Extended Channel Mode */
	char		isocc[2];	/* country code string */
	short		pad[2];
};

/*
 * MIMO antenna/radio state.
 */

#define	IEEE80211_MAX_CHAINS		3
#define	IEEE80211_MAX_EVM_PILOTS	6

/*
 * XXX This doesn't yet export both ctl/ext chain details
 */
struct ieee80211_mimo_info {
	int8_t		rssi[IEEE80211_MAX_CHAINS];	/* per-antenna rssi */
	int8_t		noise[IEEE80211_MAX_CHAINS];	/* per-antenna noise floor */
	uint8_t		pad[2];
	uint32_t	evm[3];		/* EVM data */
};
#endif /* _NET80211__IEEE80211_H_ */
