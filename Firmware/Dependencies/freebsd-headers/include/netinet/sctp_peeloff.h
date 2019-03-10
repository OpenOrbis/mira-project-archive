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

/* $KAME: sctp_peeloff.h,v 1.6 2005/03/06 16:04:18 itojun Exp $	 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD: release/9.0.0/sys/netinet/sctp_peeloff.h 218319 2011-02-05 12:12:51Z rrs $");

#ifndef __sctp_peeloff_h__
#define __sctp_peeloff_h__




#if defined(_KERNEL)

int sctp_can_peel_off(struct socket *, sctp_assoc_t);
int sctp_do_peeloff(struct socket *, struct socket *, sctp_assoc_t);
struct socket *sctp_get_peeloff(struct socket *, sctp_assoc_t, int *);



#endif				/* _KERNEL */

#endif
