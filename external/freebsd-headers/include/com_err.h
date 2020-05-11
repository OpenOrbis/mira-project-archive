/*
 * Copyright (c) 1997 - 2001 Kungliga Tekniska H�gskolan
 * (Royal Institute of Technology, Stockholm, Sweden). 
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 *
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 *
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 */

/* $FreeBSD: release/9.0.0/contrib/com_err/com_err.h 178846 2008-05-08 11:01:46Z dfr $ */
/* $Id: com_err.h 15566 2005-07-07 14:58:07Z lha $ */

/* MIT compatible com_err library */

#ifndef __COM_ERR_H__
#define __COM_ERR_H__

#include <sys/cdefs.h>
#include <stdarg.h>

#include <com_right.h>
#include <stdarg.h>

typedef void (*errf) __P((const char *, long, const char *, va_list));

const char * error_message (long);
int init_error_table (const char**, long, int);

void com_err_va __P((const char *, long, const char *, va_list))
    __printflike(3, 0);

void com_err __P((const char *, long, const char *, ...))
    __printflike(3, 4);

errf set_com_err_hook (errf);
errf reset_com_err_hook (void);

const char *error_table_name  (int num);

void add_to_error_table (struct et_list *new_table);

#endif /* __COM_ERR_H__ */
