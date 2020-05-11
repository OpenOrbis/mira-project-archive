/*-
 * Copyright (c) 2003-2004 Benno Rice <benno@FreeBSD.org>
 * All Rights Reserved.
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR  ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR  BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: release/9.0.0/sys/netgraph/ng_atmllc.h 139823 2005-01-07 01:45:51Z imp $
 */

#ifndef	_NETGRAPH_ATMLLC_H_
#define	_NETGRAPH_ATMLLC_H_

/* Node type name and magic cookie. */
#define	NG_ATMLLC_NODE_TYPE	"atmllc"
#define	NGM_ATMLLC_COOKIE	1065246274

/* Hook names. */
#define	NG_ATMLLC_HOOK_ATM	"atm"
#define	NG_ATMLLC_HOOK_ETHER	"ether"
#define	NG_ATMLLC_HOOK_802_4	"ieee8024"
#define	NG_ATMLLC_HOOK_802_5	"ieee8025"
#define	NG_ATMLLC_HOOK_802_6	"ieee8026"
#define	NG_ATMLLC_HOOK_FDDI	"fddi"
#define	NG_ATMLLC_HOOK_BPDU	"bpdu"

#endif /* _NETGRAPH_ATMLLC_H_ */
