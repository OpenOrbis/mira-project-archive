/*-
 * Copyright (c) 2009 Stanislav Sedov <stas@FreeBSD.org>
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
 * $FreeBSD: release/9.0.0/lib/libprocstat/libprocstat.h 224859 2011-08-14 00:42:09Z rwatson $
 */

#ifndef _LIBPROCSTAT_H_
#define	_LIBPROCSTAT_H_

/*
 * Vnode types.
 */
#define	PS_FST_VTYPE_VNON	1
#define	PS_FST_VTYPE_VREG	2
#define	PS_FST_VTYPE_VDIR	3
#define	PS_FST_VTYPE_VBLK	4
#define	PS_FST_VTYPE_VCHR	5
#define	PS_FST_VTYPE_VLNK	6
#define	PS_FST_VTYPE_VSOCK	7
#define	PS_FST_VTYPE_VFIFO	8
#define	PS_FST_VTYPE_VBAD	9
#define	PS_FST_VTYPE_UNKNOWN	255

/*
 * Descriptor types.
 */
#define	PS_FST_TYPE_VNODE	1
#define	PS_FST_TYPE_FIFO	2
#define	PS_FST_TYPE_SOCKET	3
#define	PS_FST_TYPE_PIPE	4
#define	PS_FST_TYPE_PTS		5
#define	PS_FST_TYPE_KQUEUE	6
#define	PS_FST_TYPE_CRYPTO	7
#define	PS_FST_TYPE_MQUEUE	8
#define	PS_FST_TYPE_SHM		9
#define	PS_FST_TYPE_SEM		10
#define	PS_FST_TYPE_UNKNOWN	11
#define	PS_FST_TYPE_NONE	12

/*
 * Special descriptor numbers.
 */
#define	PS_FST_UFLAG_RDIR	0x0001
#define	PS_FST_UFLAG_CDIR	0x0002
#define	PS_FST_UFLAG_JAIL	0x0004
#define	PS_FST_UFLAG_TRACE	0x0008
#define	PS_FST_UFLAG_TEXT	0x0010
#define	PS_FST_UFLAG_MMAP	0x0020
#define	PS_FST_UFLAG_CTTY	0x0040

/*
 * Descriptor flags.
 */
#define PS_FST_FFLAG_READ	0x0001
#define PS_FST_FFLAG_WRITE	0x0002
#define	PS_FST_FFLAG_NONBLOCK	0x0004
#define	PS_FST_FFLAG_APPEND	0x0008
#define	PS_FST_FFLAG_SHLOCK	0x0010
#define	PS_FST_FFLAG_EXLOCK	0x0020
#define	PS_FST_FFLAG_ASYNC	0x0040
#define	PS_FST_FFLAG_SYNC	0x0080
#define	PS_FST_FFLAG_NOFOLLOW	0x0100
#define	PS_FST_FFLAG_CREAT	0x0200
#define	PS_FST_FFLAG_TRUNC	0x0400
#define	PS_FST_FFLAG_EXCL	0x0800
#define	PS_FST_FFLAG_DIRECT	0x1000
#define	PS_FST_FFLAG_EXEC	0x2000
#define	PS_FST_FFLAG_HASLOCK	0x4000
#define	PS_FST_FFLAG_CAPABILITY	0x8000

struct procstat;
struct filestat {
	int	fs_type;	/* Descriptor type. */
	int	fs_flags;	/* filestat specific flags. */
	int	fs_fflags;	/* Descriptor access flags. */
	int	fs_uflags;	/* How this file is used. */
	int	fs_fd;		/* File descriptor number. */
	int	fs_ref_count;	/* Reference count. */
	off_t	fs_offset;	/* Seek location. */
	void	*fs_typedep;	/* Type dependent data. */
	char	*fs_path;
	STAILQ_ENTRY(filestat)	next;
	cap_rights_t	fs_cap_rights;	/* Capability rights, if flag set. */
};
struct vnstat {
	uint64_t	vn_fileid;
	uint64_t	vn_size;
	char		*vn_mntdir;
	uint32_t	vn_dev;
	uint32_t	vn_fsid;
	int		vn_type;
	uint16_t	vn_mode;
	char		vn_devname[SPECNAMELEN + 1];
};
struct ptsstat {
	uint32_t	dev;
	char		devname[SPECNAMELEN + 1];
};
struct pipestat {
	size_t		buffer_cnt;
	uint64_t	addr;
	uint64_t	peer;
};
struct sockstat {
	uint64_t	inp_ppcb;
	uint64_t	so_addr;
	uint64_t	so_pcb;
	uint64_t	unp_conn;
	int		dom_family;
	int		proto;
	int		so_rcv_sb_state;
	int		so_snd_sb_state;
	struct sockaddr_storage	sa_local;	/* Socket address. */
	struct sockaddr_storage	sa_peer;	/* Peer address. */
	int		type;
	char		dname[32];
};

STAILQ_HEAD(filestat_list, filestat);

void	procstat_close(struct procstat *procstat);
void	procstat_freeprocs(struct procstat *procstat, struct kinfo_proc *p);
void	procstat_freefiles(struct procstat *procstat,
    struct filestat_list *head);
struct filestat_list	*procstat_getfiles(struct procstat *procstat,
    struct kinfo_proc *kp, int mmapped);
struct kinfo_proc	*procstat_getprocs(struct procstat *procstat,
    int what, int arg, unsigned int *count);
int	procstat_get_pipe_info(struct procstat *procstat, struct filestat *fst,
    struct pipestat *pipe, char *errbuf);
int	procstat_get_pts_info(struct procstat *procstat, struct filestat *fst,
    struct ptsstat *pts, char *errbuf);
int	procstat_get_socket_info(struct procstat *procstat, struct filestat *fst,
    struct sockstat *sock, char *errbuf);
int	procstat_get_vnode_info(struct procstat *procstat, struct filestat *fst,
    struct vnstat *vn, char *errbuf);
struct procstat	*procstat_open_sysctl(void);
struct procstat	*procstat_open_kvm(const char *nlistf, const char *memf);

#endif	/* !_LIBPROCSTAT_H_ */
