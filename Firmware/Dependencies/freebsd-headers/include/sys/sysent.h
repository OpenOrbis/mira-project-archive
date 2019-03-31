/*-
 * Copyright (c) 1982, 1988, 1991 The Regents of the University of California.
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
 * $FreeBSD: release/9.0.0/sys/sys/sysent.h 225617 2011-09-16 13:58:51Z kmacy $
 */

#ifndef _SYS_SYSENT_H_
#define	_SYS_SYSENT_H_

#include <bsm/audit.h>

struct rlimit;
struct sysent;
struct thread;
struct ksiginfo;

typedef	int	sy_call_t(struct thread *, void *);

/* Used by the machine dependent syscall() code. */
typedef	void (*systrace_probe_func_t)(u_int32_t, int, struct sysent *, void *,
    int);

/*
 * Used by loaded syscalls to convert arguments to a DTrace array
 * of 64-bit arguments.
 */
typedef	void (*systrace_args_func_t)(int, void *, u_int64_t *, int *);

extern systrace_probe_func_t	systrace_probe_func;

struct sysent {			/* system call table */
	int	sy_narg;	/* number of arguments */
	sy_call_t *sy_call;	/* implementing function */
	au_event_t sy_auevent;	/* audit event associated with syscall */
	systrace_args_func_t sy_systrace_args_func;
				/* optional argument conversion function. */
	u_int32_t sy_entry;	/* DTrace entry ID for systrace. */
	u_int32_t sy_return;	/* DTrace return ID for systrace. */
	u_int32_t sy_flags;	/* General flags for system calls. */
	u_int32_t sy_thrcnt;
};

/*
 * A system call is permitted in capability mode.
 */
#define	SYF_CAPENABLED	0x00000001

#define	SY_THR_FLAGMASK	0x7
#define	SY_THR_STATIC	0x1
#define	SY_THR_DRAINING	0x2
#define	SY_THR_ABSENT	0x4
#define	SY_THR_INCR	0x8

struct image_params;
struct __sigset;
struct syscall_args;
struct trapframe;
struct vnode;

struct sysentvec {
	int		sv_size;	/* number of entries */
	struct sysent	*sv_table;	/* pointer to sysent */
	u_int		sv_mask;	/* optional mask to index */
	int		sv_sigsize;	/* size of signal translation table */
	int		*sv_sigtbl;	/* signal translation table */
	int		sv_errsize;	/* size of errno translation table */
	int 		*sv_errtbl;	/* errno translation table */
	int		(*sv_transtrap)(int, int);
					/* translate trap-to-signal mapping */
	int		(*sv_fixup)(register_t **, struct image_params *);
					/* stack fixup function */
	void		(*sv_sendsig)(void (*)(int), struct ksiginfo *, struct __sigset *);
			    		/* send signal */
	char 		*sv_sigcode;	/* start of sigtramp code */
	int 		*sv_szsigcode;	/* size of sigtramp code */
	void		(*sv_prepsyscall)(struct trapframe *, int *, u_int *,
			    caddr_t *);
	char		*sv_name;	/* name of binary type */
	int		(*sv_coredump)(struct thread *, struct vnode *, off_t, int);
					/* function to dump core, or NULL */
	int		(*sv_imgact_try)(struct image_params *);
	int		sv_minsigstksz;	/* minimum signal stack size */
	int		sv_pagesize;	/* pagesize */
	vm_offset_t	sv_minuser;	/* VM_MIN_ADDRESS */
	vm_offset_t	sv_maxuser;	/* VM_MAXUSER_ADDRESS */
	vm_offset_t	sv_usrstack;	/* USRSTACK */
	vm_offset_t	sv_psstrings;	/* PS_STRINGS */
	int		sv_stackprot;	/* vm protection for stack */
	register_t	*(*sv_copyout_strings)(struct image_params *);
	void		(*sv_setregs)(struct thread *, struct image_params *,
			    u_long);
	void		(*sv_fixlimit)(struct rlimit *, int);
	u_long		*sv_maxssiz;
	u_int		sv_flags;
	void		(*sv_set_syscall_retval)(struct thread *, int);
	int		(*sv_fetch_syscall_args)(struct thread *, struct
			    syscall_args *);
	const char	**sv_syscallnames;
	vm_offset_t	sv_shared_page_base;
	vm_offset_t	sv_shared_page_len;
	vm_offset_t	sv_sigcode_base;
	void		*sv_shared_page_obj;
	void		(*sv_schedtail)(struct thread *);
};

#define	SV_ILP32	0x000100
#define	SV_LP64		0x000200
#define	SV_IA32		0x004000
#define	SV_AOUT		0x008000
#define	SV_SHP		0x010000

#define	SV_ABI_MASK	0xff
#define	SV_PROC_FLAG(p, x)	((p)->p_sysent->sv_flags & (x))
#define	SV_PROC_ABI(p)		((p)->p_sysent->sv_flags & SV_ABI_MASK)
#define	SV_CURPROC_FLAG(x)	SV_PROC_FLAG(curproc, x)
#define	SV_CURPROC_ABI()	SV_PROC_ABI(curproc)
/* same as ELFOSABI_XXX, to prevent header pollution */
#define	SV_ABI_LINUX	3
#define	SV_ABI_FREEBSD 	9
#define	SV_ABI_UNDEF	255

#ifdef _KERNEL
extern struct sysentvec aout_sysvec;
extern struct sysentvec elf_freebsd_sysvec;
extern struct sysentvec null_sysvec;
extern struct sysent sysent[];
extern const char *syscallnames[];

#define	NO_SYSCALL (-1)

struct module;

struct syscall_module_data {
	int	(*chainevh)(struct module *, int, void *); /* next handler */
	void	*chainarg;		/* arg for next event handler */
	int	*offset;		/* offset into sysent */
	struct sysent *new_sysent;	/* new sysent */
	struct sysent old_sysent;	/* old sysent */
};

#define	MAKE_SYSENT(syscallname)				\
static struct sysent syscallname##_sysent = {			\
	(sizeof(struct syscallname ## _args )			\
	    / sizeof(register_t)),				\
	(sy_call_t *)& sys_##syscallname,	       		\
	SYS_AUE_##syscallname					\
}

#define	MAKE_SYSENT_COMPAT(syscallname)				\
static struct sysent syscallname##_sysent = {			\
	(sizeof(struct syscallname ## _args )			\
	    / sizeof(register_t)),				\
	(sy_call_t *)& syscallname,				\
	SYS_AUE_##syscallname					\
}

#define SYSCALL_MODULE(name, offset, new_sysent, evh, arg)	\
static struct syscall_module_data name##_syscall_mod = {	\
	evh, arg, offset, new_sysent, { 0, NULL, AUE_NULL }	\
};								\
								\
static moduledata_t name##_mod = {				\
	"sys/" #name,						\
	syscall_module_handler,					\
	&name##_syscall_mod					\
};								\
DECLARE_MODULE(name, name##_mod, SI_SUB_SYSCALLS, SI_ORDER_MIDDLE)

#define	SYSCALL_MODULE_HELPER(syscallname)			\
static int syscallname##_syscall = SYS_##syscallname;		\
MAKE_SYSENT(syscallname);					\
SYSCALL_MODULE(syscallname,					\
    & syscallname##_syscall, & syscallname##_sysent,		\
    NULL, NULL)

#define	SYSCALL_MODULE_PRESENT(syscallname)				\
	(sysent[SYS_##syscallname].sy_call != (sy_call_t *)lkmnosys &&	\
	sysent[SYS_##syscallname].sy_call != (sy_call_t *)lkmressys)

/*
 * Syscall registration helpers with resource allocation handling.
 */
struct syscall_helper_data {
	struct sysent new_sysent;
	struct sysent old_sysent;
	int syscall_no;
	int registered;
};
#define SYSCALL_INIT_HELPER(syscallname) {			\
    .new_sysent = {						\
	.sy_narg = (sizeof(struct syscallname ## _args )	\
	    / sizeof(register_t)),				\
	.sy_call = (sy_call_t *)& sys_ ## syscallname,		\
	.sy_auevent = SYS_AUE_##syscallname			\
    },								\
    .syscall_no = SYS_##syscallname				\
}
#define SYSCALL_INIT_HELPER_COMPAT(syscallname) {		\
    .new_sysent = {						\
	.sy_narg = (sizeof(struct syscallname ## _args )	\
	    / sizeof(register_t)),				\
	.sy_call = (sy_call_t *)& syscallname,			\
	.sy_auevent = SYS_AUE_##syscallname			\
    },								\
    .syscall_no = SYS_##syscallname				\
}
#define SYSCALL_INIT_LAST {					\
    .syscall_no = NO_SYSCALL					\
}

int	syscall_register(int *offset, struct sysent *new_sysent,
	    struct sysent *old_sysent);
int	syscall_deregister(int *offset, struct sysent *old_sysent);
int	syscall_module_handler(struct module *mod, int what, void *arg);
int	syscall_helper_register(struct syscall_helper_data *sd);
int	syscall_helper_unregister(struct syscall_helper_data *sd);

struct proc;
const char *syscallname(struct proc *p, u_int code);

/* Special purpose system call functions. */
struct nosys_args;

int	lkmnosys(struct thread *, struct nosys_args *);
int	lkmressys(struct thread *, struct nosys_args *);

int	syscall_thread_enter(struct thread *td, struct sysent *se);
void	syscall_thread_exit(struct thread *td, struct sysent *se);

int shared_page_fill(int size, int align, const char *data);
void exec_sysvec_init(void *param);

#define INIT_SYSENTVEC(name, sv)					\
    SYSINIT(name, SI_SUB_EXEC, SI_ORDER_ANY,				\
	(sysinit_cfunc_t)exec_sysvec_init, sv);

#endif /* _KERNEL */

#endif /* !_SYS_SYSENT_H_ */
