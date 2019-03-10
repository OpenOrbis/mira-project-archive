/*-
 * Copyright (c) 1985, 1986, 1993
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
 *	@(#)in_var.h	8.2 (Berkeley) 1/9/95
 * $FreeBSD: release/9.0.0/sys/netinet/in_var.h 226572 2011-10-20 15:58:05Z glebius $
 */

#ifndef _NETINET_IN_VAR_H_
#define _NETINET_IN_VAR_H_

#include <sys/queue.h>
#include <sys/fnv_hash.h>
#include <sys/tree.h>

struct igmp_ifinfo;
struct in_multi;
struct lltable;

/*
 * IPv4 per-interface state.
 */
struct in_ifinfo {
	struct lltable		*ii_llt;	/* ARP state */
	struct igmp_ifinfo	*ii_igmp;	/* IGMP state */
	struct in_multi		*ii_allhosts;	/* 224.0.0.1 membership */
};

/*
 * Interface address, Internet version.  One of these structures
 * is allocated for each Internet address on an interface.
 * The ifaddr structure contains the protocol-independent part
 * of the structure and is assumed to be first.
 */
struct in_ifaddr {
	struct	ifaddr ia_ifa;		/* protocol-independent info */
#define	ia_ifp		ia_ifa.ifa_ifp
#define ia_flags	ia_ifa.ifa_flags
					/* ia_subnet{,mask} in host order */
	u_long	ia_subnet;		/* subnet address */
	u_long	ia_subnetmask;		/* mask of subnet */
	LIST_ENTRY(in_ifaddr) ia_hash;	/* entry in bucket of inet addresses */
	TAILQ_ENTRY(in_ifaddr) ia_link;	/* list of internet addresses */
	struct	sockaddr_in ia_addr;	/* reserve space for interface name */
	struct	sockaddr_in ia_dstaddr; /* reserve space for broadcast addr */
#define	ia_broadaddr	ia_dstaddr
	struct	sockaddr_in ia_sockmask; /* reserve space for general netmask */
};

struct	in_aliasreq {
	char	ifra_name[IFNAMSIZ];		/* if name, e.g. "en0" */
	struct	sockaddr_in ifra_addr;
	struct	sockaddr_in ifra_broadaddr;
#define ifra_dstaddr ifra_broadaddr
	struct	sockaddr_in ifra_mask;
};
/*
 * Given a pointer to an in_ifaddr (ifaddr),
 * return a pointer to the addr as a sockaddr_in.
 */
#define IA_SIN(ia)    (&(((struct in_ifaddr *)(ia))->ia_addr))
#define IA_DSTSIN(ia) (&(((struct in_ifaddr *)(ia))->ia_dstaddr))

#define IN_LNAOF(in, ifa) \
	((ntohl((in).s_addr) & ~((struct in_ifaddr *)(ifa)->ia_subnetmask))


#ifdef	_KERNEL
extern	u_char	inetctlerrmap[];

#define LLTABLE(ifp)	\
	((struct in_ifinfo *)(ifp)->if_afdata[AF_INET])->ii_llt
/*
 * Hash table for IP addresses.
 */
TAILQ_HEAD(in_ifaddrhead, in_ifaddr);
LIST_HEAD(in_ifaddrhashhead, in_ifaddr);

VNET_DECLARE(struct in_ifaddrhashhead *, in_ifaddrhashtbl);
VNET_DECLARE(struct in_ifaddrhead, in_ifaddrhead);
VNET_DECLARE(u_long, in_ifaddrhmask);		/* mask for hash table */

#define	V_in_ifaddrhashtbl	VNET(in_ifaddrhashtbl)
#define	V_in_ifaddrhead		VNET(in_ifaddrhead)
#define	V_in_ifaddrhmask	VNET(in_ifaddrhmask)

#define INADDR_NHASH_LOG2       9
#define INADDR_NHASH		(1 << INADDR_NHASH_LOG2)
#define INADDR_HASHVAL(x)	fnv_32_buf((&(x)), sizeof(x), FNV1_32_INIT)
#define INADDR_HASH(x) \
	(&V_in_ifaddrhashtbl[INADDR_HASHVAL(x) & V_in_ifaddrhmask])

extern	struct rwlock in_ifaddr_lock;

#define	IN_IFADDR_LOCK_ASSERT()	rw_assert(&in_ifaddr_lock, RA_LOCKED)
#define	IN_IFADDR_RLOCK()	rw_rlock(&in_ifaddr_lock)
#define	IN_IFADDR_RLOCK_ASSERT()	rw_assert(&in_ifaddr_lock, RA_RLOCKED)
#define	IN_IFADDR_RUNLOCK()	rw_runlock(&in_ifaddr_lock)
#define	IN_IFADDR_WLOCK()	rw_wlock(&in_ifaddr_lock)
#define	IN_IFADDR_WLOCK_ASSERT()	rw_assert(&in_ifaddr_lock, RA_WLOCKED)
#define	IN_IFADDR_WUNLOCK()	rw_wunlock(&in_ifaddr_lock)

/*
 * Macro for finding the internet address structure (in_ifaddr)
 * corresponding to one of our IP addresses (in_addr).
 */
#define INADDR_TO_IFADDR(addr, ia) \
	/* struct in_addr addr; */ \
	/* struct in_ifaddr *ia; */ \
do { \
\
	LIST_FOREACH(ia, INADDR_HASH((addr).s_addr), ia_hash) \
		if (IA_SIN(ia)->sin_addr.s_addr == (addr).s_addr) \
			break; \
} while (0)

/*
 * Macro for finding the interface (ifnet structure) corresponding to one
 * of our IP addresses.
 */
#define INADDR_TO_IFP(addr, ifp) \
	/* struct in_addr addr; */ \
	/* struct ifnet *ifp; */ \
{ \
	struct in_ifaddr *ia; \
\
	INADDR_TO_IFADDR(addr, ia); \
	(ifp) = (ia == NULL) ? NULL : ia->ia_ifp; \
}

/*
 * Macro for finding the internet address structure (in_ifaddr) corresponding
 * to a given interface (ifnet structure).
 */
#define IFP_TO_IA(ifp, ia)						\
	/* struct ifnet *ifp; */					\
	/* struct in_ifaddr *ia; */					\
{									\
	for ((ia) = TAILQ_FIRST(&V_in_ifaddrhead);			\
	    (ia) != NULL && (ia)->ia_ifp != (ifp);			\
	    (ia) = TAILQ_NEXT((ia), ia_link))				\
		continue;						\
	if ((ia) != NULL)						\
		ifa_ref(&(ia)->ia_ifa);					\
}
#endif

/*
 * IP datagram reassembly.
 */
#define	IPREASS_NHASH_LOG2	6
#define	IPREASS_NHASH		(1 << IPREASS_NHASH_LOG2)
#define	IPREASS_HMASK		(IPREASS_NHASH - 1)
#define	IPREASS_HASH(x,y) \
	(((((x) & 0xF) | ((((x) >> 8) & 0xF) << 4)) ^ (y)) & IPREASS_HMASK)

/*
 * Legacy IPv4 IGMP per-link structure.
 */
struct router_info {
	struct ifnet *rti_ifp;
	int    rti_type; /* type of router which is querier on this interface */
	int    rti_time; /* # of slow timeouts since last old query */
	SLIST_ENTRY(router_info) rti_list;
};

/*
 * Per-interface IGMP router version information.
 */
struct igmp_ifinfo {
	LIST_ENTRY(igmp_ifinfo) igi_link;
	struct ifnet *igi_ifp;	/* interface this instance belongs to */
	uint32_t igi_version;	/* IGMPv3 Host Compatibility Mode */
	uint32_t igi_v1_timer;	/* IGMPv1 Querier Present timer (s) */
	uint32_t igi_v2_timer;	/* IGMPv2 Querier Present timer (s) */
	uint32_t igi_v3_timer;	/* IGMPv3 General Query (interface) timer (s)*/
	uint32_t igi_flags;	/* IGMP per-interface flags */
	uint32_t igi_rv;	/* IGMPv3 Robustness Variable */
	uint32_t igi_qi;	/* IGMPv3 Query Interval (s) */
	uint32_t igi_qri;	/* IGMPv3 Query Response Interval (s) */
	uint32_t igi_uri;	/* IGMPv3 Unsolicited Report Interval (s) */
	SLIST_HEAD(,in_multi)	igi_relinmhead; /* released groups */
	struct ifqueue	 igi_gq;	/* queue of general query responses */
};

#define IGIF_SILENT	0x00000001	/* Do not use IGMP on this ifp */
#define IGIF_LOOPBACK	0x00000002	/* Send IGMP reports to loopback */

/*
 * IPv4 multicast IGMP-layer source entry.
 */
struct ip_msource {
	RB_ENTRY(ip_msource)	ims_link;	/* RB tree links */
	in_addr_t		ims_haddr;	/* host byte order */
	struct ims_st {
		uint16_t	ex;		/* # of exclusive members */
		uint16_t	in;		/* # of inclusive members */
	}			ims_st[2];	/* state at t0, t1 */
	uint8_t			ims_stp;	/* pending query */
};

/*
 * IPv4 multicast PCB-layer source entry.
 */
struct in_msource {
	RB_ENTRY(ip_msource)	ims_link;	/* RB tree links */
	in_addr_t		ims_haddr;	/* host byte order */
	uint8_t			imsl_st[2];	/* state before/at commit */
};

RB_HEAD(ip_msource_tree, ip_msource);	/* define struct ip_msource_tree */

static __inline int
ip_msource_cmp(const struct ip_msource *a, const struct ip_msource *b)
{

	if (a->ims_haddr < b->ims_haddr)
		return (-1);
	if (a->ims_haddr == b->ims_haddr)
		return (0);
	return (1);
}
RB_PROTOTYPE(ip_msource_tree, ip_msource, ims_link, ip_msource_cmp);

/*
 * IPv4 multicast PCB-layer group filter descriptor.
 */
struct in_mfilter {
	struct ip_msource_tree	imf_sources; /* source list for (S,G) */
	u_long			imf_nsrc;    /* # of source entries */
	uint8_t			imf_st[2];   /* state before/at commit */
};

/*
 * IPv4 group descriptor.
 *
 * For every entry on an ifnet's if_multiaddrs list which represents
 * an IP multicast group, there is one of these structures.
 *
 * If any source filters are present, then a node will exist in the RB-tree
 * to permit fast lookup by source whenever an operation takes place.
 * This permits pre-order traversal when we issue reports.
 * Source filter trees are kept separately from the socket layer to
 * greatly simplify locking.
 *
 * When IGMPv3 is active, inm_timer is the response to group query timer.
 * The state-change timer inm_sctimer is separate; whenever state changes
 * for the group the state change record is generated and transmitted,
 * and kept if retransmissions are necessary.
 *
 * FUTURE: inm_link is now only used when groups are being purged
 * on a detaching ifnet. It could be demoted to a SLIST_ENTRY, but
 * because it is at the very start of the struct, we can't do this
 * w/o breaking the ABI for ifmcstat.
 */
struct in_multi {
	LIST_ENTRY(in_multi) inm_link;	/* to-be-released by in_ifdetach */
	struct	in_addr inm_addr;	/* IP multicast address, convenience */
	struct	ifnet *inm_ifp;		/* back pointer to ifnet */
	struct	ifmultiaddr *inm_ifma;	/* back pointer to ifmultiaddr */
	u_int	inm_timer;		/* IGMPv1/v2 group / v3 query timer */
	u_int	inm_state;		/* state of the membership */
	void	*inm_rti;		/* unused, legacy field */
	u_int	inm_refcount;		/* reference count */

	/* New fields for IGMPv3 follow. */
	struct igmp_ifinfo	*inm_igi;	/* IGMP info */
	SLIST_ENTRY(in_multi)	 inm_nrele;	/* to-be-released by IGMP */
	struct ip_msource_tree	 inm_srcs;	/* tree of sources */
	u_long			 inm_nsrc;	/* # of tree entries */

	struct ifqueue		 inm_scq;	/* queue of pending
						 * state-change packets */
	struct timeval		 inm_lastgsrtv;	/* Time of last G-S-R query */
	uint16_t		 inm_sctimer;	/* state-change timer */
	uint16_t		 inm_scrv;	/* state-change rexmit count */

	/*
	 * SSM state counters which track state at T0 (the time the last
	 * state-change report's RV timer went to zero) and T1
	 * (time of pending report, i.e. now).
	 * Used for computing IGMPv3 state-change reports. Several refcounts
	 * are maintained here to optimize for common use-cases.
	 */
	struct inm_st {
		uint16_t	iss_fmode;	/* IGMP filter mode */
		uint16_t	iss_asm;	/* # of ASM listeners */
		uint16_t	iss_ex;		/* # of exclusive members */
		uint16_t	iss_in;		/* # of inclusive members */
		uint16_t	iss_rec;	/* # of recorded sources */
	}			inm_st[2];	/* state at t0, t1 */
};

/*
 * Helper function to derive the filter mode on a source entry
 * from its internal counters. Predicates are:
 *  A source is only excluded if all listeners exclude it.
 *  A source is only included if no listeners exclude it,
 *  and at least one listener includes it.
 * May be used by ifmcstat(8).
 */
static __inline uint8_t
ims_get_mode(const struct in_multi *inm, const struct ip_msource *ims,
    uint8_t t)
{

	t = !!t;
	if (inm->inm_st[t].iss_ex > 0 &&
	    inm->inm_st[t].iss_ex == ims->ims_st[t].ex)
		return (MCAST_EXCLUDE);
	else if (ims->ims_st[t].in > 0 && ims->ims_st[t].ex == 0)
		return (MCAST_INCLUDE);
	return (MCAST_UNDEFINED);
}

#ifdef _KERNEL

#ifdef SYSCTL_DECL
SYSCTL_DECL(_net_inet);
SYSCTL_DECL(_net_inet_ip);
SYSCTL_DECL(_net_inet_raw);
#endif

/*
 * Lock macros for IPv4 layer multicast address lists.  IPv4 lock goes
 * before link layer multicast locks in the lock order.  In most cases,
 * consumers of IN_*_MULTI() macros should acquire the locks before
 * calling them; users of the in_{add,del}multi() functions should not.
 */
extern struct mtx in_multi_mtx;
#define	IN_MULTI_LOCK()		mtx_lock(&in_multi_mtx)
#define	IN_MULTI_UNLOCK()	mtx_unlock(&in_multi_mtx)
#define	IN_MULTI_LOCK_ASSERT()	mtx_assert(&in_multi_mtx, MA_OWNED)
#define	IN_MULTI_UNLOCK_ASSERT() mtx_assert(&in_multi_mtx, MA_NOTOWNED)

/*
 * Function for looking up an in_multi record for an IPv4 multicast address
 * on a given interface. ifp must be valid. If no record found, return NULL.
 * The IN_MULTI_LOCK and IF_ADDR_LOCK on ifp must be held.
 */
static __inline struct in_multi *
inm_lookup_locked(struct ifnet *ifp, const struct in_addr ina)
{
	struct ifmultiaddr *ifma;
	struct in_multi *inm;

	IN_MULTI_LOCK_ASSERT();
	IF_ADDR_LOCK_ASSERT(ifp);

	inm = NULL;
	TAILQ_FOREACH(ifma, &((ifp)->if_multiaddrs), ifma_link) {
		if (ifma->ifma_addr->sa_family == AF_INET) {
			inm = (struct in_multi *)ifma->ifma_protospec;
			if (inm->inm_addr.s_addr == ina.s_addr)
				break;
			inm = NULL;
		}
	}
	return (inm);
}

/*
 * Wrapper for inm_lookup_locked().
 * The IF_ADDR_LOCK will be taken on ifp and released on return.
 */
static __inline struct in_multi *
inm_lookup(struct ifnet *ifp, const struct in_addr ina)
{
	struct in_multi *inm;

	IN_MULTI_LOCK_ASSERT();
	IF_ADDR_LOCK(ifp);
	inm = inm_lookup_locked(ifp, ina);
	IF_ADDR_UNLOCK(ifp);

	return (inm);
}

/* Acquire an in_multi record. */
static __inline void
inm_acquire_locked(struct in_multi *inm)
{

	IN_MULTI_LOCK_ASSERT();
	++inm->inm_refcount;
}

/*
 * Return values for imo_multi_filter().
 */
#define MCAST_PASS		0	/* Pass */
#define MCAST_NOTGMEMBER	1	/* This host not a member of group */
#define MCAST_NOTSMEMBER	2	/* This host excluded source */
#define MCAST_MUTED		3	/* [deprecated] */

struct	rtentry;
struct	route;
struct	ip_moptions;

int	imo_multi_filter(const struct ip_moptions *, const struct ifnet *,
	    const struct sockaddr *, const struct sockaddr *);
void	inm_commit(struct in_multi *);
void	inm_clear_recorded(struct in_multi *);
void	inm_print(const struct in_multi *);
int	inm_record_source(struct in_multi *inm, const in_addr_t);
void	inm_release(struct in_multi *);
void	inm_release_locked(struct in_multi *);
struct	in_multi *
	in_addmulti(struct in_addr *, struct ifnet *);
void	in_delmulti(struct in_multi *);
int	in_joingroup(struct ifnet *, const struct in_addr *,
	    /*const*/ struct in_mfilter *, struct in_multi **);
int	in_joingroup_locked(struct ifnet *, const struct in_addr *,
	    /*const*/ struct in_mfilter *, struct in_multi **);
int	in_leavegroup(struct in_multi *, /*const*/ struct in_mfilter *);
int	in_leavegroup_locked(struct in_multi *,
	    /*const*/ struct in_mfilter *);
int	in_control(struct socket *, u_long, caddr_t, struct ifnet *,
	    struct thread *);
void	in_rtqdrain(void);
void	ip_input(struct mbuf *);
int	in_ifadown(struct ifaddr *ifa, int);
void	in_ifscrub(struct ifnet *, struct in_ifaddr *, u_int);
struct	mbuf	*ip_fastforward(struct mbuf *);
void	*in_domifattach(struct ifnet *);
void	in_domifdetach(struct ifnet *, void *);


/* XXX */
void	 in_rtalloc_ign(struct route *ro, u_long ignflags, u_int fibnum);
void	 in_rtalloc(struct route *ro, u_int fibnum);
struct rtentry *in_rtalloc1(struct sockaddr *, int, u_long, u_int);
void	 in_rtredirect(struct sockaddr *, struct sockaddr *,
	    struct sockaddr *, int, struct sockaddr *, u_int);
int	 in_rtrequest(int, struct sockaddr *,
	    struct sockaddr *, struct sockaddr *, int, struct rtentry **, u_int);

#if 0
int	 in_rt_getifa(struct rt_addrinfo *, u_int fibnum);
int	 in_rtioctl(u_long, caddr_t, u_int);
int	 in_rtrequest1(int, struct rt_addrinfo *, struct rtentry **, u_int);
#endif
#endif /* _KERNEL */

/* INET6 stuff */
#include <netinet6/in6_var.h>

#endif /* _NETINET_IN_VAR_H_ */
