/*	$NetBSD: ntfsmount.h,v 1.3 1999/07/26 14:02:32 jdolecek Exp $	*/

/*-
 * Copyright (c) 1998, 1999 Semen Ustimenko
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
 * $FreeBSD: release/9.0.0/sys/fs/ntfs/ntfsmount.h 213664 2010-10-10 07:05:47Z kib $
 */

#define	NTFS_MFLAG_CASEINS	0x00000001
#define	NTFS_MFLAG_ALLNAMES	0x00000002
#define	NTFS_MFLAG_KICONV	0x00000004

struct ntfs_args {
	char	*fspec;			/* block special device to mount */
	struct	oexport_args export;	/* network export information */
	uid_t	uid;			/* uid that owns ntfs files */
	gid_t	gid;			/* gid that owns ntfs files */
	mode_t	mode;			/* mask to be applied for ntfs perms */
	u_long	flag;			/* additional flags */
	char	*cs_ntfs;		/* NTFS Charset */
	char	*cs_local;		/* Local Charset */
};
