/* Generated from /usr/src/kerberos5/lib/libkrb5/../../../crypto/heimdal/lib/krb5/k524_err.et */
/* $Id: k524_err.et 10141 2001-06-20 02:45:58Z joda $ */

#ifndef __k524_err_h__
#define __k524_err_h__

struct et_list;

void initialize_k524_error_table_r(struct et_list **);

void initialize_k524_error_table(void);
#define init_k524_err_tbl initialize_k524_error_table

typedef enum k524_error_number{
	KRB524_BADKEY = -1750206208,
	KRB524_BADADDR = -1750206207,
	KRB524_BADPRINC = -1750206206,
	KRB524_BADREALM = -1750206205,
	KRB524_V4ERR = -1750206204,
	KRB524_ENCFULL = -1750206203,
	KRB524_DECEMPTY = -1750206202,
	KRB524_NOTRESP = -1750206201
} k524_error_number;

#define ERROR_TABLE_BASE_k524 -1750206208

#endif /* __k524_err_h__ */
