///*-
// * Copyright (c) 2003 Peter Wemm.
// * Copyright (c) 1993 The Regents of the University of California.
// * All rights reserved.
// *
// * Redistribution and use in source and binary forms, with or without
// * modification, are permitted provided that the following conditions
// * are met:
// * 1. Redistributions of source code must retain the above copyright
// *    notice, this list of conditions and the following disclaimer.
// * 2. Redistributions in binary form must reproduce the above copyright
// *    notice, this list of conditions and the following disclaimer in the
// *    documentation and/or other materials provided with the distribution.
// * 4. Neither the name of the University nor the names of its contributors
// *    may be used to endorse or promote products derived from this software
// *    without specific prior written permission.
// *
// * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
// * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
// * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// * SUCH DAMAGE.
// *
// * $FreeBSD: release/9.0.0/sys/amd64/include/cpufunc.h 223796 2011-07-05 18:42:10Z jkim $
// */
//
///*
// * Functions to provide access to special i386 instructions.
// * This in included in sys/systm.h, and that file should be
// * used in preference to this.
// */
//
//#ifndef _MACHINE_CPUFUNC_H_
//#define	_MACHINE_CPUFUNC_H_
//
//#ifndef _SYS_CDEFS_H_
//#error this file needs sys/cdefs.h as a prerequisite
//#endif
//
//struct region_descriptor;
//
//#define readb(va)	(*(volatile uint8_t *) (va))
//#define readw(va)	(*(volatile uint16_t *) (va))
//#define readl(va)	(*(volatile uint32_t *) (va))
//#define readq(va)	(*(volatile uint64_t *) (va))
//
//#define writeb(va, d)	(*(volatile uint8_t *) (va) = (d))
//#define writew(va, d)	(*(volatile uint16_t *) (va) = (d))
//#define writel(va, d)	(*(volatile uint32_t *) (va) = (d))
//#define writeq(va, d)	(*(volatile uint64_t *) (va) = (d))
//
//#if defined(__GNUCLIKE_ASM) && defined(__CC_SUPPORTS___INLINE)
//
//static __inline void
//breakpoint(void)
//{
//	__asm __volatile("int $3");
//}
//
//static __inline u_int
//bsfl(u_int mask)
//{
//	u_int	result;
//
//	__asm __volatile("bsfl %1,%0" : "=r" (result) : "rm" (mask));
//	return (result);
//}
//
//static __inline u_long
//bsfq(u_long mask)
//{
//	u_long	result;
//
//	__asm __volatile("bsfq %1,%0" : "=r" (result) : "rm" (mask));
//	return (result);
//}
//
//static __inline u_int
//bsrl(u_int mask)
//{
//	u_int	result;
//
//	__asm __volatile("bsrl %1,%0" : "=r" (result) : "rm" (mask));
//	return (result);
//}
//
//static __inline u_long
//bsrq(u_long mask)
//{
//	u_long	result;
//
//	__asm __volatile("bsrq %1,%0" : "=r" (result) : "rm" (mask));
//	return (result);
//}
//
//static __inline void
//clflush(u_long addr)
//{
//
//	__asm __volatile("clflush %0" : : "m" (*(char *)addr));
//}
//
//static __inline void
//disable_intr(void)
//{
//	__asm __volatile("cli" : : : "memory");
//}
//
//static __inline void
//do_cpuid(u_int ax, u_int *p)
//{
//	__asm __volatile("cpuid"
//			 : "=a" (p[0]), "=b" (p[1]), "=c" (p[2]), "=d" (p[3])
//			 :  "0" (ax));
//}
//
//static __inline void
//cpuid_count(u_int ax, u_int cx, u_int *p)
//{
//	__asm __volatile("cpuid"
//			 : "=a" (p[0]), "=b" (p[1]), "=c" (p[2]), "=d" (p[3])
//			 :  "0" (ax), "c" (cx));
//}
//
//static __inline void
//enable_intr(void)
//{
//	__asm __volatile("sti");
//}
//
//#ifdef _KERNEL
//
//#define	HAVE_INLINE_FFS
//#define        ffs(x)  __builtin_ffs(x)
//
//#define	HAVE_INLINE_FFSL
//
//static __inline int
//ffsl(long mask)
//{
//	return (mask == 0 ? mask : (int)bsfq((u_long)mask) + 1);
//}
//
//#define	HAVE_INLINE_FLS
//
//static __inline int
//fls(int mask)
//{
//	return (mask == 0 ? mask : (int)bsrl((u_int)mask) + 1);
//}
//
//#define	HAVE_INLINE_FLSL
//
//static __inline int
//flsl(long mask)
//{
//	return (mask == 0 ? mask : (int)bsrq((u_long)mask) + 1);
//}
//
//#endif /* _KERNEL */
//
//static __inline void
//halt(void)
//{
//	__asm __volatile("hlt");
//}
//
//static __inline u_char
//inb(u_int port)
//{
//	u_char	data;
//
//	__asm __volatile("inb %w1, %0" : "=a" (data) : "Nd" (port));
//	return (data);
//}
//
//static __inline u_int
//inl(u_int port)
//{
//	u_int	data;
//
//	__asm __volatile("inl %w1, %0" : "=a" (data) : "Nd" (port));
//	return (data);
//}
//
//static __inline void
//insb(u_int port, void *addr, size_t count)
//{
//	__asm __volatile("cld; rep; insb"
//			 : "+D" (addr), "+c" (count)
//			 : "d" (port)
//			 : "memory");
//}
//
//static __inline void
//insw(u_int port, void *addr, size_t count)
//{
//	__asm __volatile("cld; rep; insw"
//			 : "+D" (addr), "+c" (count)
//			 : "d" (port)
//			 : "memory");
//}
//
//static __inline void
//insl(u_int port, void *addr, size_t count)
//{
//	__asm __volatile("cld; rep; insl"
//			 : "+D" (addr), "+c" (count)
//			 : "d" (port)
//			 : "memory");
//}
//
//static __inline void
//invd(void)
//{
//	__asm __volatile("invd");
//}
//
//static __inline u_short
//inw(u_int port)
//{
//	u_short	data;
//
//	__asm __volatile("inw %w1, %0" : "=a" (data) : "Nd" (port));
//	return (data);
//}
//
//static __inline void
//outb(u_int port, u_char data)
//{
//	__asm __volatile("outb %0, %w1" : : "a" (data), "Nd" (port));
//}
//
//static __inline void
//outl(u_int port, u_int data)
//{
//	__asm __volatile("outl %0, %w1" : : "a" (data), "Nd" (port));
//}
//
//static __inline void
//outsb(u_int port, const void *addr, size_t count)
//{
//	__asm __volatile("cld; rep; outsb"
//			 : "+S" (addr), "+c" (count)
//			 : "d" (port));
//}
//
//static __inline void
//outsw(u_int port, const void *addr, size_t count)
//{
//	__asm __volatile("cld; rep; outsw"
//			 : "+S" (addr), "+c" (count)
//			 : "d" (port));
//}
//
//static __inline void
//outsl(u_int port, const void *addr, size_t count)
//{
//	__asm __volatile("cld; rep; outsl"
//			 : "+S" (addr), "+c" (count)
//			 : "d" (port));
//}
//
//static __inline void
//outw(u_int port, u_short data)
//{
//	__asm __volatile("outw %0, %w1" : : "a" (data), "Nd" (port));
//}
//
//static __inline void
//mfence(void)
//{
//
//	__asm __volatile("mfence" : : : "memory");
//}
//
//static __inline void
//ia32_pause(void)
//{
//	__asm __volatile("pause");
//}
//
//static __inline u_long
//read_rflags(void)
//{
//	u_long	rf;
//
//	__asm __volatile("pushfq; popq %0" : "=r" (rf));
//	return (rf);
//}
//
//static __inline uint64_t
//rdmsr(u_int msr)
//{
//	uint32_t low, high;
//
//	__asm __volatile("rdmsr" : "=a" (low), "=d" (high) : "c" (msr));
//	return (low | ((uint64_t)high << 32));
//}
//
//static __inline uint64_t
//rdpmc(u_int pmc)
//{
//	uint32_t low, high;
//
//	__asm __volatile("rdpmc" : "=a" (low), "=d" (high) : "c" (pmc));
//	return (low | ((uint64_t)high << 32));
//}
//
//static __inline uint64_t
//rdtsc(void)
//{
//	uint32_t low, high;
//
//	__asm __volatile("rdtsc" : "=a" (low), "=d" (high));
//	return (low | ((uint64_t)high << 32));
//}
//
//static __inline uint32_t
//rdtsc32(void)
//{
//	uint32_t rv;
//
//	__asm __volatile("rdtsc" : "=a" (rv) : : "edx");
//	return (rv);
//}
//
//static __inline void
//wbinvd(void)
//{
//	__asm __volatile("wbinvd");
//}
//
//static __inline void
//write_rflags(u_long rf)
//{
//	__asm __volatile("pushq %0;  popfq" : : "r" (rf));
//}
//
//static __inline void
//wrmsr(u_int msr, uint64_t newval)
//{
//	uint32_t low, high;
//
//	low = newval;
//	high = newval >> 32;
//	__asm __volatile("wrmsr" : : "a" (low), "d" (high), "c" (msr));
//}
//
//static __inline void
//load_cr0(u_long data)
//{
//
//	__asm __volatile("movq %0,%%cr0" : : "r" (data));
//}
//
//static __inline u_long
//rcr0(void)
//{
//	u_long	data;
//
//	__asm __volatile("movq %%cr0,%0" : "=r" (data));
//	return (data);
//}
//
//static __inline u_long
//rcr2(void)
//{
//	u_long	data;
//
//	__asm __volatile("movq %%cr2,%0" : "=r" (data));
//	return (data);
//}
//
//static __inline void
//load_cr3(u_long data)
//{
//
//	__asm __volatile("movq %0,%%cr3" : : "r" (data) : "memory");
//}
//
//static __inline u_long
//rcr3(void)
//{
//	u_long	data;
//
//	__asm __volatile("movq %%cr3,%0" : "=r" (data));
//	return (data);
//}
//
//static __inline void
//load_cr4(u_long data)
//{
//	__asm __volatile("movq %0,%%cr4" : : "r" (data));
//}
//
//static __inline u_long
//rcr4(void)
//{
//	u_long	data;
//
//	__asm __volatile("movq %%cr4,%0" : "=r" (data));
//	return (data);
//}
//
///*
// * Global TLB flush (except for thise for pages marked PG_G)
// */
//static __inline void
//invltlb(void)
//{
//
//	load_cr3(rcr3());
//}
//
///*
// * TLB flush for an individual page (even if it has PG_G).
// * Only works on 486+ CPUs (i386 does not have PG_G).
// */
//static __inline void
//invlpg(u_long addr)
//{
//
//	__asm __volatile("invlpg %0" : : "m" (*(char *)addr) : "memory");
//}
//
//static __inline u_short
//rfs(void)
//{
//	u_short sel;
//	__asm __volatile("movw %%fs,%0" : "=rm" (sel));
//	return (sel);
//}
//
//static __inline u_short
//rgs(void)
//{
//	u_short sel;
//	__asm __volatile("movw %%gs,%0" : "=rm" (sel));
//	return (sel);
//}
//
//static __inline u_short
//rss(void)
//{
//	u_short sel;
//	__asm __volatile("movw %%ss,%0" : "=rm" (sel));
//	return (sel);
//}
//
//static __inline void
//load_ds(u_short sel)
//{
//	__asm __volatile("movw %0,%%ds" : : "rm" (sel));
//}
//
//static __inline void
//load_es(u_short sel)
//{
//	__asm __volatile("movw %0,%%es" : : "rm" (sel));
//}
//
//static __inline void
//cpu_monitor(const void *addr, u_long extensions, u_int hints)
//{
//
//	__asm __volatile("monitor"
//	    : : "a" (addr), "c" (extensions), "d" (hints));
//}
//
//static __inline void
//cpu_mwait(u_long extensions, u_int hints)
//{
//
//	__asm __volatile("mwait" : : "a" (hints), "c" (extensions));
//}
//
//#ifdef _KERNEL
///* This is defined in <machine/specialreg.h> but is too painful to get to */
//#ifndef	MSR_FSBASE
//#define	MSR_FSBASE	0xc0000100
//#endif
//static __inline void
//load_fs(u_short sel)
//{
//	/* Preserve the fsbase value across the selector load */
//	__asm __volatile("rdmsr; movw %0,%%fs; wrmsr"
//	    : : "rm" (sel), "c" (MSR_FSBASE) : "eax", "edx");
//}
//
//#ifndef	MSR_GSBASE
//#define	MSR_GSBASE	0xc0000101
//#endif
//static __inline void
//load_gs(u_short sel)
//{
//	/*
//	 * Preserve the gsbase value across the selector load.
//	 * Note that we have to disable interrupts because the gsbase
//	 * being trashed happens to be the kernel gsbase at the time.
//	 */
//	__asm __volatile("pushfq; cli; rdmsr; movw %0,%%gs; wrmsr; popfq"
//	    : : "rm" (sel), "c" (MSR_GSBASE) : "eax", "edx");
//}
//#else
///* Usable by userland */
//static __inline void
//load_fs(u_short sel)
//{
//	__asm __volatile("movw %0,%%fs" : : "rm" (sel));
//}
//
//static __inline void
//load_gs(u_short sel)
//{
//	__asm __volatile("movw %0,%%gs" : : "rm" (sel));
//}
//#endif
//
//static __inline void
//lidt(struct region_descriptor *addr)
//{
//	__asm __volatile("lidt (%0)" : : "r" (addr));
//}
//
//static __inline void
//lldt(u_short sel)
//{
//	__asm __volatile("lldt %0" : : "r" (sel));
//}
//
//static __inline void
//ltr(u_short sel)
//{
//	__asm __volatile("ltr %0" : : "r" (sel));
//}
//
//static __inline uint64_t
//rdr0(void)
//{
//	uint64_t data;
//	__asm __volatile("movq %%dr0,%0" : "=r" (data));
//	return (data);
//}
//
//static __inline void
//load_dr0(uint64_t dr0)
//{
//	__asm __volatile("movq %0,%%dr0" : : "r" (dr0));
//}
//
//static __inline uint64_t
//rdr1(void)
//{
//	uint64_t data;
//	__asm __volatile("movq %%dr1,%0" : "=r" (data));
//	return (data);
//}
//
//static __inline void
//load_dr1(uint64_t dr1)
//{
//	__asm __volatile("movq %0,%%dr1" : : "r" (dr1));
//}
//
//static __inline uint64_t
//rdr2(void)
//{
//	uint64_t data;
//	__asm __volatile("movq %%dr2,%0" : "=r" (data));
//	return (data);
//}
//
//static __inline void
//load_dr2(uint64_t dr2)
//{
//	__asm __volatile("movq %0,%%dr2" : : "r" (dr2));
//}
//
//static __inline uint64_t
//rdr3(void)
//{
//	uint64_t data;
//	__asm __volatile("movq %%dr3,%0" : "=r" (data));
//	return (data);
//}
//
//static __inline void
//load_dr3(uint64_t dr3)
//{
//	__asm __volatile("movq %0,%%dr3" : : "r" (dr3));
//}
//
//static __inline uint64_t
//rdr4(void)
//{
//	uint64_t data;
//	__asm __volatile("movq %%dr4,%0" : "=r" (data));
//	return (data);
//}
//
//static __inline void
//load_dr4(uint64_t dr4)
//{
//	__asm __volatile("movq %0,%%dr4" : : "r" (dr4));
//}
//
//static __inline uint64_t
//rdr5(void)
//{
//	uint64_t data;
//	__asm __volatile("movq %%dr5,%0" : "=r" (data));
//	return (data);
//}
//
//static __inline void
//load_dr5(uint64_t dr5)
//{
//	__asm __volatile("movq %0,%%dr5" : : "r" (dr5));
//}
//
//static __inline uint64_t
//rdr6(void)
//{
//	uint64_t data;
//	__asm __volatile("movq %%dr6,%0" : "=r" (data));
//	return (data);
//}
//
//static __inline void
//load_dr6(uint64_t dr6)
//{
//	__asm __volatile("movq %0,%%dr6" : : "r" (dr6));
//}
//
//static __inline uint64_t
//rdr7(void)
//{
//	uint64_t data;
//	__asm __volatile("movq %%dr7,%0" : "=r" (data));
//	return (data);
//}
//
//static __inline void
//load_dr7(uint64_t dr7)
//{
//	__asm __volatile("movq %0,%%dr7" : : "r" (dr7));
//}
//
//static __inline register_t
//intr_disable(void)
//{
//	register_t rflags;
//
//	rflags = read_rflags();
//	disable_intr();
//	return (rflags);
//}
//
//static __inline void
//intr_restore(register_t rflags)
//{
//	write_rflags(rflags);
//}
//
//#else /* !(__GNUCLIKE_ASM && __CC_SUPPORTS___INLINE) */
//
//int	breakpoint(void);
//u_int	bsfl(u_int mask);
//u_int	bsrl(u_int mask);
//void	disable_intr(void);
//void	do_cpuid(u_int ax, u_int *p);
//void	enable_intr(void);
//void	halt(void);
//void	ia32_pause(void);
//u_char	inb(u_int port);
//u_int	inl(u_int port);
//void	insb(u_int port, void *addr, size_t count);
//void	insl(u_int port, void *addr, size_t count);
//void	insw(u_int port, void *addr, size_t count);
//register_t	intr_disable(void);
//void	intr_restore(register_t rf);
//void	invd(void);
//void	invlpg(u_int addr);
//void	invltlb(void);
//u_short	inw(u_int port);
//void	lidt(struct region_descriptor *addr);
//void	lldt(u_short sel);
//void	load_cr0(u_long cr0);
//void	load_cr3(u_long cr3);
//void	load_cr4(u_long cr4);
//void	load_dr0(uint64_t dr0);
//void	load_dr1(uint64_t dr1);
//void	load_dr2(uint64_t dr2);
//void	load_dr3(uint64_t dr3);
//void	load_dr4(uint64_t dr4);
//void	load_dr5(uint64_t dr5);
//void	load_dr6(uint64_t dr6);
//void	load_dr7(uint64_t dr7);
//void	load_fs(u_short sel);
//void	load_gs(u_short sel);
//void	ltr(u_short sel);
//void	outb(u_int port, u_char data);
//void	outl(u_int port, u_int data);
//void	outsb(u_int port, const void *addr, size_t count);
//void	outsl(u_int port, const void *addr, size_t count);
//void	outsw(u_int port, const void *addr, size_t count);
//void	outw(u_int port, u_short data);
//u_long	rcr0(void);
//u_long	rcr2(void);
//u_long	rcr3(void);
//u_long	rcr4(void);
//uint64_t rdmsr(u_int msr);
//uint64_t rdpmc(u_int pmc);
//uint64_t rdr0(void);
//uint64_t rdr1(void);
//uint64_t rdr2(void);
//uint64_t rdr3(void);
//uint64_t rdr4(void);
//uint64_t rdr5(void);
//uint64_t rdr6(void);
//uint64_t rdr7(void);
//uint64_t rdtsc(void);
//u_int	read_rflags(void);
//u_int	rfs(void);
//u_int	rgs(void);
//void	wbinvd(void);
//void	write_rflags(u_int rf);
//void	wrmsr(u_int msr, uint64_t newval);
//
//#endif	/* __GNUCLIKE_ASM && __CC_SUPPORTS___INLINE */
//
//void	reset_dbregs(void);
//
//#ifdef _KERNEL
//int	rdmsr_safe(u_int msr, uint64_t *val);
//int	wrmsr_safe(u_int msr, uint64_t newval);
//#endif
//
//#endif /* !_MACHINE_CPUFUNC_H_ */
