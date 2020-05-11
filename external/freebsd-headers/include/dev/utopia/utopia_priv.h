/*-
 * Copyright (c) 2005
 *	Hartmut Brandt.
 * 	All rights reserved.
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
 * Author: Hartmut Brandt <harti@freebsd.org>
 *
 * $FreeBSD: release/9.0.0/sys/dev/utopia/utopia_priv.h 142384 2005-02-24 16:56:36Z harti $
 *
 * Private include file for the interface between chip files and
 * the utopia main stuff.
 */

#ifndef _DEV_UTOPIA_UTOPIA_PRIV_H
#define	_DEV_UTOPIA_UTOPIA_PRIV_H

#define UTP_READREGS(UTOPIA, REG, VALP, NP)				\
    (UTOPIA)->methods->readregs((UTOPIA)->ifatm, REG, VALP, NP)
#define UTP_WRITEREG(UTOPIA, REG, MASK, VAL)			\
    (UTOPIA)->methods->writereg((UTOPIA)->ifatm, REG, MASK, VAL)

uint32_t utopia_update(struct utopia *, u_int, u_int, uint32_t);
void utopia_check_carrier(struct utopia *, u_int);

#endif	/* _DEV_UTOPIA_UTOPIA_PRIV_H */
