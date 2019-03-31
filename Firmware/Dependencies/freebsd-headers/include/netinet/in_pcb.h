/*-
 * Copyright (c) 1982, 1986, 1990, 1993
 *	The Regents of the University of California.
 * Copyright (c) 2010-2011 Juniper Networks, Inc.
 * All rights reserved.
 *
 * Portions of this software were developed by Robert N. M. Watson under
 * contract to Juniper Networks, Inc.
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
 *	@(#)in_pcb.h	8.1 (Berkeley) 6/10/93
 * $FreeBSD: release/9.0.0/sys/netinet/in_pcb.h 227428 2011-11-10 20:28:30Z trociny $
 */

#ifndef _NETINET_IN_PCB_H_
#define _NETINET_IN_PCB_H_

#include <sys/queue.h>
#include <sys/_lock.h>
#include <sys/_mutex.h>
#include <sys/_rwlock.h>

#ifdef _KERNEL
#include <sys/lock.h>
#include <sys/rwlock.h>
#include <net/vnet.h>
#include <vm/uma.h>
#endif

#define	in6pcb		inpcb	/* for KAME src sync over BSD*'s */
#define	in6p_sp		inp_sp	/* for KAME src sync over BSD*'s */
struct inpcbpolicy;

/*
 * struct inpcb is the common protocol control block structure used in most
 * IP transport protocols.
 *
 * Pointers to local and foreign host table entries, local and foreign socket
 * numbers, and pointers up (to a socket structure) and down (to a
 * protocol-specific control block) are stored here.
 */
LIST_HEAD(inpcbhead, inpcb);
LIST_HEAD(inpcbporthead, inpcbport);
typedef	u_quad_t	inp_gen_t;

/*
 * PCB with AF_INET6 null bind'ed laddr can receive AF_INET input packet.
 * So, AF_INET6 null laddr is also used as AF_INET null laddr, by utilizing
 * the following structure.
 */
struct in_addr_4in6 {
	u_int32_t	ia46_pad32[3];
	struct	in_addr	ia46_addr4;
};

/*
 * NOTE: ipv6 addrs should be 64-bit aligned, per RFC 2553.  in_conninfo has
 * some extra padding to accomplish this.
 */
struct in_endpoints {
	u_int16_t	ie_fport;		/* foreign port */
	u_int16_t	ie_lport;		/* local port */
	/* protocol dependent part, local and foreign addr */
	union {
		/* foreign host table entry */
		struct	in_addr_4in6 ie46_foreign;
		struct	in6_addr ie6_foreign;
	} ie_dependfaddr;
	union {
		/* local host table entry */
		struct	in_addr_4in6 ie46_local;
		struct	in6_addr ie6_local;
	} ie_dependladdr;
};
#define	ie_faddr	ie_dependfaddr.ie46_foreign.ia46_addr4
#define	ie_laddr	ie_dependladdr.ie46_local.ia46_addr4
#define	ie6_faddr	ie_dependfaddr.ie6_foreign
#define	ie6_laddr	ie_dependladdr.ie6_local

/*
 * XXX The defines for inc_* are hacks and should be changed to direct
 * references.
 */
struct in_conninfo {
	u_int8_t	inc_flags;
	u_int8_t	inc_len;
	u_int16_t	inc_fibnum;	/* XXX was pad, 16 bits is plenty */
	/* protocol dependent part */
	struct	in_endpoints inc_ie;
};

/*
 * Flags for inc_flags.
 */
#define	INC_ISIPV6	0x01

#define inc_isipv6	inc_flags	/* temp compatability */
#define	inc_fport	inc_ie.ie_fport
#define	inc_lport	inc_ie.ie_lport
#define	inc_faddr	inc_ie.ie_faddr
#define	inc_laddr	inc_ie.ie_laddr
#define	inc6_faddr	inc_ie.ie6_faddr
#define	inc6_laddr	inc_ie.ie6_laddr

struct	icmp6_filter;

/*-
 * struct inpcb captures the network layer state for TCP, UDP, and raw IPv4
 * and IPv6 sockets.  In the case of TCP, further per-connection state is
 * hung off of inp_ppcb most of the time.  Almost all fields of struct inpcb
 * are static after creation or protected by a per-inpcb rwlock, inp_lock.  A
 * few fields also require the global pcbinfo lock for the inpcb to be held,
 * when modified, such as the global connection lists and hashes, as well as
 * binding information (which affects which hash a connection is on).  This
 * model means that connections can be looked up without holding the
 * per-connection lock, which is important for performance when attempting to
 * find the connection for a packet given its IP and port tuple.  Writing to
 * these fields that write locks be held on both the inpcb and global locks.
 *
 * Key:
 * (c) - Constant after initialization
 * (g) - Protected by the pcbgroup lock
 * (i) - Protected by the inpcb lock
 * (p) - Protected by the pcbinfo lock for the inpcb
 * (s) - Protected by another subsystem's locks
 * (x) - Undefined locking
 *
 * A few other notes:
 *
 * When a read lock is held, stability of the field is guaranteed; to write
 * to a field, a write lock must generally be held.
 *
 * netinet/netinet6-layer code should not assume that the inp_socket pointer
 * is safe to dereference without inp_lock being held, even for protocols
 * other than TCP (where the inpcb persists during TIMEWAIT even after the
 * socket has been freed), or there may be close(2)-related races.
 *
 * The inp_vflag field is overloaded, and would otherwise ideally be (c).
 */
struct inpcb {
	LIST_ENTRY(inpcb) inp_hash;	/* (i/p) hash list */
	LIST_ENTRY(inpcb) inp_pcbgrouphash;	/* (g/i) hash list */
	LIST_ENTRY(inpcb) inp_list;	/* (i/p) list for all PCBs for proto */
	void	*inp_ppcb;		/* (i) pointer to per-protocol pcb */
	struct	inpcbinfo *inp_pcbinfo;	/* (c) PCB list info */
	struct	inpcbgroup *inp_pcbgroup; /* (g/i) PCB group list */
	LIST_ENTRY(inpcb) inp_pcbgroup_wild; /* (g/i/p) group wildcard entry */
	struct	socket *inp_socket;	/* (i) back pointer to socket */
	struct	ucred	*inp_cred;	/* (c) cache of socket cred */
	u_int32_t inp_flow;		/* (i) IPv6 flow information */
	int	inp_flags;		/* (i) generic IP/datagram flags */
	int	inp_flags2;		/* (i) generic IP/datagram flags #2*/
	u_char	inp_vflag;		/* (i) IP version flag (v4/v6) */
	u_char	inp_ip_ttl;		/* (i) time to live proto */
	u_char	inp_ip_p;		/* (c) protocol proto */
	u_char	inp_ip_minttl;		/* (i) minimum TTL or drop */
	uint32_t inp_flowid;		/* (x) flow id / queue id */
	u_int	inp_refcount;		/* (i) refcount */
	void	*inp_pspare[5];		/* (x) route caching / general use */
	u_int	inp_ispare[6];		/* (x) route caching / user cookie /
					 *     general use */

	/* Local and foreign ports, local and foreign addr. */
	struct	in_conninfo inp_inc;	/* (i/p) list for PCB's local port */

	/* MAC and IPSEC policy information. */
	struct	label *inp_label;	/* (i) MAC label */
	struct	inpcbpolicy *inp_sp;    /* (s) for IPSEC */

	/* Protocol-dependent part; options. */
	struct {
		u_char	inp4_ip_tos;		/* (i) type of service proto */
		struct	mbuf *inp4_options;	/* (i) IP options */
		struct	ip_moptions *inp4_moptions; /* (i) IP mcast options */
	} inp_depend4;
	struct {
		/* (i) IP options */
		struct	mbuf *inp6_options;
		/* (i) IP6 options for outgoing packets */
		struct	ip6_pktopts *inp6_outputopts;
		/* (i) IP multicast options */
		struct	ip6_moptions *inp6_moptions;
		/* (i) ICMPv6 code type filter */
		struct	icmp6_filter *inp6_icmp6filt;
		/* (i) IPV6_CHECKSUM setsockopt */
		int	inp6_cksum;
		short	inp6_hops;
	} inp_depend6;
	LIST_ENTRY(inpcb) inp_portlist;	/* (i/p) */
	struct	inpcbport *inp_phd;	/* (i/p) head of this list */
#define inp_zero_size offsetof(struct inpcb, inp_gencnt)
	inp_gen_t	inp_gencnt;	/* (c) generation count */
	struct llentry	*inp_lle;	/* cached L2 information */
	struct rtentry	*inp_rt;	/* cached L3 information */
	struct rwlock	inp_lock;
};
#define	inp_fport	inp_inc.inc_fport
#define	inp_lport	inp_inc.inc_lport
#define	inp_faddr	inp_inc.inc_faddr
#define	inp_laddr	inp_inc.inc_laddr
#define	inp_ip_tos	inp_depend4.inp4_ip_tos
#define	inp_options	inp_depend4.inp4_options
#define	inp_moptions	inp_depend4.inp4_moptions

#define	in6p_faddr	inp_inc.inc6_faddr
#define	in6p_laddr	inp_inc.inc6_laddr
#define	in6p_hops	inp_depend6.inp6_hops	/* default hop limit */
#define	in6p_flowinfo	inp_flow
#define	in6p_options	inp_depend6.inp6_options
#define	in6p_outputopts	inp_depend6.inp6_outputopts
#define	in6p_moptions	inp_depend6.inp6_moptions
#define	in6p_icmp6filt	inp_depend6.inp6_icmp6filt
#define	in6p_cksum	inp_depend6.inp6_cksum

#define	inp_vnet	inp_pcbinfo->ipi_vnet

/*
 * The range of the generation count, as used in this implementation, is 9e19.
 * We would have to create 300 billion connections per second for this number
 * to roll over in a year.  This seems sufficiently unlikely that we simply
 * don't concern ourselves with that possibility.
 */

/*
 * Interface exported to userland by various protocols which use inpcbs.  Hack
 * alert -- only define if struct xsocket is in scope.
 */
#ifdef _SYS_SOCKETVAR_H_
struct	xinpcb {
	size_t	xi_len;		/* length of this structure */
	struct	inpcb xi_inp;
	struct	xsocket xi_socket;
	u_quad_t	xi_alignment_hack;
};

struct	xinpgen {
	size_t	xig_len;	/* length of this structure */
	u_int	xig_count;	/* number of PCBs at this time */
	inp_gen_t xig_gen;	/* generation count at this time */
	so_gen_t xig_sogen;	/* socket generation count at this time */
};
#endif /* _SYS_SOCKETVAR_H_ */

struct inpcbport {
	LIST_ENTRY(inpcbport) phd_hash;
	struct inpcbhead phd_pcblist;
	u_short phd_port;
};

/*-
 * Global data structure for each high-level protocol (UDP, TCP, ...) in both
 * IPv4 and IPv6.  Holds inpcb lists and information for managing them.
 *
 * Each pcbinfo is protected by two locks: ipi_lock and ipi_hash_lock,
 * the former covering mutable global fields (such as the global pcb list),
 * and the latter covering the hashed lookup tables.  The lock order is:
 *
 *    ipi_lock (before) inpcb locks (before) {ipi_hash_lock, pcbgroup locks}
 *
 * Locking key:
 *
 * (c) Constant or nearly constant after initialisation
 * (g) Locked by ipi_lock
 * (h) Read using either ipi_hash_lock or inpcb lock; write requires both
 * (p) Protected by one or more pcbgroup locks
 * (x) Synchronisation properties poorly defined
 */
struct inpcbinfo {
	/*
	 * Global lock protecting global inpcb list, inpcb count, etc.
	 */
	struct rwlock		 ipi_lock;

	/*
	 * Global list of inpcbs on the protocol.
	 */
	struct inpcbhead	*ipi_listhead;		/* (g) */
	u_int			 ipi_count;		/* (g) */

	/*
	 * Generation count -- incremented each time a connection is allocated
	 * or freed.
	 */
	u_quad_t		 ipi_gencnt;		/* (g) */

	/*
	 * Fields associated with port lookup and allocation.
	 */
	u_short			 ipi_lastport;		/* (x) */
	u_short			 ipi_lastlow;		/* (x) */
	u_short			 ipi_lasthi;		/* (x) */

	/*
	 * UMA zone from which inpcbs are allocated for this protocol.
	 */
	struct	uma_zone	*ipi_zone;		/* (c) */

	/*
	 * Connection groups associated with this protocol.  These fields are
	 * constant, but pcbgroup structures themselves are protected by
	 * per-pcbgroup locks.
	 */
	struct inpcbgroup	*ipi_pcbgroups;		/* (c) */
	u_int			 ipi_npcbgroups;	/* (c) */
	u_int			 ipi_hashfields;	/* (c) */

	/*
	 * Global lock protecting non-pcbgroup hash lookup tables.
	 */
	struct rwlock		 ipi_hash_lock;

	/*
	 * Global hash of inpcbs, hashed by local and foreign addresses and
	 * port numbers.
	 */
	struct inpcbhead	*ipi_hashbase;		/* (h) */
	u_long			 ipi_hashmask;		/* (h) */

	/*
	 * Global hash of inpcbs, hashed by only local port number.
	 */
	struct inpcbporthead	*ipi_porthashbase;	/* (h) */
	u_long			 ipi_porthashmask;	/* (h) */

	/*
	 * List of wildcard inpcbs for use with pcbgroups.  In the past, was
	 * per-pcbgroup but is now global.  All pcbgroup locks must be held
	 * to modify the list, so any is sufficient to read it.
	 */
	struct inpcbhead	*ipi_wildbase;		/* (p) */
	u_long			 ipi_wildmask;		/* (p) */

	/*
	 * Pointer to network stack instance
	 */
	struct vnet		*ipi_vnet;		/* (c) */

	/*
	 * general use 2
	 */
	void 			*ipi_pspare[2];
};

/*
 * Connection groups hold sets of connections that have similar CPU/thread
 * affinity.  Each connection belongs to exactly one connection group.
 */
struct inpcbgroup {
	/*
	 * Per-connection group hash of inpcbs, hashed by local and foreign
	 * addresses and port numbers.
	 */
	struct inpcbhead	*ipg_hashbase;		/* (c) */
	u_long			 ipg_hashmask;		/* (c) */

	/*
	 * Notional affinity of this pcbgroup.
	 */
	u_int			 ipg_cpu;		/* (p) */

	/*
	 * Per-connection group lock, not to be confused with ipi_lock.
	 * Protects the hash table hung off the group, but also the global
	 * wildcard list in inpcbinfo.
	 */
	struct mtx		 ipg_lock;
} __aligned(CACHE_LINE_SIZE);

#define INP_LOCK_INIT(inp, d, t) \
	rw_init_flags(&(inp)->inp_lock, (t), RW_RECURSE |  RW_DUPOK)
#define INP_LOCK_DESTROY(inp)	rw_destroy(&(inp)->inp_lock)
#define INP_RLOCK(inp)		rw_rlock(&(inp)->inp_lock)
#define INP_WLOCK(inp)		rw_wlock(&(inp)->inp_lock)
#define INP_TRY_RLOCK(inp)	rw_try_rlock(&(inp)->inp_lock)
#define INP_TRY_WLOCK(inp)	rw_try_wlock(&(inp)->inp_lock)
#define INP_RUNLOCK(inp)	rw_runlock(&(inp)->inp_lock)
#define INP_WUNLOCK(inp)	rw_wunlock(&(inp)->inp_lock)
#define	INP_TRY_UPGRADE(inp)	rw_try_upgrade(&(inp)->inp_lock)
#define	INP_DOWNGRADE(inp)	rw_downgrade(&(inp)->inp_lock)
#define	INP_WLOCKED(inp)	rw_wowned(&(inp)->inp_lock)
#define	INP_LOCK_ASSERT(inp)	rw_assert(&(inp)->inp_lock, RA_LOCKED)
#define	INP_RLOCK_ASSERT(inp)	rw_assert(&(inp)->inp_lock, RA_RLOCKED)
#define	INP_WLOCK_ASSERT(inp)	rw_assert(&(inp)->inp_lock, RA_WLOCKED)
#define	INP_UNLOCK_ASSERT(inp)	rw_assert(&(inp)->inp_lock, RA_UNLOCKED)

#ifdef _KERNEL
/*
 * These locking functions are for inpcb consumers outside of sys/netinet,
 * more specifically, they were added for the benefit of TOE drivers. The
 * macros are reserved for use by the stack.
 */
void inp_wlock(struct inpcb *);
void inp_wunlock(struct inpcb *);
void inp_rlock(struct inpcb *);
void inp_runlock(struct inpcb *);

#ifdef INVARIANTS
void inp_lock_assert(struct inpcb *);
void inp_unlock_assert(struct inpcb *);
#else
static __inline void
inp_lock_assert(struct inpcb *inp __unused)
{
}

static __inline void
inp_unlock_assert(struct inpcb *inp __unused)
{
}

#endif

void	inp_apply_all(void (*func)(struct inpcb *, void *), void *arg);
int 	inp_ip_tos_get(const struct inpcb *inp);
void 	inp_ip_tos_set(struct inpcb *inp, int val);
struct socket *
	inp_inpcbtosocket(struct inpcb *inp);
struct tcpcb *
	inp_inpcbtotcpcb(struct inpcb *inp);
void 	inp_4tuple_get(struct inpcb *inp, uint32_t *laddr, uint16_t *lp,
		uint32_t *faddr, uint16_t *fp);

#endif /* _KERNEL */

#define INP_INFO_LOCK_INIT(ipi, d) \
	rw_init_flags(&(ipi)->ipi_lock, (d), RW_RECURSE)
#define INP_INFO_LOCK_DESTROY(ipi)  rw_destroy(&(ipi)->ipi_lock)
#define INP_INFO_RLOCK(ipi)	rw_rlock(&(ipi)->ipi_lock)
#define INP_INFO_WLOCK(ipi)	rw_wlock(&(ipi)->ipi_lock)
#define INP_INFO_TRY_RLOCK(ipi)	rw_try_rlock(&(ipi)->ipi_lock)
#define INP_INFO_TRY_WLOCK(ipi)	rw_try_wlock(&(ipi)->ipi_lock)
#define INP_INFO_TRY_UPGRADE(ipi)	rw_try_upgrade(&(ipi)->ipi_lock)
#define INP_INFO_RUNLOCK(ipi)	rw_runlock(&(ipi)->ipi_lock)
#define INP_INFO_WUNLOCK(ipi)	rw_wunlock(&(ipi)->ipi_lock)
#define	INP_INFO_LOCK_ASSERT(ipi)	rw_assert(&(ipi)->ipi_lock, RA_LOCKED)
#define INP_INFO_RLOCK_ASSERT(ipi)	rw_assert(&(ipi)->ipi_lock, RA_RLOCKED)
#define INP_INFO_WLOCK_ASSERT(ipi)	rw_assert(&(ipi)->ipi_lock, RA_WLOCKED)
#define INP_INFO_UNLOCK_ASSERT(ipi)	rw_assert(&(ipi)->ipi_lock, RA_UNLOCKED)

#define	INP_HASH_LOCK_INIT(ipi, d) \
	rw_init_flags(&(ipi)->ipi_hash_lock, (d), 0)
#define	INP_HASH_LOCK_DESTROY(ipi)	rw_destroy(&(ipi)->ipi_hash_lock)
#define	INP_HASH_RLOCK(ipi)		rw_rlock(&(ipi)->ipi_hash_lock)
#define	INP_HASH_WLOCK(ipi)		rw_wlock(&(ipi)->ipi_hash_lock)
#define	INP_HASH_RUNLOCK(ipi)		rw_runlock(&(ipi)->ipi_hash_lock)
#define	INP_HASH_WUNLOCK(ipi)		rw_wunlock(&(ipi)->ipi_hash_lock)
#define	INP_HASH_LOCK_ASSERT(ipi)	rw_assert(&(ipi)->ipi_hash_lock, \
					    RA_LOCKED)
#define	INP_HASH_WLOCK_ASSERT(ipi)	rw_assert(&(ipi)->ipi_hash_lock, \
					    RA_WLOCKED)

#define	INP_GROUP_LOCK_INIT(ipg, d)	mtx_init(&(ipg)->ipg_lock, (d), NULL, \
					    MTX_DEF | MTX_DUPOK)
#define	INP_GROUP_LOCK_DESTROY(ipg)	mtx_destroy(&(ipg)->ipg_lock)

#define	INP_GROUP_LOCK(ipg)		mtx_lock(&(ipg)->ipg_lock)
#define	INP_GROUP_LOCK_ASSERT(ipg)	mtx_assert(&(ipg)->ipg_lock, MA_OWNED)
#define	INP_GROUP_UNLOCK(ipg)		mtx_unlock(&(ipg)->ipg_lock)

#define INP_PCBHASH(faddr, lport, fport, mask) \
	(((faddr) ^ ((faddr) >> 16) ^ ntohs((lport) ^ (fport))) & (mask))
#define INP_PCBPORTHASH(lport, mask) \
	(ntohs((lport)) & (mask))

/*
 * Flags for inp_vflags -- historically version flags only
 */
#define	INP_IPV4	0x1
#define	INP_IPV6	0x2
#define	INP_IPV6PROTO	0x4		/* opened under IPv6 protocol */

/*
 * Flags for inp_flags.
 */
#define	INP_RECVOPTS		0x00000001 /* receive incoming IP options */
#define	INP_RECVRETOPTS		0x00000002 /* receive IP options for reply */
#define	INP_RECVDSTADDR		0x00000004 /* receive IP dst address */
#define	INP_HDRINCL		0x00000008 /* user supplies entire IP header */
#define	INP_HIGHPORT		0x00000010 /* user wants "high" port binding */
#define	INP_LOWPORT		0x00000020 /* user wants "low" port binding */
#define	INP_ANONPORT		0x00000040 /* port chosen for user */
#define	INP_RECVIF		0x00000080 /* receive incoming interface */
#define	INP_MTUDISC		0x00000100 /* user can do MTU discovery */
#define	INP_FAITH		0x00000200 /* accept FAITH'ed connections */
#define	INP_RECVTTL		0x00000400 /* receive incoming IP TTL */
#define	INP_DONTFRAG		0x00000800 /* don't fragment packet */
#define	INP_BINDANY		0x00001000 /* allow bind to any address */
#define	INP_INHASHLIST		0x00002000 /* in_pcbinshash() has been called */
#define	IN6P_IPV6_V6ONLY	0x00008000 /* restrict AF_INET6 socket for v6 */
#define	IN6P_PKTINFO		0x00010000 /* receive IP6 dst and I/F */
#define	IN6P_HOPLIMIT		0x00020000 /* receive hoplimit */
#define	IN6P_HOPOPTS		0x00040000 /* receive hop-by-hop options */
#define	IN6P_DSTOPTS		0x00080000 /* receive dst options after rthdr */
#define	IN6P_RTHDR		0x00100000 /* receive routing header */
#define	IN6P_RTHDRDSTOPTS	0x00200000 /* receive dstoptions before rthdr */
#define	IN6P_TCLASS		0x00400000 /* receive traffic class value */
#define	IN6P_AUTOFLOWLABEL	0x00800000 /* attach flowlabel automatically */
#define	INP_TIMEWAIT		0x01000000 /* in TIMEWAIT, ppcb is tcptw */
#define	INP_ONESBCAST		0x02000000 /* send all-ones broadcast */
#define	INP_DROPPED		0x04000000 /* protocol drop flag */
#define	INP_SOCKREF		0x08000000 /* strong socket reference */
#define	INP_SW_FLOWID           0x10000000 /* software generated flow id */
#define	INP_HW_FLOWID           0x20000000 /* hardware generated flow id */
#define	IN6P_RFC2292		0x40000000 /* used RFC2292 API on the socket */
#define	IN6P_MTU		0x80000000 /* receive path MTU */

#define	INP_CONTROLOPTS		(INP_RECVOPTS|INP_RECVRETOPTS|INP_RECVDSTADDR|\
				 INP_RECVIF|INP_RECVTTL|\
				 IN6P_PKTINFO|IN6P_HOPLIMIT|IN6P_HOPOPTS|\
				 IN6P_DSTOPTS|IN6P_RTHDR|IN6P_RTHDRDSTOPTS|\
				 IN6P_TCLASS|IN6P_AUTOFLOWLABEL|IN6P_RFC2292|\
				 IN6P_MTU)

/*
 * Flags for inp_flags2.
 */
#define	INP_LLE_VALID		0x00000001 /* cached lle is valid */	
#define	INP_RT_VALID		0x00000002 /* cached rtentry is valid */
#define	INP_PCBGROUPWILD	0x00000004 /* in pcbgroup wildcard list */
#define	INP_REUSEPORT		0x00000008 /* SO_REUSEPORT option is set */

/*
 * Flags passed to in_pcblookup*() functions.
 */
#define	INPLOOKUP_WILDCARD	0x00000001	/* Allow wildcard sockets. */
#define	INPLOOKUP_RLOCKPCB	0x00000002	/* Return inpcb read-locked. */
#define	INPLOOKUP_WLOCKPCB	0x00000004	/* Return inpcb write-locked. */

#define	INPLOOKUP_MASK	(INPLOOKUP_WILDCARD | INPLOOKUP_RLOCKPCB | \
			    INPLOOKUP_WLOCKPCB)

#define	sotoinpcb(so)	((struct inpcb *)(so)->so_pcb)
#define	sotoin6pcb(so)	sotoinpcb(so) /* for KAME src sync over BSD*'s */

#define	INP_SOCKAF(so) so->so_proto->pr_domain->dom_family

#define	INP_CHECK_SOCKAF(so, af)	(INP_SOCKAF(so) == af)

/*
 * Constants for pcbinfo.ipi_hashfields.
 */
#define	IPI_HASHFIELDS_NONE	0
#define	IPI_HASHFIELDS_2TUPLE	1
#define	IPI_HASHFIELDS_4TUPLE	2

#ifdef _KERNEL
VNET_DECLARE(int, ipport_reservedhigh);
VNET_DECLARE(int, ipport_reservedlow);
VNET_DECLARE(int, ipport_lowfirstauto);
VNET_DECLARE(int, ipport_lowlastauto);
VNET_DECLARE(int, ipport_firstauto);
VNET_DECLARE(int, ipport_lastauto);
VNET_DECLARE(int, ipport_hifirstauto);
VNET_DECLARE(int, ipport_hilastauto);
VNET_DECLARE(int, ipport_randomized);
VNET_DECLARE(int, ipport_randomcps);
VNET_DECLARE(int, ipport_randomtime);
VNET_DECLARE(int, ipport_stoprandom);
VNET_DECLARE(int, ipport_tcpallocs);

#define	V_ipport_reservedhigh	VNET(ipport_reservedhigh)
#define	V_ipport_reservedlow	VNET(ipport_reservedlow)
#define	V_ipport_lowfirstauto	VNET(ipport_lowfirstauto)
#define	V_ipport_lowlastauto	VNET(ipport_lowlastauto)
#define	V_ipport_firstauto	VNET(ipport_firstauto)
#define	V_ipport_lastauto	VNET(ipport_lastauto)
#define	V_ipport_hifirstauto	VNET(ipport_hifirstauto)
#define	V_ipport_hilastauto	VNET(ipport_hilastauto)
#define	V_ipport_randomized	VNET(ipport_randomized)
#define	V_ipport_randomcps	VNET(ipport_randomcps)
#define	V_ipport_randomtime	VNET(ipport_randomtime)
#define	V_ipport_stoprandom	VNET(ipport_stoprandom)
#define	V_ipport_tcpallocs	VNET(ipport_tcpallocs)

void	in_pcbinfo_destroy(struct inpcbinfo *);
void	in_pcbinfo_init(struct inpcbinfo *, const char *, struct inpcbhead *,
	    int, int, char *, uma_init, uma_fini, uint32_t, u_int);

struct inpcbgroup *
	in_pcbgroup_byhash(struct inpcbinfo *, u_int, uint32_t);
struct inpcbgroup *
	in_pcbgroup_byinpcb(struct inpcb *);
struct inpcbgroup *
	in_pcbgroup_bytuple(struct inpcbinfo *, struct in_addr, u_short,
	    struct in_addr, u_short);
void	in_pcbgroup_destroy(struct inpcbinfo *);
int	in_pcbgroup_enabled(struct inpcbinfo *);
void	in_pcbgroup_init(struct inpcbinfo *, u_int, int);
void	in_pcbgroup_remove(struct inpcb *);
void	in_pcbgroup_update(struct inpcb *);
void	in_pcbgroup_update_mbuf(struct inpcb *, struct mbuf *);

void	in_pcbpurgeif0(struct inpcbinfo *, struct ifnet *);
int	in_pcballoc(struct socket *, struct inpcbinfo *);
int	in_pcbbind(struct inpcb *, struct sockaddr *, struct ucred *);
int	in_pcb_lport(struct inpcb *, struct in_addr *, u_short *,
	    struct ucred *, int);
int	in_pcbbind_setup(struct inpcb *, struct sockaddr *, in_addr_t *,
	    u_short *, struct ucred *);
int	in_pcbconnect(struct inpcb *, struct sockaddr *, struct ucred *);
int	in_pcbconnect_mbuf(struct inpcb *, struct sockaddr *, struct ucred *,
	    struct mbuf *);
int	in_pcbconnect_setup(struct inpcb *, struct sockaddr *, in_addr_t *,
	    u_short *, in_addr_t *, u_short *, struct inpcb **,
	    struct ucred *);
void	in_pcbdetach(struct inpcb *);
void	in_pcbdisconnect(struct inpcb *);
void	in_pcbdrop(struct inpcb *);
void	in_pcbfree(struct inpcb *);
int	in_pcbinshash(struct inpcb *);
int	in_pcbinshash_nopcbgroup(struct inpcb *);
struct inpcb *
	in_pcblookup_local(struct inpcbinfo *,
	    struct in_addr, u_short, int, struct ucred *);
struct inpcb *
	in_pcblookup(struct inpcbinfo *, struct in_addr, u_int,
	    struct in_addr, u_int, int, struct ifnet *);
struct inpcb *
	in_pcblookup_mbuf(struct inpcbinfo *, struct in_addr, u_int,
	    struct in_addr, u_int, int, struct ifnet *, struct mbuf *);
void	in_pcbnotifyall(struct inpcbinfo *pcbinfo, struct in_addr,
	    int, struct inpcb *(*)(struct inpcb *, int));
void	in_pcbref(struct inpcb *);
void	in_pcbrehash(struct inpcb *);
void	in_pcbrehash_mbuf(struct inpcb *, struct mbuf *);
int	in_pcbrele(struct inpcb *);
int	in_pcbrele_rlocked(struct inpcb *);
int	in_pcbrele_wlocked(struct inpcb *);
void	in_pcbsetsolabel(struct socket *so);
int	in_getpeeraddr(struct socket *so, struct sockaddr **nam);
int	in_getsockaddr(struct socket *so, struct sockaddr **nam);
struct sockaddr *
	in_sockaddr(in_port_t port, struct in_addr *addr);
void	in_pcbsosetlabel(struct socket *so);
#endif /* _KERNEL */

#endif /* !_NETINET_IN_PCB_H_ */
