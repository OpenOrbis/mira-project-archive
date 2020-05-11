/*-
 * Copyright (c) 2001-2007, by Cisco Systems, Inc. All rights reserved.
 * Copyright (c) 2008-2011, by Randall Stewart. All rights reserved.
 * Copyright (c) 2008-2011, by Michael Tuexen. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * a) Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * b) Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the distribution.
 *
 * c) Neither the name of Cisco Systems, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/* $KAME: sctp_output.h,v 1.14 2005/03/06 16:04:18 itojun Exp $	 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: release/9.0.0/sys/netinet/sctp_output.h 224641 2011-08-03 20:21:00Z tuexen $");

#ifndef __sctp_output_h__
#define __sctp_output_h__

#include <netinet/sctp_header.h>

#if defined(_KERNEL) || defined(__Userspace__)


struct mbuf *
sctp_add_addresses_to_i_ia(struct sctp_inpcb *inp,
    struct sctp_tcb *stcb,
    struct sctp_scoping *scope,
    struct mbuf *m_at,
    int cnt_inits_to);


int sctp_is_addr_restricted(struct sctp_tcb *, struct sctp_ifa *);


int
sctp_is_address_in_scope(struct sctp_ifa *ifa,
    int ipv4_addr_legal,
    int ipv6_addr_legal,
    int loopback_scope,
    int ipv4_local_scope,
    int local_scope,
    int site_scope,
    int do_update);
int
    sctp_is_addr_in_ep(struct sctp_inpcb *inp, struct sctp_ifa *ifa);

struct sctp_ifa *
sctp_source_address_selection(struct sctp_inpcb *inp,
    struct sctp_tcb *stcb,
    sctp_route_t * ro, struct sctp_nets *net,
    int non_asoc_addr_ok, uint32_t vrf_id);

int
    sctp_v6src_match_nexthop(struct sockaddr_in6 *src6, sctp_route_t * ro);
int
    sctp_v4src_match_nexthop(struct sctp_ifa *sifa, sctp_route_t * ro);

void 
sctp_send_initiate(struct sctp_inpcb *, struct sctp_tcb *, int
#if !defined(__APPLE__) && !defined(SCTP_SO_LOCK_TESTING)
    SCTP_UNUSED
#endif
);

void
sctp_send_initiate_ack(struct sctp_inpcb *, struct sctp_tcb *,
    struct mbuf *, int, int, struct sctphdr *, struct sctp_init_chunk *,
    uint32_t, uint16_t, int);

struct mbuf *
sctp_arethere_unrecognized_parameters(struct mbuf *, int, int *,
    struct sctp_chunkhdr *, int *);
void sctp_queue_op_err(struct sctp_tcb *, struct mbuf *);

int
sctp_send_cookie_echo(struct mbuf *, int, struct sctp_tcb *,
    struct sctp_nets *);

void sctp_send_cookie_ack(struct sctp_tcb *);

void
sctp_send_heartbeat_ack(struct sctp_tcb *, struct mbuf *, int, int,
    struct sctp_nets *);

void
sctp_remove_from_wheel(struct sctp_tcb *stcb,
    struct sctp_association *asoc,
    struct sctp_stream_out *strq, int holds_lock);


void sctp_send_shutdown(struct sctp_tcb *, struct sctp_nets *);

void sctp_send_shutdown_ack(struct sctp_tcb *, struct sctp_nets *);

void sctp_send_shutdown_complete(struct sctp_tcb *, struct sctp_nets *, int);

void 
sctp_send_shutdown_complete2(struct mbuf *, int, struct sctphdr *,
    uint32_t, uint16_t);

void sctp_send_asconf(struct sctp_tcb *, struct sctp_nets *, int addr_locked);

void sctp_send_asconf_ack(struct sctp_tcb *);

int sctp_get_frag_point(struct sctp_tcb *, struct sctp_association *);

void sctp_toss_old_cookies(struct sctp_tcb *, struct sctp_association *);

void sctp_toss_old_asconf(struct sctp_tcb *);

void sctp_fix_ecn_echo(struct sctp_association *);

void sctp_move_chunks_from_net(struct sctp_tcb *stcb, struct sctp_nets *net);

int
sctp_output(struct sctp_inpcb *, struct mbuf *, struct sockaddr *,
    struct mbuf *, struct thread *, int);

void 
sctp_chunk_output(struct sctp_inpcb *, struct sctp_tcb *, int, int
#if !defined(__APPLE__) && !defined(SCTP_SO_LOCK_TESTING)
    SCTP_UNUSED
#endif
);
void 
sctp_send_abort_tcb(struct sctp_tcb *, struct mbuf *, int
#if !defined(__APPLE__) && !defined(SCTP_SO_LOCK_TESTING)
    SCTP_UNUSED
#endif
);

void send_forward_tsn(struct sctp_tcb *, struct sctp_association *);

void sctp_send_sack(struct sctp_tcb *, int);

void sctp_send_hb(struct sctp_tcb *, struct sctp_nets *, int);

void sctp_send_ecn_echo(struct sctp_tcb *, struct sctp_nets *, uint32_t);


void
sctp_send_packet_dropped(struct sctp_tcb *, struct sctp_nets *, struct mbuf *,
    int, int);



void sctp_send_cwr(struct sctp_tcb *, struct sctp_nets *, uint32_t, uint8_t);


void
sctp_add_stream_reset_out(struct sctp_tmit_chunk *chk,
    int number_entries, uint16_t * list,
    uint32_t seq, uint32_t resp_seq, uint32_t last_sent);

void
sctp_add_stream_reset_in(struct sctp_tmit_chunk *chk,
    int number_entries, uint16_t * list,
    uint32_t seq);

void
sctp_add_stream_reset_tsn(struct sctp_tmit_chunk *chk,
    uint32_t seq);

void
sctp_add_stream_reset_result(struct sctp_tmit_chunk *chk,
    uint32_t resp_seq, uint32_t result);

void
sctp_add_stream_reset_result_tsn(struct sctp_tmit_chunk *chk,
    uint32_t resp_seq, uint32_t result,
    uint32_t send_una, uint32_t recv_next);

int
sctp_send_str_reset_req(struct sctp_tcb *stcb,
    int number_entries,
    uint16_t * list,
    uint8_t send_out_req,
    uint32_t resp_seq,
    uint8_t send_in_req,
    uint8_t send_tsn_req,
    uint8_t add_str,
    uint16_t adding);


void
sctp_send_abort(struct mbuf *, int, struct sctphdr *, uint32_t,
    struct mbuf *, uint32_t, uint16_t);

void sctp_send_operr_to(struct mbuf *, int, struct mbuf *, uint32_t, uint32_t, uint16_t);

#endif				/* _KERNEL || __Userspace__ */

#if defined(_KERNEL) || defined (__Userspace__)
int
sctp_sosend(struct socket *so,
    struct sockaddr *addr,
    struct uio *uio,
    struct mbuf *top,
    struct mbuf *control,
    int flags,
    struct thread *p
);

#endif
#endif
