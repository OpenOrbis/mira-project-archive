/*-
 * Copyright (c) 2007-2009 Sam Leffler, Errno Consulting
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
 * $FreeBSD: release/9.0.0/sys/net80211/ieee80211_input.h 221418 2011-05-04 02:23:59Z adrian $
 */
#ifndef _NET80211_IEEE80211_INPUT_H_
#define _NET80211_IEEE80211_INPUT_H_

/* Verify the existence and length of __elem or get out. */
#define IEEE80211_VERIFY_ELEMENT(__elem, __maxlen, _action) do {	\
	if ((__elem) == NULL) {						\
		IEEE80211_DISCARD(vap, IEEE80211_MSG_ELEMID,		\
		    wh, NULL, "%s", "no " #__elem );			\
		vap->iv_stats.is_rx_elem_missing++;			\
		_action;						\
	} else if ((__elem)[1] > (__maxlen)) {				\
		IEEE80211_DISCARD(vap, IEEE80211_MSG_ELEMID,		\
		    wh, NULL, "bad " #__elem " len %d", (__elem)[1]);	\
		vap->iv_stats.is_rx_elem_toobig++;			\
		_action;						\
	}								\
} while (0)

#define	IEEE80211_VERIFY_LENGTH(_len, _minlen, _action) do {		\
	if ((_len) < (_minlen)) {					\
		IEEE80211_DISCARD(vap, IEEE80211_MSG_ELEMID,		\
		    wh, NULL, "ie too short, got %d, expected %d",	\
		    (_len), (_minlen));					\
		vap->iv_stats.is_rx_elem_toosmall++;			\
		_action;						\
	}								\
} while (0)

#ifdef IEEE80211_DEBUG
void	ieee80211_ssid_mismatch(struct ieee80211vap *, const char *tag,
	uint8_t mac[IEEE80211_ADDR_LEN], uint8_t *ssid);

#define	IEEE80211_VERIFY_SSID(_ni, _ssid, _action) do {			\
	if ((_ssid)[1] != 0 &&						\
	    ((_ssid)[1] != (_ni)->ni_esslen ||				\
	    memcmp((_ssid) + 2, (_ni)->ni_essid, (_ssid)[1]) != 0)) {	\
		if (ieee80211_msg_input(vap))				\
			ieee80211_ssid_mismatch(vap, 			\
			    ieee80211_mgt_subtype_name[subtype >>	\
				IEEE80211_FC0_SUBTYPE_SHIFT],		\
				wh->i_addr2, _ssid);			\
		vap->iv_stats.is_rx_ssidmismatch++;			\
		_action;						\
	}								\
} while (0)
#else /* !IEEE80211_DEBUG */
#define	IEEE80211_VERIFY_SSID(_ni, _ssid, _action) do {			\
	if ((_ssid)[1] != 0 &&						\
	    ((_ssid)[1] != (_ni)->ni_esslen ||				\
	    memcmp((_ssid) + 2, (_ni)->ni_essid, (_ssid)[1]) != 0)) {	\
		vap->iv_stats.is_rx_ssidmismatch++;			\
		_action;						\
	}								\
} while (0)
#endif /* !IEEE80211_DEBUG */

/* unalligned little endian access */     
#define LE_READ_2(p)					\
	((uint16_t)					\
	 ((((const uint8_t *)(p))[0]      ) |		\
	  (((const uint8_t *)(p))[1] <<  8)))
#define LE_READ_4(p)					\
	((uint32_t)					\
	 ((((const uint8_t *)(p))[0]      ) |		\
	  (((const uint8_t *)(p))[1] <<  8) |		\
	  (((const uint8_t *)(p))[2] << 16) |		\
	  (((const uint8_t *)(p))[3] << 24)))

static __inline int
iswpaoui(const uint8_t *frm)
{
	return frm[1] > 3 && LE_READ_4(frm+2) == ((WPA_OUI_TYPE<<24)|WPA_OUI);
}

static __inline int
iswmeoui(const uint8_t *frm)
{
	return frm[1] > 3 && LE_READ_4(frm+2) == ((WME_OUI_TYPE<<24)|WME_OUI);
}

static __inline int
iswmeparam(const uint8_t *frm)
{
	return frm[1] > 5 && LE_READ_4(frm+2) == ((WME_OUI_TYPE<<24)|WME_OUI) &&
		frm[6] == WME_PARAM_OUI_SUBTYPE;
}

static __inline int
iswmeinfo(const uint8_t *frm)
{
	return frm[1] > 5 && LE_READ_4(frm+2) == ((WME_OUI_TYPE<<24)|WME_OUI) &&
		frm[6] == WME_INFO_OUI_SUBTYPE;
}

static __inline int
isatherosoui(const uint8_t *frm)
{
	return frm[1] > 3 && LE_READ_4(frm+2) == ((ATH_OUI_TYPE<<24)|ATH_OUI);
}

static __inline int
istdmaoui(const uint8_t *frm)
{
	return frm[1] > 3 && LE_READ_4(frm+2) == ((TDMA_OUI_TYPE<<24)|TDMA_OUI);
}

static __inline int
ishtcapoui(const uint8_t *frm)
{
	return frm[1] > 3 && LE_READ_4(frm+2) == ((BCM_OUI_HTCAP<<24)|BCM_OUI);
}

static __inline int
ishtinfooui(const uint8_t *frm)
{
	return frm[1] > 3 && LE_READ_4(frm+2) == ((BCM_OUI_HTINFO<<24)|BCM_OUI);
}

#include <sys/endian.h>		/* For le16toh() */

/*
 * Check the current frame sequence number against the current TID
 * state and return whether it's in sequence or should be dropped.
 *
 * Since out of order packet and duplicate packet eliminations should
 * be done by the AMPDU RX code, this routine blindly accepts all
 * frames from a HT station w/ a TID that is currently doing AMPDU-RX.
 * HT stations without WME or where the TID is not doing AMPDU-RX
 * are checked like non-HT stations.
 *
 * The routine only eliminates packets whose sequence/fragment
 * match or are less than the last seen sequence/fragment number
 * AND are retransmits It doesn't try to eliminate out of order packets.
 *
 * Since all frames after sequence number 4095 will be less than 4095
 * (as the seqnum wraps), handle that special case so packets aren't
 * incorrectly dropped - ie, if the next packet is sequence number 0
 * but a retransmit since the initial packet didn't make it.
 */
static __inline int
ieee80211_check_rxseq(struct ieee80211_node *ni, struct ieee80211_frame *wh)
{
#define	SEQ_LEQ(a,b)	((int)((a)-(b)) <= 0)
#define	SEQ_EQ(a,b)	((int)((a)-(b)) == 0)
#define	HAS_SEQ(type)	((type & 0x4) == 0)
#define	SEQNO(a)	((a) >> IEEE80211_SEQ_SEQ_SHIFT)
#define	FRAGNO(a)	((a) & IEEE80211_SEQ_FRAG_MASK)
	uint16_t rxseq;
	uint8_t type;
	uint8_t tid;
	struct ieee80211_rx_ampdu *rap;

	rxseq = le16toh(*(uint16_t *)wh->i_seq);
	type = wh->i_fc[0] & IEEE80211_FC0_TYPE_MASK;

	/* Types with no sequence number are always treated valid */
	if (! HAS_SEQ(type))
		return 1;

	tid = ieee80211_gettid(wh);

	/*
	 * Only do the HT AMPDU check for WME stations; non-WME HT stations
	 * shouldn't exist outside of debugging. We should at least
	 * handle that.
	 */
	if (tid < WME_NUM_TID) {
		rap = &ni->ni_rx_ampdu[tid];
		/* HT nodes currently doing RX AMPDU are always valid */
		if ((ni->ni_flags & IEEE80211_NODE_HT) &&
		    (rap->rxa_flags & IEEE80211_AGGR_RUNNING))
			return 1;
	}

	/*	
	 * Otherwise, retries for packets below or equal to the last
	 * seen sequence number should be dropped.
	 */

	/*
	 * Treat frame seqnum 4095 as special due to boundary
	 * wrapping conditions.
	 */
	if (SEQNO(ni->ni_rxseqs[tid]) == 4095) {
		/*
		 * Drop retransmits on seqnum 4095/current fragment for itself.
		 */
		if (SEQ_EQ(rxseq, ni->ni_rxseqs[tid]) &&
		    (wh->i_fc[1] & IEEE80211_FC1_RETRY))
			return 0;
		/*
		 * Treat any subsequent frame as fine if the last seen frame
		 * is 4095 and it's not a retransmit for the same sequence
		 * number. However, this doesn't capture incorrectly ordered
	 	 * fragments w/ sequence number 4095. It shouldn't be seen
		 * in practice, but see the comment above for further info.
		 */
		return 1;
	}

	/*
	 * At this point we assume that retransmitted seq/frag numbers below
	 * the current can simply be eliminated.
	 */
	if ((wh->i_fc[1] & IEEE80211_FC1_RETRY) &&
	    SEQ_LEQ(rxseq, ni->ni_rxseqs[tid]))
		return 0;

	return 1;
#undef	SEQ_LEQ
#undef	SEQ_EQ
#undef	HAS_SEQ
#undef	SEQNO
#undef	FRAGNO
}

void	ieee80211_deliver_data(struct ieee80211vap *,
		struct ieee80211_node *, struct mbuf *);
struct mbuf *ieee80211_defrag(struct ieee80211_node *,
		struct mbuf *, int);
struct mbuf *ieee80211_realign(struct ieee80211vap *, struct mbuf *, size_t);
struct mbuf *ieee80211_decap(struct ieee80211vap *, struct mbuf *, int);
struct mbuf *ieee80211_decap1(struct mbuf *, int *);
int	ieee80211_setup_rates(struct ieee80211_node *ni,
		const uint8_t *rates, const uint8_t *xrates, int flags);
void ieee80211_send_error(struct ieee80211_node *,
		const uint8_t mac[IEEE80211_ADDR_LEN], int subtype, int arg);
int	ieee80211_alloc_challenge(struct ieee80211_node *);
int	ieee80211_parse_beacon(struct ieee80211_node *, struct mbuf *,
		struct ieee80211_scanparams *);
int	ieee80211_parse_action(struct ieee80211_node *, struct mbuf *);
#endif /* _NET80211_IEEE80211_INPUT_H_ */
