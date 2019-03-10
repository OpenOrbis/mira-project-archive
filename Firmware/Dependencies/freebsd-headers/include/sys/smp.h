/*-
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@FreeBSD.org> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 *
 * $FreeBSD: release/9.0.0/sys/sys/smp.h 222813 2011-06-07 08:46:13Z attilio $
 */

#ifndef _SYS_SMP_H_
#define _SYS_SMP_H_

#ifdef _KERNEL

#ifndef LOCORE

#include <sys/cpuset.h>

/*
 * Topology of a NUMA or HTT system.
 *
 * The top level topology is an array of pointers to groups.  Each group
 * contains a bitmask of cpus in its group or subgroups.  It may also
 * contain a pointer to an array of child groups.
 *
 * The bitmasks at non leaf groups may be used by consumers who support
 * a smaller depth than the hardware provides.
 *
 * The topology may be omitted by systems where all CPUs are equal.
 */

struct cpu_group {
	struct cpu_group *cg_parent;	/* Our parent group. */
	struct cpu_group *cg_child;	/* Optional children groups. */
	cpuset_t	cg_mask;	/* Mask of cpus in this group. */
	int32_t		cg_count;	/* Count of cpus in this group. */
	int16_t		cg_children;	/* Number of children groups. */
	int8_t		cg_level;	/* Shared cache level. */
	int8_t		cg_flags;	/* Traversal modifiers. */
};

typedef struct cpu_group *cpu_group_t;

/*
 * Defines common resources for CPUs in the group.  The highest level
 * resource should be used when multiple are shared.
 */
#define	CG_SHARE_NONE	0
#define	CG_SHARE_L1	1
#define	CG_SHARE_L2	2
#define	CG_SHARE_L3	3

/*
 * Behavior modifiers for load balancing and affinity.
 */
#define	CG_FLAG_HTT	0x01		/* Schedule the alternate core last. */
#define	CG_FLAG_SMT	0x02		/* New age htt, less crippled. */
#define	CG_FLAG_THREAD	(CG_FLAG_HTT | CG_FLAG_SMT)	/* Any threading. */

/*
 * Convenience routines for building topologies.
 */
#ifdef SMP
struct cpu_group *smp_topo(void);
struct cpu_group *smp_topo_none(void);
struct cpu_group *smp_topo_1level(int l1share, int l1count, int l1flags);
struct cpu_group *smp_topo_2level(int l2share, int l2count, int l1share,
    int l1count, int l1flags);
struct cpu_group *smp_topo_find(struct cpu_group *top, int cpu);

extern void (*cpustop_restartfunc)(void);
extern int smp_active;
extern int smp_cpus;
extern volatile cpuset_t started_cpus;
extern volatile cpuset_t stopped_cpus;
extern cpuset_t hlt_cpus_mask;
extern cpuset_t logical_cpus_mask;
#endif /* SMP */

extern u_int mp_maxid;
extern int mp_maxcpus;
extern int mp_ncpus;
extern volatile int smp_started;

extern cpuset_t all_cpus;

/*
 * Macro allowing us to determine whether a CPU is absent at any given
 * time, thus permitting us to configure sparse maps of cpuid-dependent
 * (per-CPU) structures.
 */
#define	CPU_ABSENT(x_cpu)	(!CPU_ISSET(x_cpu, &all_cpus))

/*
 * Macros to iterate over non-absent CPUs.  CPU_FOREACH() takes an
 * integer iterator and iterates over the available set of CPUs.
 * CPU_FIRST() returns the id of the first non-absent CPU.  CPU_NEXT()
 * returns the id of the next non-absent CPU.  It will wrap back to
 * CPU_FIRST() once the end of the list is reached.  The iterators are
 * currently implemented via inline functions.
 */
#define	CPU_FOREACH(i)							\
	for ((i) = 0; (i) <= mp_maxid; (i)++)				\
		if (!CPU_ABSENT((i)))

static __inline int
cpu_first(void)
{
	int i;

	for (i = 0;; i++)
		if (!CPU_ABSENT(i))
			return (i);
}

static __inline int
cpu_next(int i)
{

	for (;;) {
		i++;
		if (i > mp_maxid)
			i = 0;
		if (!CPU_ABSENT(i))
			return (i);
	}
}

#define	CPU_FIRST()	cpu_first()
#define	CPU_NEXT(i)	cpu_next((i))

#ifdef SMP
/*
 * Machine dependent functions used to initialize MP support.
 *
 * The cpu_mp_probe() should check to see if MP support is present and return
 * zero if it is not or non-zero if it is.  If MP support is present, then
 * cpu_mp_start() will be called so that MP can be enabled.  This function
 * should do things such as startup secondary processors.  It should also
 * setup mp_ncpus, all_cpus, and smp_cpus.  It should also ensure that
 * smp_active and smp_started are initialized at the appropriate time.
 * Once cpu_mp_start() returns, machine independent MP startup code will be
 * executed and a simple message will be output to the console.  Finally,
 * cpu_mp_announce() will be called so that machine dependent messages about
 * the MP support may be output to the console if desired.
 *
 * The cpu_setmaxid() function is called very early during the boot process
 * so that the MD code may set mp_maxid to provide an upper bound on CPU IDs
 * that other subsystems may use.  If a platform is not able to determine
 * the exact maximum ID that early, then it may set mp_maxid to MAXCPU - 1.
 */
struct thread;

struct cpu_group *cpu_topo(void);
void	cpu_mp_announce(void);
int	cpu_mp_probe(void);
void	cpu_mp_setmaxid(void);
void	cpu_mp_start(void);

void	forward_signal(struct thread *);
int	restart_cpus(cpuset_t);
int	stop_cpus(cpuset_t);
int	stop_cpus_hard(cpuset_t);
#if defined(__amd64__)
int	suspend_cpus(cpuset_t);
#endif
void	smp_rendezvous_action(void);
extern	struct mtx smp_ipi_mtx;

#endif /* SMP */
void	smp_no_rendevous_barrier(void *);
void	smp_rendezvous(void (*)(void *), 
		       void (*)(void *),
		       void (*)(void *),
		       void *arg);
void	smp_rendezvous_cpus(cpuset_t,
		       void (*)(void *), 
		       void (*)(void *),
		       void (*)(void *),
		       void *arg);
#endif /* !LOCORE */
#endif /* _KERNEL */
#endif /* _SYS_SMP_H_ */
