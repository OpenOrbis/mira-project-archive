/*-
 * Copyright (c) 1999 Boris Popov
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: release/9.0.0/sys/netncp/ncp_rcfile.h 206361 2010-04-07 16:50:38Z joel $
 */
#ifndef _NETNCP_NCP_RCFILE_H_
#define _NETNCP_NCP_RCFILE_H_
#include <sys/queue.h>

struct rckey {
	SLIST_ENTRY(rckey)	rk_next;
	char 			*rk_name;
	char			*rk_value;
};

struct rcsection {
	SLIST_ENTRY(rcsection)	rs_next;
	SLIST_HEAD(rckey_head,rckey) rs_keys;
	char			*rs_name;
};
    
struct rcfile {
	SLIST_ENTRY(rcfile)	rf_next;
	SLIST_HEAD(rcsec_head, rcsection) rf_sect;
	char			*rf_name;
	FILE			*rf_f;
};

__BEGIN_DECLS

int  rc_open(char *, char *,struct rcfile **);
int  rc_close(struct rcfile *);
int  rc_getstringptr(struct rcfile *, char *, char *, char **);
int  rc_getstring(struct rcfile *, char *, char *, int, char *);
int  rc_getint(struct rcfile *, char *, char *, int *);
int  rc_getbool(struct rcfile *, char *, char *, int *);

__END_DECLS

#endif	/* _NETNCP_NCP_RCFILE_H_ */
