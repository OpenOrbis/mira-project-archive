/*-
 * Copyright (c) 1982, 1986, 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 * Copyright (c) 1995, Mike Mitchell
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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
 *	@(#)spx_timer.h
 *
 * $FreeBSD: release/9.0.0/sys/netipx/spx_timer.h 165899 2007-01-08 22:14:00Z rwatson $
 */

#ifndef _NETIPX_SPX_TIMER_H_
#define _NETIPX_SPX_TIMER_H_

/*
 * Definitions of the SPX timers.  These timers are counted
 * down PR_SLOWHZ times a second.
 */
#define	SPXT_REXMT	0		/* retransmit */
#define	SPXT_PERSIST	1		/* retransmit persistence */
#define	SPXT_KEEP	2		/* keep alive */
#define	SPXT_2MSL	3		/* 2*msl quiet time timer */

/*
 * The SPXT_REXMT timer is used to force retransmissions.
 * The SPX has the SPXT_REXMT timer set whenever segments
 * have been sent for which ACKs are expected but not yet
 * received.  If an ACK is received which advances tp->snd_una,
 * then the retransmit timer is cleared (if there are no more
 * outstanding segments) or reset to the base value (if there
 * are more ACKs expected).  Whenever the retransmit timer goes off,
 * we retransmit one unacknowledged segment, and do a backoff
 * on the retransmit timer.
 *
 * The SPXT_PERSIST timer is used to keep window size information
 * flowing even if the window goes shut.  If all previous transmissions
 * have been acknowledged (so that there are no retransmissions in progress),
 * and the window is too small to bother sending anything, then we start
 * the SPXT_PERSIST timer.  When it expires, if the window is nonzero,
 * we go to transmit state.  Otherwise, at intervals send a single byte
 * into the peer's window to force him to update our window information.
 * We do this at most as often as SPXT_PERSMIN time intervals,
 * but no more frequently than the current estimate of round-trip
 * packet time.  The SPXT_PERSIST timer is cleared whenever we receive
 * a window update from the peer.
 *
 * The SPXT_KEEP timer is used to keep connections alive.  If an
 * connection is idle (no segments received) for SPXTV_KEEP amount of time,
 * but not yet established, then we drop the connection.  If the connection
 * is established, then we force the peer to send us a segment by sending:
 *	<SEQ=SND.UNA-1><ACK=RCV.NXT><CTL=ACK>
 * This segment is (deliberately) outside the window, and should elicit
 * an ack segment in response from the peer.  If, despite the SPXT_KEEP
 * initiated segments we cannot elicit a response from a peer in SPXT_MAXIDLE
 * amount of time, then we drop the connection.
 */

#define	SPX_TTL		30		/* default time to live for SPX segs */
/*
 * Time constants.
 */
#define	SPXTV_MSL	( 15*PR_SLOWHZ)		/* max seg lifetime */
#define	SPXTV_SRTTBASE	0			/* base roundtrip time;
						   if 0, no idea yet */
#define	SPXTV_SRTTDFLT	(  3*PR_SLOWHZ)		/* assumed RTT if no info */

#define	SPXTV_PERSMIN	(  5*PR_SLOWHZ)		/* retransmit persistence */
#define	SPXTV_PERSMAX	( 60*PR_SLOWHZ)		/* maximum persist interval */

#define	SPXTV_KEEP	( 75*PR_SLOWHZ)		/* keep alive - 75 secs */
#define	SPXTV_MAXIDLE	(  8*SPXTV_KEEP)	/* maximum allowable idle
						   time before drop conn */

#define	SPXTV_MIN	(  1*PR_SLOWHZ)		/* minimum allowable value */
#define	SPXTV_REXMTMAX	( 64*PR_SLOWHZ)		/* max allowable REXMT value */

#define	SPX_LINGERTIME	120			/* linger at most 2 minutes */

#define	SPX_MAXRXTSHIFT	12			/* maximum retransmits */

#ifdef	SPXTIMERS
char *spxtimers[] =
    { "REXMT", "PERSIST", "KEEP", "2MSL" };
#endif

/*
 * Force a time value to be in a certain range.
 */
#define	SPXT_RANGESET(tv, value, tvmin, tvmax) { \
	(tv) = (value); \
	if ((tv) < (tvmin)) \
		(tv) = (tvmin); \
	else if ((tv) > (tvmax)) \
		(tv) = (tvmax); \
}

#endif /* !_NETIPX_SPX_TIMER_H_ */
