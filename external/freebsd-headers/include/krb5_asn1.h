/* Generated from /usr/src/kerberos5/lib/libasn1/../../../crypto/heimdal/lib/asn1/k5.asn1 */
/* Do not edit */

#ifndef __krb5_asn1_h__
#define __krb5_asn1_h__

#include <stddef.h>
#include <time.h>

#ifndef __asn1_common_definitions__
#define __asn1_common_definitions__

typedef struct heim_integer {
  size_t length;
  void *data;
  int negative;
} heim_integer;

typedef struct heim_octet_string {
  size_t length;
  void *data;
} heim_octet_string;

typedef char *heim_general_string;

typedef char *heim_utf8_string;

typedef char *heim_printable_string;

typedef char *heim_ia5_string;

typedef struct heim_bmp_string {
  size_t length;
  uint16_t *data;
} heim_bmp_string;

typedef struct heim_universal_string {
  size_t length;
  uint32_t *data;
} heim_universal_string;

typedef char *heim_visible_string;

typedef struct heim_oid {
  size_t length;
  unsigned *components;
} heim_oid;

typedef struct heim_bit_string {
  size_t length;
  void *data;
} heim_bit_string;

typedef struct heim_octet_string heim_any;
typedef struct heim_octet_string heim_any_set;

#define ASN1_MALLOC_ENCODE(T, B, BL, S, L, R)                  \
  do {                                                         \
    (BL) = length_##T((S));                                    \
    (B) = malloc((BL));                                        \
    if((B) == NULL) {                                          \
      (R) = ENOMEM;                                            \
    } else {                                                   \
      (R) = encode_##T(((unsigned char*)(B)) + (BL) - 1, (BL), \
                       (S), (L));                              \
      if((R) != 0) {                                           \
        free((B));                                             \
        (B) = NULL;                                            \
      }                                                        \
    }                                                          \
  } while (0)

struct units;

#endif

/*
NAME-TYPE ::= INTEGER {
  KRB5_NT_UNKNOWN(0),
  KRB5_NT_PRINCIPAL(1),
  KRB5_NT_SRV_INST(2),
  KRB5_NT_SRV_HST(3),
  KRB5_NT_SRV_XHST(4),
  KRB5_NT_UID(5),
  KRB5_NT_X500_PRINCIPAL(6),
  KRB5_NT_SMTP_NAME(7),
  KRB5_NT_ENTERPRISE_PRINCIPAL(10),
  KRB5_NT_ENT_PRINCIPAL_AND_ID(-130),
  KRB5_NT_MS_PRINCIPAL(-128),
  KRB5_NT_MS_PRINCIPAL_AND_ID(-129)
}
*/

typedef enum NAME_TYPE {
  KRB5_NT_UNKNOWN = 0,
  KRB5_NT_PRINCIPAL = 1,
  KRB5_NT_SRV_INST = 2,
  KRB5_NT_SRV_HST = 3,
  KRB5_NT_SRV_XHST = 4,
  KRB5_NT_UID = 5,
  KRB5_NT_X500_PRINCIPAL = 6,
  KRB5_NT_SMTP_NAME = 7,
  KRB5_NT_ENTERPRISE_PRINCIPAL = 10,
  KRB5_NT_ENT_PRINCIPAL_AND_ID = -130,
  KRB5_NT_MS_PRINCIPAL = -128,
  KRB5_NT_MS_PRINCIPAL_AND_ID = -129
} NAME_TYPE;

int    encode_NAME_TYPE(unsigned char *, size_t, const NAME_TYPE *, size_t *);
int    decode_NAME_TYPE(const unsigned char *, size_t, NAME_TYPE *, size_t *);
void   free_NAME_TYPE  (NAME_TYPE *);
size_t length_NAME_TYPE(const NAME_TYPE *);
int    copy_NAME_TYPE  (const NAME_TYPE *, NAME_TYPE *);


/*
MESSAGE-TYPE ::= INTEGER {
  krb_as_req(10),
  krb_as_rep(11),
  krb_tgs_req(12),
  krb_tgs_rep(13),
  krb_ap_req(14),
  krb_ap_rep(15),
  krb_safe(20),
  krb_priv(21),
  krb_cred(22),
  krb_error(30)
}
*/

typedef enum MESSAGE_TYPE {
  krb_as_req = 10,
  krb_as_rep = 11,
  krb_tgs_req = 12,
  krb_tgs_rep = 13,
  krb_ap_req = 14,
  krb_ap_rep = 15,
  krb_safe = 20,
  krb_priv = 21,
  krb_cred = 22,
  krb_error = 30
} MESSAGE_TYPE;

int    encode_MESSAGE_TYPE(unsigned char *, size_t, const MESSAGE_TYPE *, size_t *);
int    decode_MESSAGE_TYPE(const unsigned char *, size_t, MESSAGE_TYPE *, size_t *);
void   free_MESSAGE_TYPE  (MESSAGE_TYPE *);
size_t length_MESSAGE_TYPE(const MESSAGE_TYPE *);
int    copy_MESSAGE_TYPE  (const MESSAGE_TYPE *, MESSAGE_TYPE *);


/*
PADATA-TYPE ::= INTEGER {
  KRB5_PADATA_NONE(0),
  KRB5_PADATA_TGS_REQ(1),
  KRB5_PADATA_AP_REQ(1),
  KRB5_PADATA_ENC_TIMESTAMP(2),
  KRB5_PADATA_PW_SALT(3),
  KRB5_PADATA_ENC_UNIX_TIME(5),
  KRB5_PADATA_SANDIA_SECUREID(6),
  KRB5_PADATA_SESAME(7),
  KRB5_PADATA_OSF_DCE(8),
  KRB5_PADATA_CYBERSAFE_SECUREID(9),
  KRB5_PADATA_AFS3_SALT(10),
  KRB5_PADATA_ETYPE_INFO(11),
  KRB5_PADATA_SAM_CHALLENGE(12),
  KRB5_PADATA_SAM_RESPONSE(13),
  KRB5_PADATA_PK_AS_REQ_19(14),
  KRB5_PADATA_PK_AS_REP_19(15),
  KRB5_PADATA_PK_AS_REQ_WIN(15),
  KRB5_PADATA_PK_AS_REQ(16),
  KRB5_PADATA_PK_AS_REP(17),
  KRB5_PADATA_PA_PK_OCSP_RESPONSE(18),
  KRB5_PADATA_ETYPE_INFO2(19),
  KRB5_PADATA_USE_SPECIFIED_KVNO(20),
  KRB5_PADATA_SVR_REFERRAL_INFO(20),
  KRB5_PADATA_SAM_REDIRECT(21),
  KRB5_PADATA_GET_FROM_TYPED_DATA(22),
  KRB5_PADATA_SAM_ETYPE_INFO(23),
  KRB5_PADATA_SERVER_REFERRAL(25),
  KRB5_PADATA_TD_KRB_PRINCIPAL(102),
  KRB5_PADATA_PK_TD_TRUSTED_CERTIFIERS(104),
  KRB5_PADATA_PK_TD_CERTIFICATE_INDEX(105),
  KRB5_PADATA_TD_APP_DEFINED_ERROR(106),
  KRB5_PADATA_TD_REQ_NONCE(107),
  KRB5_PADATA_TD_REQ_SEQ(108),
  KRB5_PADATA_PA_PAC_REQUEST(128),
  KRB5_PADATA_S4U2SELF(129),
  KRB5_PADATA_PK_AS_09_BINDING(132),
  KRB5_PADATA_CLIENT_CANONICALIZED(133)
}
*/

typedef enum PADATA_TYPE {
  KRB5_PADATA_NONE = 0,
  KRB5_PADATA_TGS_REQ = 1,
  KRB5_PADATA_AP_REQ = 1,
  KRB5_PADATA_ENC_TIMESTAMP = 2,
  KRB5_PADATA_PW_SALT = 3,
  KRB5_PADATA_ENC_UNIX_TIME = 5,
  KRB5_PADATA_SANDIA_SECUREID = 6,
  KRB5_PADATA_SESAME = 7,
  KRB5_PADATA_OSF_DCE = 8,
  KRB5_PADATA_CYBERSAFE_SECUREID = 9,
  KRB5_PADATA_AFS3_SALT = 10,
  KRB5_PADATA_ETYPE_INFO = 11,
  KRB5_PADATA_SAM_CHALLENGE = 12,
  KRB5_PADATA_SAM_RESPONSE = 13,
  KRB5_PADATA_PK_AS_REQ_19 = 14,
  KRB5_PADATA_PK_AS_REP_19 = 15,
  KRB5_PADATA_PK_AS_REQ_WIN = 15,
  KRB5_PADATA_PK_AS_REQ = 16,
  KRB5_PADATA_PK_AS_REP = 17,
  KRB5_PADATA_PA_PK_OCSP_RESPONSE = 18,
  KRB5_PADATA_ETYPE_INFO2 = 19,
  KRB5_PADATA_USE_SPECIFIED_KVNO = 20,
  KRB5_PADATA_SVR_REFERRAL_INFO = 20,
  KRB5_PADATA_SAM_REDIRECT = 21,
  KRB5_PADATA_GET_FROM_TYPED_DATA = 22,
  KRB5_PADATA_SAM_ETYPE_INFO = 23,
  KRB5_PADATA_SERVER_REFERRAL = 25,
  KRB5_PADATA_TD_KRB_PRINCIPAL = 102,
  KRB5_PADATA_PK_TD_TRUSTED_CERTIFIERS = 104,
  KRB5_PADATA_PK_TD_CERTIFICATE_INDEX = 105,
  KRB5_PADATA_TD_APP_DEFINED_ERROR = 106,
  KRB5_PADATA_TD_REQ_NONCE = 107,
  KRB5_PADATA_TD_REQ_SEQ = 108,
  KRB5_PADATA_PA_PAC_REQUEST = 128,
  KRB5_PADATA_S4U2SELF = 129,
  KRB5_PADATA_PK_AS_09_BINDING = 132,
  KRB5_PADATA_CLIENT_CANONICALIZED = 133
} PADATA_TYPE;

int    encode_PADATA_TYPE(unsigned char *, size_t, const PADATA_TYPE *, size_t *);
int    decode_PADATA_TYPE(const unsigned char *, size_t, PADATA_TYPE *, size_t *);
void   free_PADATA_TYPE  (PADATA_TYPE *);
size_t length_PADATA_TYPE(const PADATA_TYPE *);
int    copy_PADATA_TYPE  (const PADATA_TYPE *, PADATA_TYPE *);


/*
AUTHDATA-TYPE ::= INTEGER {
  KRB5_AUTHDATA_IF_RELEVANT(1),
  KRB5_AUTHDATA_INTENDED_FOR_SERVER(2),
  KRB5_AUTHDATA_INTENDED_FOR_APPLICATION_CLASS(3),
  KRB5_AUTHDATA_KDC_ISSUED(4),
  KRB5_AUTHDATA_AND_OR(5),
  KRB5_AUTHDATA_MANDATORY_TICKET_EXTENSIONS(6),
  KRB5_AUTHDATA_IN_TICKET_EXTENSIONS(7),
  KRB5_AUTHDATA_MANDATORY_FOR_KDC(8),
  KRB5_AUTHDATA_INITIAL_VERIFIED_CAS(9),
  KRB5_AUTHDATA_OSF_DCE(64),
  KRB5_AUTHDATA_SESAME(65),
  KRB5_AUTHDATA_OSF_DCE_PKI_CERTID(66),
  KRB5_AUTHDATA_WIN2K_PAC(128),
  KRB5_AUTHDATA_GSS_API_ETYPE_NEGOTIATION(129),
  KRB5_AUTHDATA_SIGNTICKET(-17)
}
*/

typedef enum AUTHDATA_TYPE {
  KRB5_AUTHDATA_IF_RELEVANT = 1,
  KRB5_AUTHDATA_INTENDED_FOR_SERVER = 2,
  KRB5_AUTHDATA_INTENDED_FOR_APPLICATION_CLASS = 3,
  KRB5_AUTHDATA_KDC_ISSUED = 4,
  KRB5_AUTHDATA_AND_OR = 5,
  KRB5_AUTHDATA_MANDATORY_TICKET_EXTENSIONS = 6,
  KRB5_AUTHDATA_IN_TICKET_EXTENSIONS = 7,
  KRB5_AUTHDATA_MANDATORY_FOR_KDC = 8,
  KRB5_AUTHDATA_INITIAL_VERIFIED_CAS = 9,
  KRB5_AUTHDATA_OSF_DCE = 64,
  KRB5_AUTHDATA_SESAME = 65,
  KRB5_AUTHDATA_OSF_DCE_PKI_CERTID = 66,
  KRB5_AUTHDATA_WIN2K_PAC = 128,
  KRB5_AUTHDATA_GSS_API_ETYPE_NEGOTIATION = 129,
  KRB5_AUTHDATA_SIGNTICKET = -17
} AUTHDATA_TYPE;

int    encode_AUTHDATA_TYPE(unsigned char *, size_t, const AUTHDATA_TYPE *, size_t *);
int    decode_AUTHDATA_TYPE(const unsigned char *, size_t, AUTHDATA_TYPE *, size_t *);
void   free_AUTHDATA_TYPE  (AUTHDATA_TYPE *);
size_t length_AUTHDATA_TYPE(const AUTHDATA_TYPE *);
int    copy_AUTHDATA_TYPE  (const AUTHDATA_TYPE *, AUTHDATA_TYPE *);


/*
CKSUMTYPE ::= INTEGER {
  CKSUMTYPE_NONE(0),
  CKSUMTYPE_CRC32(1),
  CKSUMTYPE_RSA_MD4(2),
  CKSUMTYPE_RSA_MD4_DES(3),
  CKSUMTYPE_DES_MAC(4),
  CKSUMTYPE_DES_MAC_K(5),
  CKSUMTYPE_RSA_MD4_DES_K(6),
  CKSUMTYPE_RSA_MD5(7),
  CKSUMTYPE_RSA_MD5_DES(8),
  CKSUMTYPE_RSA_MD5_DES3(9),
  CKSUMTYPE_SHA1_OTHER(10),
  CKSUMTYPE_HMAC_SHA1_DES3(12),
  CKSUMTYPE_SHA1(14),
  CKSUMTYPE_HMAC_SHA1_96_AES_128(15),
  CKSUMTYPE_HMAC_SHA1_96_AES_256(16),
  CKSUMTYPE_GSSAPI(32771),
  CKSUMTYPE_HMAC_MD5(-138),
  CKSUMTYPE_HMAC_MD5_ENC(-1138)
}
*/

typedef enum CKSUMTYPE {
  CKSUMTYPE_NONE = 0,
  CKSUMTYPE_CRC32 = 1,
  CKSUMTYPE_RSA_MD4 = 2,
  CKSUMTYPE_RSA_MD4_DES = 3,
  CKSUMTYPE_DES_MAC = 4,
  CKSUMTYPE_DES_MAC_K = 5,
  CKSUMTYPE_RSA_MD4_DES_K = 6,
  CKSUMTYPE_RSA_MD5 = 7,
  CKSUMTYPE_RSA_MD5_DES = 8,
  CKSUMTYPE_RSA_MD5_DES3 = 9,
  CKSUMTYPE_SHA1_OTHER = 10,
  CKSUMTYPE_HMAC_SHA1_DES3 = 12,
  CKSUMTYPE_SHA1 = 14,
  CKSUMTYPE_HMAC_SHA1_96_AES_128 = 15,
  CKSUMTYPE_HMAC_SHA1_96_AES_256 = 16,
  CKSUMTYPE_GSSAPI = 32771,
  CKSUMTYPE_HMAC_MD5 = -138,
  CKSUMTYPE_HMAC_MD5_ENC = -1138
} CKSUMTYPE;

int    encode_CKSUMTYPE(unsigned char *, size_t, const CKSUMTYPE *, size_t *);
int    decode_CKSUMTYPE(const unsigned char *, size_t, CKSUMTYPE *, size_t *);
void   free_CKSUMTYPE  (CKSUMTYPE *);
size_t length_CKSUMTYPE(const CKSUMTYPE *);
int    copy_CKSUMTYPE  (const CKSUMTYPE *, CKSUMTYPE *);


/*
ENCTYPE ::= INTEGER {
  ETYPE_NULL(0),
  ETYPE_DES_CBC_CRC(1),
  ETYPE_DES_CBC_MD4(2),
  ETYPE_DES_CBC_MD5(3),
  ETYPE_DES3_CBC_MD5(5),
  ETYPE_OLD_DES3_CBC_SHA1(7),
  ETYPE_SIGN_DSA_GENERATE(8),
  ETYPE_ENCRYPT_RSA_PRIV(9),
  ETYPE_ENCRYPT_RSA_PUB(10),
  ETYPE_DES3_CBC_SHA1(16),
  ETYPE_AES128_CTS_HMAC_SHA1_96(17),
  ETYPE_AES256_CTS_HMAC_SHA1_96(18),
  ETYPE_ARCFOUR_HMAC_MD5(23),
  ETYPE_ARCFOUR_HMAC_MD5_56(24),
  ETYPE_ENCTYPE_PK_CROSS(48),
  ETYPE_ARCFOUR_MD4(-128),
  ETYPE_ARCFOUR_HMAC_OLD(-133),
  ETYPE_ARCFOUR_HMAC_OLD_EXP(-135),
  ETYPE_DES_CBC_NONE(-4096),
  ETYPE_DES3_CBC_NONE(-4097),
  ETYPE_DES_CFB64_NONE(-4098),
  ETYPE_DES_PCBC_NONE(-4099),
  ETYPE_DIGEST_MD5_NONE(-4100),
  ETYPE_CRAM_MD5_NONE(-4101)
}
*/

typedef enum ENCTYPE {
  ETYPE_NULL = 0,
  ETYPE_DES_CBC_CRC = 1,
  ETYPE_DES_CBC_MD4 = 2,
  ETYPE_DES_CBC_MD5 = 3,
  ETYPE_DES3_CBC_MD5 = 5,
  ETYPE_OLD_DES3_CBC_SHA1 = 7,
  ETYPE_SIGN_DSA_GENERATE = 8,
  ETYPE_ENCRYPT_RSA_PRIV = 9,
  ETYPE_ENCRYPT_RSA_PUB = 10,
  ETYPE_DES3_CBC_SHA1 = 16,
  ETYPE_AES128_CTS_HMAC_SHA1_96 = 17,
  ETYPE_AES256_CTS_HMAC_SHA1_96 = 18,
  ETYPE_ARCFOUR_HMAC_MD5 = 23,
  ETYPE_ARCFOUR_HMAC_MD5_56 = 24,
  ETYPE_ENCTYPE_PK_CROSS = 48,
  ETYPE_ARCFOUR_MD4 = -128,
  ETYPE_ARCFOUR_HMAC_OLD = -133,
  ETYPE_ARCFOUR_HMAC_OLD_EXP = -135,
  ETYPE_DES_CBC_NONE = -4096,
  ETYPE_DES3_CBC_NONE = -4097,
  ETYPE_DES_CFB64_NONE = -4098,
  ETYPE_DES_PCBC_NONE = -4099,
  ETYPE_DIGEST_MD5_NONE = -4100,
  ETYPE_CRAM_MD5_NONE = -4101
} ENCTYPE;

int    encode_ENCTYPE(unsigned char *, size_t, const ENCTYPE *, size_t *);
int    decode_ENCTYPE(const unsigned char *, size_t, ENCTYPE *, size_t *);
void   free_ENCTYPE  (ENCTYPE *);
size_t length_ENCTYPE(const ENCTYPE *);
int    copy_ENCTYPE  (const ENCTYPE *, ENCTYPE *);


/*
krb5uint32 ::= INTEGER (0..-1)
*/

typedef unsigned int krb5uint32;

int    encode_krb5uint32(unsigned char *, size_t, const krb5uint32 *, size_t *);
int    decode_krb5uint32(const unsigned char *, size_t, krb5uint32 *, size_t *);
void   free_krb5uint32  (krb5uint32 *);
size_t length_krb5uint32(const krb5uint32 *);
int    copy_krb5uint32  (const krb5uint32 *, krb5uint32 *);


/*
krb5int32 ::= INTEGER (-2147483648..2147483647)
*/

typedef int krb5int32;

int    encode_krb5int32(unsigned char *, size_t, const krb5int32 *, size_t *);
int    decode_krb5int32(const unsigned char *, size_t, krb5int32 *, size_t *);
void   free_krb5int32  (krb5int32 *);
size_t length_krb5int32(const krb5int32 *);
int    copy_krb5int32  (const krb5int32 *, krb5int32 *);


/*
KerberosString ::= GeneralString
*/

typedef heim_general_string KerberosString;

int    encode_KerberosString(unsigned char *, size_t, const KerberosString *, size_t *);
int    decode_KerberosString(const unsigned char *, size_t, KerberosString *, size_t *);
void   free_KerberosString  (KerberosString *);
size_t length_KerberosString(const KerberosString *);
int    copy_KerberosString  (const KerberosString *, KerberosString *);


/*
Realm ::= GeneralString
*/

typedef heim_general_string Realm;

int    encode_Realm(unsigned char *, size_t, const Realm *, size_t *);
int    decode_Realm(const unsigned char *, size_t, Realm *, size_t *);
void   free_Realm  (Realm *);
size_t length_Realm(const Realm *);
int    copy_Realm  (const Realm *, Realm *);


/*
PrincipalName ::= SEQUENCE {
  name-type       [0] NAME-TYPE,
  name-string     [1] SEQUENCE OF GeneralString,
}
*/

typedef struct PrincipalName {
  NAME_TYPE name_type;
  struct  {
    unsigned int len;
    heim_general_string *val;
  } name_string;
} PrincipalName;

int    encode_PrincipalName(unsigned char *, size_t, const PrincipalName *, size_t *);
int    decode_PrincipalName(const unsigned char *, size_t, PrincipalName *, size_t *);
void   free_PrincipalName  (PrincipalName *);
size_t length_PrincipalName(const PrincipalName *);
int    copy_PrincipalName  (const PrincipalName *, PrincipalName *);


/*
Principal ::= SEQUENCE {
  name            [0] PrincipalName,
  realm           [1] Realm,
}
*/

typedef struct Principal {
  PrincipalName name;
  Realm realm;
} Principal;

int    encode_Principal(unsigned char *, size_t, const Principal *, size_t *);
int    decode_Principal(const unsigned char *, size_t, Principal *, size_t *);
void   free_Principal  (Principal *);
size_t length_Principal(const Principal *);
int    copy_Principal  (const Principal *, Principal *);


/*
HostAddress ::= SEQUENCE {
  addr-type       [0] krb5int32,
  address         [1] OCTET STRING,
}
*/

typedef struct HostAddress {
  krb5int32 addr_type;
  heim_octet_string address;
} HostAddress;

int    encode_HostAddress(unsigned char *, size_t, const HostAddress *, size_t *);
int    decode_HostAddress(const unsigned char *, size_t, HostAddress *, size_t *);
void   free_HostAddress  (HostAddress *);
size_t length_HostAddress(const HostAddress *);
int    copy_HostAddress  (const HostAddress *, HostAddress *);


/*
HostAddresses ::= SEQUENCE OF HostAddress
*/

typedef struct HostAddresses {
  unsigned int len;
  HostAddress *val;
} HostAddresses;

int    encode_HostAddresses(unsigned char *, size_t, const HostAddresses *, size_t *);
int    decode_HostAddresses(const unsigned char *, size_t, HostAddresses *, size_t *);
void   free_HostAddresses  (HostAddresses *);
size_t length_HostAddresses(const HostAddresses *);
int    copy_HostAddresses  (const HostAddresses *, HostAddresses *);


/*
KerberosTime ::= GeneralizedTime
*/

typedef time_t KerberosTime;

int    encode_KerberosTime(unsigned char *, size_t, const KerberosTime *, size_t *);
int    decode_KerberosTime(const unsigned char *, size_t, KerberosTime *, size_t *);
void   free_KerberosTime  (KerberosTime *);
size_t length_KerberosTime(const KerberosTime *);
int    copy_KerberosTime  (const KerberosTime *, KerberosTime *);


/*
AuthorizationDataElement ::= SEQUENCE {
  ad-type         [0] krb5int32,
  ad-data         [1] OCTET STRING,
}
*/

typedef struct AuthorizationDataElement {
  krb5int32 ad_type;
  heim_octet_string ad_data;
} AuthorizationDataElement;

int    encode_AuthorizationDataElement(unsigned char *, size_t, const AuthorizationDataElement *, size_t *);
int    decode_AuthorizationDataElement(const unsigned char *, size_t, AuthorizationDataElement *, size_t *);
void   free_AuthorizationDataElement  (AuthorizationDataElement *);
size_t length_AuthorizationDataElement(const AuthorizationDataElement *);
int    copy_AuthorizationDataElement  (const AuthorizationDataElement *, AuthorizationDataElement *);


/*
AuthorizationData ::= SEQUENCE OF AuthorizationDataElement
*/

typedef struct AuthorizationData {
  unsigned int len;
  AuthorizationDataElement *val;
} AuthorizationData;

int    encode_AuthorizationData(unsigned char *, size_t, const AuthorizationData *, size_t *);
int    decode_AuthorizationData(const unsigned char *, size_t, AuthorizationData *, size_t *);
void   free_AuthorizationData  (AuthorizationData *);
size_t length_AuthorizationData(const AuthorizationData *);
int    copy_AuthorizationData  (const AuthorizationData *, AuthorizationData *);
int   add_AuthorizationData  (AuthorizationData *, const AuthorizationDataElement *);
int   remove_AuthorizationData  (AuthorizationData *, unsigned int);


/*
APOptions ::= BIT STRING {
  reserved(0),
  use-session-key(1),
  mutual-required(2)
}
*/

typedef struct APOptions {
  unsigned int reserved:1;
  unsigned int use_session_key:1;
  unsigned int mutual_required:1;
} APOptions;


int    encode_APOptions(unsigned char *, size_t, const APOptions *, size_t *);
int    decode_APOptions(const unsigned char *, size_t, APOptions *, size_t *);
void   free_APOptions  (APOptions *);
size_t length_APOptions(const APOptions *);
int    copy_APOptions  (const APOptions *, APOptions *);
unsigned APOptions2int(APOptions);
APOptions int2APOptions(unsigned);
#ifdef __PARSE_UNITS_H__
const struct units * asn1_APOptions_units(void);
#endif


/*
TicketFlags ::= BIT STRING {
  reserved(0),
  forwardable(1),
  forwarded(2),
  proxiable(3),
  proxy(4),
  may-postdate(5),
  postdated(6),
  invalid(7),
  renewable(8),
  initial(9),
  pre-authent(10),
  hw-authent(11),
  transited-policy-checked(12),
  ok-as-delegate(13),
  anonymous(14)
}
*/

typedef struct TicketFlags {
  unsigned int reserved:1;
  unsigned int forwardable:1;
  unsigned int forwarded:1;
  unsigned int proxiable:1;
  unsigned int proxy:1;
  unsigned int may_postdate:1;
  unsigned int postdated:1;
  unsigned int invalid:1;
  unsigned int renewable:1;
  unsigned int initial:1;
  unsigned int pre_authent:1;
  unsigned int hw_authent:1;
  unsigned int transited_policy_checked:1;
  unsigned int ok_as_delegate:1;
  unsigned int anonymous:1;
} TicketFlags;


int    encode_TicketFlags(unsigned char *, size_t, const TicketFlags *, size_t *);
int    decode_TicketFlags(const unsigned char *, size_t, TicketFlags *, size_t *);
void   free_TicketFlags  (TicketFlags *);
size_t length_TicketFlags(const TicketFlags *);
int    copy_TicketFlags  (const TicketFlags *, TicketFlags *);
unsigned TicketFlags2int(TicketFlags);
TicketFlags int2TicketFlags(unsigned);
#ifdef __PARSE_UNITS_H__
const struct units * asn1_TicketFlags_units(void);
#endif


/*
KDCOptions ::= BIT STRING {
  reserved(0),
  forwardable(1),
  forwarded(2),
  proxiable(3),
  proxy(4),
  allow-postdate(5),
  postdated(6),
  unused7(7),
  renewable(8),
  unused9(9),
  unused10(10),
  unused11(11),
  request-anonymous(14),
  canonicalize(15),
  constrained-delegation(16),
  disable-transited-check(26),
  renewable-ok(27),
  enc-tkt-in-skey(28),
  renew(30),
  validate(31)
}
*/

typedef struct KDCOptions {
  unsigned int reserved:1;
  unsigned int forwardable:1;
  unsigned int forwarded:1;
  unsigned int proxiable:1;
  unsigned int proxy:1;
  unsigned int allow_postdate:1;
  unsigned int postdated:1;
  unsigned int unused7:1;
  unsigned int renewable:1;
  unsigned int unused9:1;
  unsigned int unused10:1;
  unsigned int unused11:1;
  unsigned int request_anonymous:1;
  unsigned int canonicalize:1;
  unsigned int constrained_delegation:1;
  unsigned int disable_transited_check:1;
  unsigned int renewable_ok:1;
  unsigned int enc_tkt_in_skey:1;
  unsigned int renew:1;
  unsigned int validate:1;
} KDCOptions;


int    encode_KDCOptions(unsigned char *, size_t, const KDCOptions *, size_t *);
int    decode_KDCOptions(const unsigned char *, size_t, KDCOptions *, size_t *);
void   free_KDCOptions  (KDCOptions *);
size_t length_KDCOptions(const KDCOptions *);
int    copy_KDCOptions  (const KDCOptions *, KDCOptions *);
unsigned KDCOptions2int(KDCOptions);
KDCOptions int2KDCOptions(unsigned);
#ifdef __PARSE_UNITS_H__
const struct units * asn1_KDCOptions_units(void);
#endif


/*
LR-TYPE ::= INTEGER {
  LR_NONE(0),
  LR_INITIAL_TGT(1),
  LR_INITIAL(2),
  LR_ISSUE_USE_TGT(3),
  LR_RENEWAL(4),
  LR_REQUEST(5),
  LR_PW_EXPTIME(6),
  LR_ACCT_EXPTIME(7)
}
*/

typedef enum LR_TYPE {
  LR_NONE = 0,
  LR_INITIAL_TGT = 1,
  LR_INITIAL = 2,
  LR_ISSUE_USE_TGT = 3,
  LR_RENEWAL = 4,
  LR_REQUEST = 5,
  LR_PW_EXPTIME = 6,
  LR_ACCT_EXPTIME = 7
} LR_TYPE;

int    encode_LR_TYPE(unsigned char *, size_t, const LR_TYPE *, size_t *);
int    decode_LR_TYPE(const unsigned char *, size_t, LR_TYPE *, size_t *);
void   free_LR_TYPE  (LR_TYPE *);
size_t length_LR_TYPE(const LR_TYPE *);
int    copy_LR_TYPE  (const LR_TYPE *, LR_TYPE *);


/*
LastReq ::= SEQUENCE OF SEQUENCE {
  lr-type         [0] LR-TYPE,
  lr-value        [1] KerberosTime,
}
*/

typedef struct LastReq {
  unsigned int len;
  struct  {
    LR_TYPE lr_type;
    KerberosTime lr_value;
  } *val;
} LastReq;

int    encode_LastReq(unsigned char *, size_t, const LastReq *, size_t *);
int    decode_LastReq(const unsigned char *, size_t, LastReq *, size_t *);
void   free_LastReq  (LastReq *);
size_t length_LastReq(const LastReq *);
int    copy_LastReq  (const LastReq *, LastReq *);


/*
EncryptedData ::= SEQUENCE {
  etype           [0] ENCTYPE,
  kvno            [1] krb5int32 OPTIONAL,
  cipher          [2] OCTET STRING,
}
*/

typedef struct EncryptedData {
  ENCTYPE etype;
  krb5int32 *kvno;
  heim_octet_string cipher;
} EncryptedData;

int    encode_EncryptedData(unsigned char *, size_t, const EncryptedData *, size_t *);
int    decode_EncryptedData(const unsigned char *, size_t, EncryptedData *, size_t *);
void   free_EncryptedData  (EncryptedData *);
size_t length_EncryptedData(const EncryptedData *);
int    copy_EncryptedData  (const EncryptedData *, EncryptedData *);


/*
EncryptionKey ::= SEQUENCE {
  keytype         [0] krb5int32,
  keyvalue        [1] OCTET STRING,
}
*/

typedef struct EncryptionKey {
  krb5int32 keytype;
  heim_octet_string keyvalue;
} EncryptionKey;

int    encode_EncryptionKey(unsigned char *, size_t, const EncryptionKey *, size_t *);
int    decode_EncryptionKey(const unsigned char *, size_t, EncryptionKey *, size_t *);
void   free_EncryptionKey  (EncryptionKey *);
size_t length_EncryptionKey(const EncryptionKey *);
int    copy_EncryptionKey  (const EncryptionKey *, EncryptionKey *);


/*
TransitedEncoding ::= SEQUENCE {
  tr-type         [0] krb5int32,
  contents        [1] OCTET STRING,
}
*/

typedef struct TransitedEncoding {
  krb5int32 tr_type;
  heim_octet_string contents;
} TransitedEncoding;

int    encode_TransitedEncoding(unsigned char *, size_t, const TransitedEncoding *, size_t *);
int    decode_TransitedEncoding(const unsigned char *, size_t, TransitedEncoding *, size_t *);
void   free_TransitedEncoding  (TransitedEncoding *);
size_t length_TransitedEncoding(const TransitedEncoding *);
int    copy_TransitedEncoding  (const TransitedEncoding *, TransitedEncoding *);


/*
Ticket ::= [APPLICATION 1] SEQUENCE {
  tkt-vno         [0] krb5int32,
  realm           [1] Realm,
  sname           [2] PrincipalName,
  enc-part        [3] EncryptedData,
}
*/

typedef struct Ticket {
  krb5int32 tkt_vno;
  Realm realm;
  PrincipalName sname;
  EncryptedData enc_part;
} Ticket;

int    encode_Ticket(unsigned char *, size_t, const Ticket *, size_t *);
int    decode_Ticket(const unsigned char *, size_t, Ticket *, size_t *);
void   free_Ticket  (Ticket *);
size_t length_Ticket(const Ticket *);
int    copy_Ticket  (const Ticket *, Ticket *);


/*
EncTicketPart ::= [APPLICATION 3] SEQUENCE {
  flags                [0] TicketFlags,
  key                  [1] EncryptionKey,
  crealm               [2] Realm,
  cname                [3] PrincipalName,
  transited            [4] TransitedEncoding,
  authtime             [5] KerberosTime,
  starttime            [6] KerberosTime OPTIONAL,
  endtime              [7] KerberosTime,
  renew-till           [8] KerberosTime OPTIONAL,
  caddr                [9] HostAddresses OPTIONAL,
  authorization-data   [10] AuthorizationData OPTIONAL,
}
*/

typedef struct EncTicketPart {
  TicketFlags flags;
  EncryptionKey key;
  Realm crealm;
  PrincipalName cname;
  TransitedEncoding transited;
  KerberosTime authtime;
  KerberosTime *starttime;
  KerberosTime endtime;
  KerberosTime *renew_till;
  HostAddresses *caddr;
  AuthorizationData *authorization_data;
} EncTicketPart;

int    encode_EncTicketPart(unsigned char *, size_t, const EncTicketPart *, size_t *);
int    decode_EncTicketPart(const unsigned char *, size_t, EncTicketPart *, size_t *);
void   free_EncTicketPart  (EncTicketPart *);
size_t length_EncTicketPart(const EncTicketPart *);
int    copy_EncTicketPart  (const EncTicketPart *, EncTicketPart *);


/*
Checksum ::= SEQUENCE {
  cksumtype       [0] CKSUMTYPE,
  checksum        [1] OCTET STRING,
}
*/

typedef struct Checksum {
  CKSUMTYPE cksumtype;
  heim_octet_string checksum;
} Checksum;

int    encode_Checksum(unsigned char *, size_t, const Checksum *, size_t *);
int    decode_Checksum(const unsigned char *, size_t, Checksum *, size_t *);
void   free_Checksum  (Checksum *);
size_t length_Checksum(const Checksum *);
int    copy_Checksum  (const Checksum *, Checksum *);


/*
Authenticator ::= [APPLICATION 2] SEQUENCE {
  authenticator-vno    [0] krb5int32,
  crealm               [1] Realm,
  cname                [2] PrincipalName,
  cksum                [3] Checksum OPTIONAL,
  cusec                [4] krb5int32,
  ctime                [5] KerberosTime,
  subkey               [6] EncryptionKey OPTIONAL,
  seq-number           [7] krb5uint32 OPTIONAL,
  authorization-data   [8] AuthorizationData OPTIONAL,
}
*/

typedef struct Authenticator {
  krb5int32 authenticator_vno;
  Realm crealm;
  PrincipalName cname;
  Checksum *cksum;
  krb5int32 cusec;
  KerberosTime ctime;
  EncryptionKey *subkey;
  krb5uint32 *seq_number;
  AuthorizationData *authorization_data;
} Authenticator;

int    encode_Authenticator(unsigned char *, size_t, const Authenticator *, size_t *);
int    decode_Authenticator(const unsigned char *, size_t, Authenticator *, size_t *);
void   free_Authenticator  (Authenticator *);
size_t length_Authenticator(const Authenticator *);
int    copy_Authenticator  (const Authenticator *, Authenticator *);


/*
PA-DATA ::= SEQUENCE {
  padata-type     [1] PADATA-TYPE,
  padata-value    [2] OCTET STRING,
}
*/

typedef struct PA_DATA {
  PADATA_TYPE padata_type;
  heim_octet_string padata_value;
} PA_DATA;

int    encode_PA_DATA(unsigned char *, size_t, const PA_DATA *, size_t *);
int    decode_PA_DATA(const unsigned char *, size_t, PA_DATA *, size_t *);
void   free_PA_DATA  (PA_DATA *);
size_t length_PA_DATA(const PA_DATA *);
int    copy_PA_DATA  (const PA_DATA *, PA_DATA *);


/*
ETYPE-INFO-ENTRY ::= SEQUENCE {
  etype           [0] ENCTYPE,
  salt            [1] OCTET STRING OPTIONAL,
  salttype        [2] krb5int32 OPTIONAL,
}
*/

typedef struct ETYPE_INFO_ENTRY {
  ENCTYPE etype;
  heim_octet_string *salt;
  krb5int32 *salttype;
} ETYPE_INFO_ENTRY;

int    encode_ETYPE_INFO_ENTRY(unsigned char *, size_t, const ETYPE_INFO_ENTRY *, size_t *);
int    decode_ETYPE_INFO_ENTRY(const unsigned char *, size_t, ETYPE_INFO_ENTRY *, size_t *);
void   free_ETYPE_INFO_ENTRY  (ETYPE_INFO_ENTRY *);
size_t length_ETYPE_INFO_ENTRY(const ETYPE_INFO_ENTRY *);
int    copy_ETYPE_INFO_ENTRY  (const ETYPE_INFO_ENTRY *, ETYPE_INFO_ENTRY *);


/*
ETYPE-INFO ::= SEQUENCE OF ETYPE-INFO-ENTRY
*/

typedef struct ETYPE_INFO {
  unsigned int len;
  ETYPE_INFO_ENTRY *val;
} ETYPE_INFO;

int    encode_ETYPE_INFO(unsigned char *, size_t, const ETYPE_INFO *, size_t *);
int    decode_ETYPE_INFO(const unsigned char *, size_t, ETYPE_INFO *, size_t *);
void   free_ETYPE_INFO  (ETYPE_INFO *);
size_t length_ETYPE_INFO(const ETYPE_INFO *);
int    copy_ETYPE_INFO  (const ETYPE_INFO *, ETYPE_INFO *);
int   add_ETYPE_INFO  (ETYPE_INFO *, const ETYPE_INFO_ENTRY *);
int   remove_ETYPE_INFO  (ETYPE_INFO *, unsigned int);


/*
ETYPE-INFO2-ENTRY ::= SEQUENCE {
  etype           [0] ENCTYPE,
  salt            [1] KerberosString OPTIONAL,
  s2kparams       [2] OCTET STRING OPTIONAL,
}
*/

typedef struct ETYPE_INFO2_ENTRY {
  ENCTYPE etype;
  KerberosString *salt;
  heim_octet_string *s2kparams;
} ETYPE_INFO2_ENTRY;

int    encode_ETYPE_INFO2_ENTRY(unsigned char *, size_t, const ETYPE_INFO2_ENTRY *, size_t *);
int    decode_ETYPE_INFO2_ENTRY(const unsigned char *, size_t, ETYPE_INFO2_ENTRY *, size_t *);
void   free_ETYPE_INFO2_ENTRY  (ETYPE_INFO2_ENTRY *);
size_t length_ETYPE_INFO2_ENTRY(const ETYPE_INFO2_ENTRY *);
int    copy_ETYPE_INFO2_ENTRY  (const ETYPE_INFO2_ENTRY *, ETYPE_INFO2_ENTRY *);


/*
ETYPE-INFO2 ::= SEQUENCE OF ETYPE-INFO2-ENTRY
*/

typedef struct ETYPE_INFO2 {
  unsigned int len;
  ETYPE_INFO2_ENTRY *val;
} ETYPE_INFO2;

int    encode_ETYPE_INFO2(unsigned char *, size_t, const ETYPE_INFO2 *, size_t *);
int    decode_ETYPE_INFO2(const unsigned char *, size_t, ETYPE_INFO2 *, size_t *);
void   free_ETYPE_INFO2  (ETYPE_INFO2 *);
size_t length_ETYPE_INFO2(const ETYPE_INFO2 *);
int    copy_ETYPE_INFO2  (const ETYPE_INFO2 *, ETYPE_INFO2 *);
int   add_ETYPE_INFO2  (ETYPE_INFO2 *, const ETYPE_INFO2_ENTRY *);
int   remove_ETYPE_INFO2  (ETYPE_INFO2 *, unsigned int);


/*
METHOD-DATA ::= SEQUENCE OF PA-DATA
*/

typedef struct METHOD_DATA {
  unsigned int len;
  PA_DATA *val;
} METHOD_DATA;

int    encode_METHOD_DATA(unsigned char *, size_t, const METHOD_DATA *, size_t *);
int    decode_METHOD_DATA(const unsigned char *, size_t, METHOD_DATA *, size_t *);
void   free_METHOD_DATA  (METHOD_DATA *);
size_t length_METHOD_DATA(const METHOD_DATA *);
int    copy_METHOD_DATA  (const METHOD_DATA *, METHOD_DATA *);
int   add_METHOD_DATA  (METHOD_DATA *, const PA_DATA *);
int   remove_METHOD_DATA  (METHOD_DATA *, unsigned int);


/*
TypedData ::= SEQUENCE {
  data-type       [0] krb5int32,
  data-value      [1] OCTET STRING OPTIONAL,
}
*/

typedef struct TypedData {
  krb5int32 data_type;
  heim_octet_string *data_value;
} TypedData;

int    encode_TypedData(unsigned char *, size_t, const TypedData *, size_t *);
int    decode_TypedData(const unsigned char *, size_t, TypedData *, size_t *);
void   free_TypedData  (TypedData *);
size_t length_TypedData(const TypedData *);
int    copy_TypedData  (const TypedData *, TypedData *);


/*
TYPED-DATA ::= SEQUENCE OF TypedData
*/

typedef struct TYPED_DATA {
  unsigned int len;
  TypedData *val;
} TYPED_DATA;

int    encode_TYPED_DATA(unsigned char *, size_t, const TYPED_DATA *, size_t *);
int    decode_TYPED_DATA(const unsigned char *, size_t, TYPED_DATA *, size_t *);
void   free_TYPED_DATA  (TYPED_DATA *);
size_t length_TYPED_DATA(const TYPED_DATA *);
int    copy_TYPED_DATA  (const TYPED_DATA *, TYPED_DATA *);


/*
KDC-REQ-BODY ::= SEQUENCE {
  kdc-options              [0] KDCOptions,
  cname                    [1] PrincipalName OPTIONAL,
  realm                    [2] Realm,
  sname                    [3] PrincipalName OPTIONAL,
  from                     [4] KerberosTime OPTIONAL,
  till                     [5] KerberosTime OPTIONAL,
  rtime                    [6] KerberosTime OPTIONAL,
  nonce                    [7] krb5int32,
  etype                    [8] SEQUENCE OF ENCTYPE,
  addresses                [9] HostAddresses OPTIONAL,
  enc-authorization-data   [10] EncryptedData OPTIONAL,
  additional-tickets       [11] SEQUENCE OF Ticket OPTIONAL,
}
*/

typedef struct KDC_REQ_BODY {
  KDCOptions kdc_options;
  PrincipalName *cname;
  Realm realm;
  PrincipalName *sname;
  KerberosTime *from;
  KerberosTime *till;
  KerberosTime *rtime;
  krb5int32 nonce;
  struct  {
    unsigned int len;
    ENCTYPE *val;
  } etype;
  HostAddresses *addresses;
  EncryptedData *enc_authorization_data;
  struct  {
    unsigned int len;
    Ticket *val;
  } *additional_tickets;
} KDC_REQ_BODY;

int    encode_KDC_REQ_BODY(unsigned char *, size_t, const KDC_REQ_BODY *, size_t *);
int    decode_KDC_REQ_BODY(const unsigned char *, size_t, KDC_REQ_BODY *, size_t *);
void   free_KDC_REQ_BODY  (KDC_REQ_BODY *);
size_t length_KDC_REQ_BODY(const KDC_REQ_BODY *);
int    copy_KDC_REQ_BODY  (const KDC_REQ_BODY *, KDC_REQ_BODY *);


/*
KDC-REQ ::= SEQUENCE {
  pvno            [1] krb5int32,
  msg-type        [2] MESSAGE-TYPE,
  padata          [3] METHOD-DATA OPTIONAL,
  req-body        [4] KDC-REQ-BODY,
}
*/

typedef struct KDC_REQ {
  krb5int32 pvno;
  MESSAGE_TYPE msg_type;
  METHOD_DATA *padata;
  KDC_REQ_BODY req_body;
} KDC_REQ;

int    encode_KDC_REQ(unsigned char *, size_t, const KDC_REQ *, size_t *);
int    decode_KDC_REQ(const unsigned char *, size_t, KDC_REQ *, size_t *);
void   free_KDC_REQ  (KDC_REQ *);
size_t length_KDC_REQ(const KDC_REQ *);
int    copy_KDC_REQ  (const KDC_REQ *, KDC_REQ *);


/*
AS-REQ ::= [APPLICATION 10] KDC-REQ
*/

typedef KDC_REQ AS_REQ;

int    encode_AS_REQ(unsigned char *, size_t, const AS_REQ *, size_t *);
int    decode_AS_REQ(const unsigned char *, size_t, AS_REQ *, size_t *);
void   free_AS_REQ  (AS_REQ *);
size_t length_AS_REQ(const AS_REQ *);
int    copy_AS_REQ  (const AS_REQ *, AS_REQ *);


/*
TGS-REQ ::= [APPLICATION 12] KDC-REQ
*/

typedef KDC_REQ TGS_REQ;

int    encode_TGS_REQ(unsigned char *, size_t, const TGS_REQ *, size_t *);
int    decode_TGS_REQ(const unsigned char *, size_t, TGS_REQ *, size_t *);
void   free_TGS_REQ  (TGS_REQ *);
size_t length_TGS_REQ(const TGS_REQ *);
int    copy_TGS_REQ  (const TGS_REQ *, TGS_REQ *);


/*
PA-ENC-TS-ENC ::= SEQUENCE {
  patimestamp     [0] KerberosTime,
  pausec          [1] krb5int32 OPTIONAL,
}
*/

typedef struct PA_ENC_TS_ENC {
  KerberosTime patimestamp;
  krb5int32 *pausec;
} PA_ENC_TS_ENC;

int    encode_PA_ENC_TS_ENC(unsigned char *, size_t, const PA_ENC_TS_ENC *, size_t *);
int    decode_PA_ENC_TS_ENC(const unsigned char *, size_t, PA_ENC_TS_ENC *, size_t *);
void   free_PA_ENC_TS_ENC  (PA_ENC_TS_ENC *);
size_t length_PA_ENC_TS_ENC(const PA_ENC_TS_ENC *);
int    copy_PA_ENC_TS_ENC  (const PA_ENC_TS_ENC *, PA_ENC_TS_ENC *);


/*
PA-PAC-REQUEST ::= SEQUENCE {
  include-pac     [0] BOOLEAN,
}
*/

typedef struct PA_PAC_REQUEST {
  int include_pac;
} PA_PAC_REQUEST;

int    encode_PA_PAC_REQUEST(unsigned char *, size_t, const PA_PAC_REQUEST *, size_t *);
int    decode_PA_PAC_REQUEST(const unsigned char *, size_t, PA_PAC_REQUEST *, size_t *);
void   free_PA_PAC_REQUEST  (PA_PAC_REQUEST *);
size_t length_PA_PAC_REQUEST(const PA_PAC_REQUEST *);
int    copy_PA_PAC_REQUEST  (const PA_PAC_REQUEST *, PA_PAC_REQUEST *);


/*
PROV-SRV-LOCATION ::= GeneralString
*/

typedef heim_general_string PROV_SRV_LOCATION;

int    encode_PROV_SRV_LOCATION(unsigned char *, size_t, const PROV_SRV_LOCATION *, size_t *);
int    decode_PROV_SRV_LOCATION(const unsigned char *, size_t, PROV_SRV_LOCATION *, size_t *);
void   free_PROV_SRV_LOCATION  (PROV_SRV_LOCATION *);
size_t length_PROV_SRV_LOCATION(const PROV_SRV_LOCATION *);
int    copy_PROV_SRV_LOCATION  (const PROV_SRV_LOCATION *, PROV_SRV_LOCATION *);


/*
KDC-REP ::= SEQUENCE {
  pvno            [0] krb5int32,
  msg-type        [1] MESSAGE-TYPE,
  padata          [2] METHOD-DATA OPTIONAL,
  crealm          [3] Realm,
  cname           [4] PrincipalName,
  ticket          [5] Ticket,
  enc-part        [6] EncryptedData,
}
*/

typedef struct KDC_REP {
  krb5int32 pvno;
  MESSAGE_TYPE msg_type;
  METHOD_DATA *padata;
  Realm crealm;
  PrincipalName cname;
  Ticket ticket;
  EncryptedData enc_part;
} KDC_REP;

int    encode_KDC_REP(unsigned char *, size_t, const KDC_REP *, size_t *);
int    decode_KDC_REP(const unsigned char *, size_t, KDC_REP *, size_t *);
void   free_KDC_REP  (KDC_REP *);
size_t length_KDC_REP(const KDC_REP *);
int    copy_KDC_REP  (const KDC_REP *, KDC_REP *);


/*
AS-REP ::= [APPLICATION 11] KDC-REP
*/

typedef KDC_REP AS_REP;

int    encode_AS_REP(unsigned char *, size_t, const AS_REP *, size_t *);
int    decode_AS_REP(const unsigned char *, size_t, AS_REP *, size_t *);
void   free_AS_REP  (AS_REP *);
size_t length_AS_REP(const AS_REP *);
int    copy_AS_REP  (const AS_REP *, AS_REP *);


/*
TGS-REP ::= [APPLICATION 13] KDC-REP
*/

typedef KDC_REP TGS_REP;

int    encode_TGS_REP(unsigned char *, size_t, const TGS_REP *, size_t *);
int    decode_TGS_REP(const unsigned char *, size_t, TGS_REP *, size_t *);
void   free_TGS_REP  (TGS_REP *);
size_t length_TGS_REP(const TGS_REP *);
int    copy_TGS_REP  (const TGS_REP *, TGS_REP *);


/*
EncKDCRepPart ::= SEQUENCE {
  key                 [0] EncryptionKey,
  last-req            [1] LastReq,
  nonce               [2] krb5int32,
  key-expiration      [3] KerberosTime OPTIONAL,
  flags               [4] TicketFlags,
  authtime            [5] KerberosTime,
  starttime           [6] KerberosTime OPTIONAL,
  endtime             [7] KerberosTime,
  renew-till          [8] KerberosTime OPTIONAL,
  srealm              [9] Realm,
  sname               [10] PrincipalName,
  caddr               [11] HostAddresses OPTIONAL,
  encrypted-pa-data   [12] METHOD-DATA OPTIONAL,
}
*/

typedef struct EncKDCRepPart {
  EncryptionKey key;
  LastReq last_req;
  krb5int32 nonce;
  KerberosTime *key_expiration;
  TicketFlags flags;
  KerberosTime authtime;
  KerberosTime *starttime;
  KerberosTime endtime;
  KerberosTime *renew_till;
  Realm srealm;
  PrincipalName sname;
  HostAddresses *caddr;
  METHOD_DATA *encrypted_pa_data;
} EncKDCRepPart;

int    encode_EncKDCRepPart(unsigned char *, size_t, const EncKDCRepPart *, size_t *);
int    decode_EncKDCRepPart(const unsigned char *, size_t, EncKDCRepPart *, size_t *);
void   free_EncKDCRepPart  (EncKDCRepPart *);
size_t length_EncKDCRepPart(const EncKDCRepPart *);
int    copy_EncKDCRepPart  (const EncKDCRepPart *, EncKDCRepPart *);


/*
EncASRepPart ::= [APPLICATION 25] EncKDCRepPart
*/

typedef EncKDCRepPart EncASRepPart;

int    encode_EncASRepPart(unsigned char *, size_t, const EncASRepPart *, size_t *);
int    decode_EncASRepPart(const unsigned char *, size_t, EncASRepPart *, size_t *);
void   free_EncASRepPart  (EncASRepPart *);
size_t length_EncASRepPart(const EncASRepPart *);
int    copy_EncASRepPart  (const EncASRepPart *, EncASRepPart *);


/*
EncTGSRepPart ::= [APPLICATION 26] EncKDCRepPart
*/

typedef EncKDCRepPart EncTGSRepPart;

int    encode_EncTGSRepPart(unsigned char *, size_t, const EncTGSRepPart *, size_t *);
int    decode_EncTGSRepPart(const unsigned char *, size_t, EncTGSRepPart *, size_t *);
void   free_EncTGSRepPart  (EncTGSRepPart *);
size_t length_EncTGSRepPart(const EncTGSRepPart *);
int    copy_EncTGSRepPart  (const EncTGSRepPart *, EncTGSRepPart *);


/*
AP-REQ ::= [APPLICATION 14] SEQUENCE {
  pvno            [0] krb5int32,
  msg-type        [1] MESSAGE-TYPE,
  ap-options      [2] APOptions,
  ticket          [3] Ticket,
  authenticator   [4] EncryptedData,
}
*/

typedef struct AP_REQ {
  krb5int32 pvno;
  MESSAGE_TYPE msg_type;
  APOptions ap_options;
  Ticket ticket;
  EncryptedData authenticator;
} AP_REQ;

int    encode_AP_REQ(unsigned char *, size_t, const AP_REQ *, size_t *);
int    decode_AP_REQ(const unsigned char *, size_t, AP_REQ *, size_t *);
void   free_AP_REQ  (AP_REQ *);
size_t length_AP_REQ(const AP_REQ *);
int    copy_AP_REQ  (const AP_REQ *, AP_REQ *);


/*
AP-REP ::= [APPLICATION 15] SEQUENCE {
  pvno            [0] krb5int32,
  msg-type        [1] MESSAGE-TYPE,
  enc-part        [2] EncryptedData,
}
*/

typedef struct AP_REP {
  krb5int32 pvno;
  MESSAGE_TYPE msg_type;
  EncryptedData enc_part;
} AP_REP;

int    encode_AP_REP(unsigned char *, size_t, const AP_REP *, size_t *);
int    decode_AP_REP(const unsigned char *, size_t, AP_REP *, size_t *);
void   free_AP_REP  (AP_REP *);
size_t length_AP_REP(const AP_REP *);
int    copy_AP_REP  (const AP_REP *, AP_REP *);


/*
EncAPRepPart ::= [APPLICATION 27] SEQUENCE {
  ctime           [0] KerberosTime,
  cusec           [1] krb5int32,
  subkey          [2] EncryptionKey OPTIONAL,
  seq-number      [3] krb5uint32 OPTIONAL,
}
*/

typedef struct EncAPRepPart {
  KerberosTime ctime;
  krb5int32 cusec;
  EncryptionKey *subkey;
  krb5uint32 *seq_number;
} EncAPRepPart;

int    encode_EncAPRepPart(unsigned char *, size_t, const EncAPRepPart *, size_t *);
int    decode_EncAPRepPart(const unsigned char *, size_t, EncAPRepPart *, size_t *);
void   free_EncAPRepPart  (EncAPRepPart *);
size_t length_EncAPRepPart(const EncAPRepPart *);
int    copy_EncAPRepPart  (const EncAPRepPart *, EncAPRepPart *);


/*
KRB-SAFE-BODY ::= SEQUENCE {
  user-data       [0] OCTET STRING,
  timestamp       [1] KerberosTime OPTIONAL,
  usec            [2] krb5int32 OPTIONAL,
  seq-number      [3] krb5uint32 OPTIONAL,
  s-address       [4] HostAddress OPTIONAL,
  r-address       [5] HostAddress OPTIONAL,
}
*/

typedef struct KRB_SAFE_BODY {
  heim_octet_string user_data;
  KerberosTime *timestamp;
  krb5int32 *usec;
  krb5uint32 *seq_number;
  HostAddress *s_address;
  HostAddress *r_address;
} KRB_SAFE_BODY;

int    encode_KRB_SAFE_BODY(unsigned char *, size_t, const KRB_SAFE_BODY *, size_t *);
int    decode_KRB_SAFE_BODY(const unsigned char *, size_t, KRB_SAFE_BODY *, size_t *);
void   free_KRB_SAFE_BODY  (KRB_SAFE_BODY *);
size_t length_KRB_SAFE_BODY(const KRB_SAFE_BODY *);
int    copy_KRB_SAFE_BODY  (const KRB_SAFE_BODY *, KRB_SAFE_BODY *);


/*
KRB-SAFE ::= [APPLICATION 20] SEQUENCE {
  pvno            [0] krb5int32,
  msg-type        [1] MESSAGE-TYPE,
  safe-body       [2] KRB-SAFE-BODY,
  cksum           [3] Checksum,
}
*/

typedef struct KRB_SAFE {
  krb5int32 pvno;
  MESSAGE_TYPE msg_type;
  KRB_SAFE_BODY safe_body;
  Checksum cksum;
} KRB_SAFE;

int    encode_KRB_SAFE(unsigned char *, size_t, const KRB_SAFE *, size_t *);
int    decode_KRB_SAFE(const unsigned char *, size_t, KRB_SAFE *, size_t *);
void   free_KRB_SAFE  (KRB_SAFE *);
size_t length_KRB_SAFE(const KRB_SAFE *);
int    copy_KRB_SAFE  (const KRB_SAFE *, KRB_SAFE *);


/*
KRB-PRIV ::= [APPLICATION 21] SEQUENCE {
  pvno            [0] krb5int32,
  msg-type        [1] MESSAGE-TYPE,
  enc-part        [3] EncryptedData,
}
*/

typedef struct KRB_PRIV {
  krb5int32 pvno;
  MESSAGE_TYPE msg_type;
  EncryptedData enc_part;
} KRB_PRIV;

int    encode_KRB_PRIV(unsigned char *, size_t, const KRB_PRIV *, size_t *);
int    decode_KRB_PRIV(const unsigned char *, size_t, KRB_PRIV *, size_t *);
void   free_KRB_PRIV  (KRB_PRIV *);
size_t length_KRB_PRIV(const KRB_PRIV *);
int    copy_KRB_PRIV  (const KRB_PRIV *, KRB_PRIV *);


/*
EncKrbPrivPart ::= [APPLICATION 28] SEQUENCE {
  user-data       [0] OCTET STRING,
  timestamp       [1] KerberosTime OPTIONAL,
  usec            [2] krb5int32 OPTIONAL,
  seq-number      [3] krb5uint32 OPTIONAL,
  s-address       [4] HostAddress OPTIONAL,
  r-address       [5] HostAddress OPTIONAL,
}
*/

typedef struct EncKrbPrivPart {
  heim_octet_string user_data;
  KerberosTime *timestamp;
  krb5int32 *usec;
  krb5uint32 *seq_number;
  HostAddress *s_address;
  HostAddress *r_address;
} EncKrbPrivPart;

int    encode_EncKrbPrivPart(unsigned char *, size_t, const EncKrbPrivPart *, size_t *);
int    decode_EncKrbPrivPart(const unsigned char *, size_t, EncKrbPrivPart *, size_t *);
void   free_EncKrbPrivPart  (EncKrbPrivPart *);
size_t length_EncKrbPrivPart(const EncKrbPrivPart *);
int    copy_EncKrbPrivPart  (const EncKrbPrivPart *, EncKrbPrivPart *);


/*
KRB-CRED ::= [APPLICATION 22] SEQUENCE {
  pvno            [0] krb5int32,
  msg-type        [1] MESSAGE-TYPE,
  tickets         [2] SEQUENCE OF Ticket,
  enc-part        [3] EncryptedData,
}
*/

typedef struct KRB_CRED {
  krb5int32 pvno;
  MESSAGE_TYPE msg_type;
  struct  {
    unsigned int len;
    Ticket *val;
  } tickets;
  EncryptedData enc_part;
} KRB_CRED;

int    encode_KRB_CRED(unsigned char *, size_t, const KRB_CRED *, size_t *);
int    decode_KRB_CRED(const unsigned char *, size_t, KRB_CRED *, size_t *);
void   free_KRB_CRED  (KRB_CRED *);
size_t length_KRB_CRED(const KRB_CRED *);
int    copy_KRB_CRED  (const KRB_CRED *, KRB_CRED *);


/*
KrbCredInfo ::= SEQUENCE {
  key             [0] EncryptionKey,
  prealm          [1] Realm OPTIONAL,
  pname           [2] PrincipalName OPTIONAL,
  flags           [3] TicketFlags OPTIONAL,
  authtime        [4] KerberosTime OPTIONAL,
  starttime       [5] KerberosTime OPTIONAL,
  endtime         [6] KerberosTime OPTIONAL,
  renew-till      [7] KerberosTime OPTIONAL,
  srealm          [8] Realm OPTIONAL,
  sname           [9] PrincipalName OPTIONAL,
  caddr           [10] HostAddresses OPTIONAL,
}
*/

typedef struct KrbCredInfo {
  EncryptionKey key;
  Realm *prealm;
  PrincipalName *pname;
  TicketFlags *flags;
  KerberosTime *authtime;
  KerberosTime *starttime;
  KerberosTime *endtime;
  KerberosTime *renew_till;
  Realm *srealm;
  PrincipalName *sname;
  HostAddresses *caddr;
} KrbCredInfo;

int    encode_KrbCredInfo(unsigned char *, size_t, const KrbCredInfo *, size_t *);
int    decode_KrbCredInfo(const unsigned char *, size_t, KrbCredInfo *, size_t *);
void   free_KrbCredInfo  (KrbCredInfo *);
size_t length_KrbCredInfo(const KrbCredInfo *);
int    copy_KrbCredInfo  (const KrbCredInfo *, KrbCredInfo *);


/*
EncKrbCredPart ::= [APPLICATION 29] SEQUENCE {
  ticket-info     [0] SEQUENCE OF KrbCredInfo,
  nonce           [1] krb5int32 OPTIONAL,
  timestamp       [2] KerberosTime OPTIONAL,
  usec            [3] krb5int32 OPTIONAL,
  s-address       [4] HostAddress OPTIONAL,
  r-address       [5] HostAddress OPTIONAL,
}
*/

typedef struct EncKrbCredPart {
  struct  {
    unsigned int len;
    KrbCredInfo *val;
  } ticket_info;
  krb5int32 *nonce;
  KerberosTime *timestamp;
  krb5int32 *usec;
  HostAddress *s_address;
  HostAddress *r_address;
} EncKrbCredPart;

int    encode_EncKrbCredPart(unsigned char *, size_t, const EncKrbCredPart *, size_t *);
int    decode_EncKrbCredPart(const unsigned char *, size_t, EncKrbCredPart *, size_t *);
void   free_EncKrbCredPart  (EncKrbCredPart *);
size_t length_EncKrbCredPart(const EncKrbCredPart *);
int    copy_EncKrbCredPart  (const EncKrbCredPart *, EncKrbCredPart *);


/*
KRB-ERROR ::= [APPLICATION 30] SEQUENCE {
  pvno            [0] krb5int32,
  msg-type        [1] MESSAGE-TYPE,
  ctime           [2] KerberosTime OPTIONAL,
  cusec           [3] krb5int32 OPTIONAL,
  stime           [4] KerberosTime,
  susec           [5] krb5int32,
  error-code      [6] krb5int32,
  crealm          [7] Realm OPTIONAL,
  cname           [8] PrincipalName OPTIONAL,
  realm           [9] Realm,
  sname           [10] PrincipalName,
  e-text          [11] GeneralString OPTIONAL,
  e-data          [12] OCTET STRING OPTIONAL,
}
*/

typedef struct KRB_ERROR {
  krb5int32 pvno;
  MESSAGE_TYPE msg_type;
  KerberosTime *ctime;
  krb5int32 *cusec;
  KerberosTime stime;
  krb5int32 susec;
  krb5int32 error_code;
  Realm *crealm;
  PrincipalName *cname;
  Realm realm;
  PrincipalName sname;
  heim_general_string *e_text;
  heim_octet_string *e_data;
} KRB_ERROR;

int    encode_KRB_ERROR(unsigned char *, size_t, const KRB_ERROR *, size_t *);
int    decode_KRB_ERROR(const unsigned char *, size_t, KRB_ERROR *, size_t *);
void   free_KRB_ERROR  (KRB_ERROR *);
size_t length_KRB_ERROR(const KRB_ERROR *);
int    copy_KRB_ERROR  (const KRB_ERROR *, KRB_ERROR *);


/*
ChangePasswdDataMS ::= SEQUENCE {
  newpasswd       [0] OCTET STRING,
  targname        [1] PrincipalName OPTIONAL,
  targrealm       [2] Realm OPTIONAL,
}
*/

typedef struct ChangePasswdDataMS {
  heim_octet_string newpasswd;
  PrincipalName *targname;
  Realm *targrealm;
} ChangePasswdDataMS;

int    encode_ChangePasswdDataMS(unsigned char *, size_t, const ChangePasswdDataMS *, size_t *);
int    decode_ChangePasswdDataMS(const unsigned char *, size_t, ChangePasswdDataMS *, size_t *);
void   free_ChangePasswdDataMS  (ChangePasswdDataMS *);
size_t length_ChangePasswdDataMS(const ChangePasswdDataMS *);
int    copy_ChangePasswdDataMS  (const ChangePasswdDataMS *, ChangePasswdDataMS *);


/*
EtypeList ::= SEQUENCE OF krb5int32
*/

typedef struct EtypeList {
  unsigned int len;
  krb5int32 *val;
} EtypeList;

int    encode_EtypeList(unsigned char *, size_t, const EtypeList *, size_t *);
int    decode_EtypeList(const unsigned char *, size_t, EtypeList *, size_t *);
void   free_EtypeList  (EtypeList *);
size_t length_EtypeList(const EtypeList *);
int    copy_EtypeList  (const EtypeList *, EtypeList *);


enum { krb5_pvno = 5 };

enum { DOMAIN_X500_COMPRESS = 1 };

/*
AD-IF-RELEVANT ::= AuthorizationData
*/

typedef AuthorizationData AD_IF_RELEVANT;

int    encode_AD_IF_RELEVANT(unsigned char *, size_t, const AD_IF_RELEVANT *, size_t *);
int    decode_AD_IF_RELEVANT(const unsigned char *, size_t, AD_IF_RELEVANT *, size_t *);
void   free_AD_IF_RELEVANT  (AD_IF_RELEVANT *);
size_t length_AD_IF_RELEVANT(const AD_IF_RELEVANT *);
int    copy_AD_IF_RELEVANT  (const AD_IF_RELEVANT *, AD_IF_RELEVANT *);


/*
AD-KDCIssued ::= SEQUENCE {
  ad-checksum     [0] Checksum,
  i-realm         [1] Realm OPTIONAL,
  i-sname         [2] PrincipalName OPTIONAL,
  elements        [3] AuthorizationData,
}
*/

typedef struct AD_KDCIssued {
  Checksum ad_checksum;
  Realm *i_realm;
  PrincipalName *i_sname;
  AuthorizationData elements;
} AD_KDCIssued;

int    encode_AD_KDCIssued(unsigned char *, size_t, const AD_KDCIssued *, size_t *);
int    decode_AD_KDCIssued(const unsigned char *, size_t, AD_KDCIssued *, size_t *);
void   free_AD_KDCIssued  (AD_KDCIssued *);
size_t length_AD_KDCIssued(const AD_KDCIssued *);
int    copy_AD_KDCIssued  (const AD_KDCIssued *, AD_KDCIssued *);


/*
AD-AND-OR ::= SEQUENCE {
  condition-count   [0] INTEGER,
  elements          [1] AuthorizationData,
}
*/

typedef struct AD_AND_OR {
  heim_integer condition_count;
  AuthorizationData elements;
} AD_AND_OR;

int    encode_AD_AND_OR(unsigned char *, size_t, const AD_AND_OR *, size_t *);
int    decode_AD_AND_OR(const unsigned char *, size_t, AD_AND_OR *, size_t *);
void   free_AD_AND_OR  (AD_AND_OR *);
size_t length_AD_AND_OR(const AD_AND_OR *);
int    copy_AD_AND_OR  (const AD_AND_OR *, AD_AND_OR *);


/*
AD-MANDATORY-FOR-KDC ::= AuthorizationData
*/

typedef AuthorizationData AD_MANDATORY_FOR_KDC;

int    encode_AD_MANDATORY_FOR_KDC(unsigned char *, size_t, const AD_MANDATORY_FOR_KDC *, size_t *);
int    decode_AD_MANDATORY_FOR_KDC(const unsigned char *, size_t, AD_MANDATORY_FOR_KDC *, size_t *);
void   free_AD_MANDATORY_FOR_KDC  (AD_MANDATORY_FOR_KDC *);
size_t length_AD_MANDATORY_FOR_KDC(const AD_MANDATORY_FOR_KDC *);
int    copy_AD_MANDATORY_FOR_KDC  (const AD_MANDATORY_FOR_KDC *, AD_MANDATORY_FOR_KDC *);


/*
PA-SAM-TYPE ::= INTEGER {
  PA_SAM_TYPE_ENIGMA(1),
  PA_SAM_TYPE_DIGI_PATH(2),
  PA_SAM_TYPE_SKEY_K0(3),
  PA_SAM_TYPE_SKEY(4),
  PA_SAM_TYPE_SECURID(5),
  PA_SAM_TYPE_CRYPTOCARD(6)
}
*/

typedef enum PA_SAM_TYPE {
  PA_SAM_TYPE_ENIGMA = 1,
  PA_SAM_TYPE_DIGI_PATH = 2,
  PA_SAM_TYPE_SKEY_K0 = 3,
  PA_SAM_TYPE_SKEY = 4,
  PA_SAM_TYPE_SECURID = 5,
  PA_SAM_TYPE_CRYPTOCARD = 6
} PA_SAM_TYPE;

int    encode_PA_SAM_TYPE(unsigned char *, size_t, const PA_SAM_TYPE *, size_t *);
int    decode_PA_SAM_TYPE(const unsigned char *, size_t, PA_SAM_TYPE *, size_t *);
void   free_PA_SAM_TYPE  (PA_SAM_TYPE *);
size_t length_PA_SAM_TYPE(const PA_SAM_TYPE *);
int    copy_PA_SAM_TYPE  (const PA_SAM_TYPE *, PA_SAM_TYPE *);


/*
PA-SAM-REDIRECT ::= HostAddresses
*/

typedef HostAddresses PA_SAM_REDIRECT;

int    encode_PA_SAM_REDIRECT(unsigned char *, size_t, const PA_SAM_REDIRECT *, size_t *);
int    decode_PA_SAM_REDIRECT(const unsigned char *, size_t, PA_SAM_REDIRECT *, size_t *);
void   free_PA_SAM_REDIRECT  (PA_SAM_REDIRECT *);
size_t length_PA_SAM_REDIRECT(const PA_SAM_REDIRECT *);
int    copy_PA_SAM_REDIRECT  (const PA_SAM_REDIRECT *, PA_SAM_REDIRECT *);


/*
SAMFlags ::= BIT STRING {
  use-sad-as-key(0),
  send-encrypted-sad(1),
  must-pk-encrypt-sad(2)
}
*/

typedef struct SAMFlags {
  unsigned int use_sad_as_key:1;
  unsigned int send_encrypted_sad:1;
  unsigned int must_pk_encrypt_sad:1;
} SAMFlags;


int    encode_SAMFlags(unsigned char *, size_t, const SAMFlags *, size_t *);
int    decode_SAMFlags(const unsigned char *, size_t, SAMFlags *, size_t *);
void   free_SAMFlags  (SAMFlags *);
size_t length_SAMFlags(const SAMFlags *);
int    copy_SAMFlags  (const SAMFlags *, SAMFlags *);
unsigned SAMFlags2int(SAMFlags);
SAMFlags int2SAMFlags(unsigned);
#ifdef __PARSE_UNITS_H__
const struct units * asn1_SAMFlags_units(void);
#endif


/*
PA-SAM-CHALLENGE-2-BODY ::= SEQUENCE {
  sam-type              [0] krb5int32,
  sam-flags             [1] SAMFlags,
  sam-type-name         [2] GeneralString OPTIONAL,
  sam-track-id          [3] GeneralString OPTIONAL,
  sam-challenge-label   [4] GeneralString OPTIONAL,
  sam-challenge         [5] GeneralString OPTIONAL,
  sam-response-prompt   [6] GeneralString OPTIONAL,
  sam-pk-for-sad        [7] EncryptionKey OPTIONAL,
  sam-nonce             [8] krb5int32,
  sam-etype             [9] krb5int32,
  ...,
}
*/

typedef struct PA_SAM_CHALLENGE_2_BODY {
  krb5int32 sam_type;
  SAMFlags sam_flags;
  heim_general_string *sam_type_name;
  heim_general_string *sam_track_id;
  heim_general_string *sam_challenge_label;
  heim_general_string *sam_challenge;
  heim_general_string *sam_response_prompt;
  EncryptionKey *sam_pk_for_sad;
  krb5int32 sam_nonce;
  krb5int32 sam_etype;
} PA_SAM_CHALLENGE_2_BODY;

int    encode_PA_SAM_CHALLENGE_2_BODY(unsigned char *, size_t, const PA_SAM_CHALLENGE_2_BODY *, size_t *);
int    decode_PA_SAM_CHALLENGE_2_BODY(const unsigned char *, size_t, PA_SAM_CHALLENGE_2_BODY *, size_t *);
void   free_PA_SAM_CHALLENGE_2_BODY  (PA_SAM_CHALLENGE_2_BODY *);
size_t length_PA_SAM_CHALLENGE_2_BODY(const PA_SAM_CHALLENGE_2_BODY *);
int    copy_PA_SAM_CHALLENGE_2_BODY  (const PA_SAM_CHALLENGE_2_BODY *, PA_SAM_CHALLENGE_2_BODY *);


/*
PA-SAM-CHALLENGE-2 ::= SEQUENCE {
  sam-body        [0] PA-SAM-CHALLENGE-2-BODY,
  sam-cksum       [1] SEQUENCE OF Checksum,
  ...,
}
*/

typedef struct PA_SAM_CHALLENGE_2 {
  PA_SAM_CHALLENGE_2_BODY sam_body;
  struct  {
    unsigned int len;
    Checksum *val;
  } sam_cksum;
} PA_SAM_CHALLENGE_2;

int    encode_PA_SAM_CHALLENGE_2(unsigned char *, size_t, const PA_SAM_CHALLENGE_2 *, size_t *);
int    decode_PA_SAM_CHALLENGE_2(const unsigned char *, size_t, PA_SAM_CHALLENGE_2 *, size_t *);
void   free_PA_SAM_CHALLENGE_2  (PA_SAM_CHALLENGE_2 *);
size_t length_PA_SAM_CHALLENGE_2(const PA_SAM_CHALLENGE_2 *);
int    copy_PA_SAM_CHALLENGE_2  (const PA_SAM_CHALLENGE_2 *, PA_SAM_CHALLENGE_2 *);


/*
PA-SAM-RESPONSE-2 ::= SEQUENCE {
  sam-type               [0] krb5int32,
  sam-flags              [1] SAMFlags,
  sam-track-id           [2] GeneralString OPTIONAL,
  sam-enc-nonce-or-sad   [3] EncryptedData,
  sam-nonce              [4] krb5int32,
  ...,
}
*/

typedef struct PA_SAM_RESPONSE_2 {
  krb5int32 sam_type;
  SAMFlags sam_flags;
  heim_general_string *sam_track_id;
  EncryptedData sam_enc_nonce_or_sad;
  krb5int32 sam_nonce;
} PA_SAM_RESPONSE_2;

int    encode_PA_SAM_RESPONSE_2(unsigned char *, size_t, const PA_SAM_RESPONSE_2 *, size_t *);
int    decode_PA_SAM_RESPONSE_2(const unsigned char *, size_t, PA_SAM_RESPONSE_2 *, size_t *);
void   free_PA_SAM_RESPONSE_2  (PA_SAM_RESPONSE_2 *);
size_t length_PA_SAM_RESPONSE_2(const PA_SAM_RESPONSE_2 *);
int    copy_PA_SAM_RESPONSE_2  (const PA_SAM_RESPONSE_2 *, PA_SAM_RESPONSE_2 *);


/*
PA-ENC-SAM-RESPONSE-ENC ::= SEQUENCE {
  sam-nonce       [0] krb5int32,
  sam-sad         [1] GeneralString OPTIONAL,
  ...,
}
*/

typedef struct PA_ENC_SAM_RESPONSE_ENC {
  krb5int32 sam_nonce;
  heim_general_string *sam_sad;
} PA_ENC_SAM_RESPONSE_ENC;

int    encode_PA_ENC_SAM_RESPONSE_ENC(unsigned char *, size_t, const PA_ENC_SAM_RESPONSE_ENC *, size_t *);
int    decode_PA_ENC_SAM_RESPONSE_ENC(const unsigned char *, size_t, PA_ENC_SAM_RESPONSE_ENC *, size_t *);
void   free_PA_ENC_SAM_RESPONSE_ENC  (PA_ENC_SAM_RESPONSE_ENC *);
size_t length_PA_ENC_SAM_RESPONSE_ENC(const PA_ENC_SAM_RESPONSE_ENC *);
int    copy_PA_ENC_SAM_RESPONSE_ENC  (const PA_ENC_SAM_RESPONSE_ENC *, PA_ENC_SAM_RESPONSE_ENC *);


/*
PA-S4U2Self ::= SEQUENCE {
  name            [0] PrincipalName,
  realm           [1] Realm,
  cksum           [2] Checksum,
  auth            [3] GeneralString,
}
*/

typedef struct PA_S4U2Self {
  PrincipalName name;
  Realm realm;
  Checksum cksum;
  heim_general_string auth;
} PA_S4U2Self;

int    encode_PA_S4U2Self(unsigned char *, size_t, const PA_S4U2Self *, size_t *);
int    decode_PA_S4U2Self(const unsigned char *, size_t, PA_S4U2Self *, size_t *);
void   free_PA_S4U2Self  (PA_S4U2Self *);
size_t length_PA_S4U2Self(const PA_S4U2Self *);
int    copy_PA_S4U2Self  (const PA_S4U2Self *, PA_S4U2Self *);


/*
KRB5SignedPathPrincipals ::= SEQUENCE OF Principal
*/

typedef struct KRB5SignedPathPrincipals {
  unsigned int len;
  Principal *val;
} KRB5SignedPathPrincipals;

int    encode_KRB5SignedPathPrincipals(unsigned char *, size_t, const KRB5SignedPathPrincipals *, size_t *);
int    decode_KRB5SignedPathPrincipals(const unsigned char *, size_t, KRB5SignedPathPrincipals *, size_t *);
void   free_KRB5SignedPathPrincipals  (KRB5SignedPathPrincipals *);
size_t length_KRB5SignedPathPrincipals(const KRB5SignedPathPrincipals *);
int    copy_KRB5SignedPathPrincipals  (const KRB5SignedPathPrincipals *, KRB5SignedPathPrincipals *);
int   add_KRB5SignedPathPrincipals  (KRB5SignedPathPrincipals *, const Principal *);
int   remove_KRB5SignedPathPrincipals  (KRB5SignedPathPrincipals *, unsigned int);


/*
KRB5SignedPathData ::= SEQUENCE {
  encticket       [0] EncTicketPart,
  delegated       [1] KRB5SignedPathPrincipals OPTIONAL,
}
*/

typedef struct KRB5SignedPathData {
  EncTicketPart encticket;
  KRB5SignedPathPrincipals *delegated;
} KRB5SignedPathData;

int    encode_KRB5SignedPathData(unsigned char *, size_t, const KRB5SignedPathData *, size_t *);
int    decode_KRB5SignedPathData(const unsigned char *, size_t, KRB5SignedPathData *, size_t *);
void   free_KRB5SignedPathData  (KRB5SignedPathData *);
size_t length_KRB5SignedPathData(const KRB5SignedPathData *);
int    copy_KRB5SignedPathData  (const KRB5SignedPathData *, KRB5SignedPathData *);


/*
KRB5SignedPath ::= SEQUENCE {
  etype           [0] ENCTYPE,
  cksum           [1] Checksum,
  delegated       [2] KRB5SignedPathPrincipals OPTIONAL,
}
*/

typedef struct KRB5SignedPath {
  ENCTYPE etype;
  Checksum cksum;
  KRB5SignedPathPrincipals *delegated;
} KRB5SignedPath;

int    encode_KRB5SignedPath(unsigned char *, size_t, const KRB5SignedPath *, size_t *);
int    decode_KRB5SignedPath(const unsigned char *, size_t, KRB5SignedPath *, size_t *);
void   free_KRB5SignedPath  (KRB5SignedPath *);
size_t length_KRB5SignedPath(const KRB5SignedPath *);
int    copy_KRB5SignedPath  (const KRB5SignedPath *, KRB5SignedPath *);


/*
PA-ClientCanonicalizedNames ::= SEQUENCE {
  requested-name   [0] PrincipalName,
  real-name        [1] PrincipalName,
}
*/

typedef struct PA_ClientCanonicalizedNames {
  PrincipalName requested_name;
  PrincipalName real_name;
} PA_ClientCanonicalizedNames;

int    encode_PA_ClientCanonicalizedNames(unsigned char *, size_t, const PA_ClientCanonicalizedNames *, size_t *);
int    decode_PA_ClientCanonicalizedNames(const unsigned char *, size_t, PA_ClientCanonicalizedNames *, size_t *);
void   free_PA_ClientCanonicalizedNames  (PA_ClientCanonicalizedNames *);
size_t length_PA_ClientCanonicalizedNames(const PA_ClientCanonicalizedNames *);
int    copy_PA_ClientCanonicalizedNames  (const PA_ClientCanonicalizedNames *, PA_ClientCanonicalizedNames *);


/*
PA-ClientCanonicalized ::= SEQUENCE {
  names            [0] PA-ClientCanonicalizedNames,
  canon-checksum   [1] Checksum,
}
*/

typedef struct PA_ClientCanonicalized {
  PA_ClientCanonicalizedNames names;
  Checksum canon_checksum;
} PA_ClientCanonicalized;

int    encode_PA_ClientCanonicalized(unsigned char *, size_t, const PA_ClientCanonicalized *, size_t *);
int    decode_PA_ClientCanonicalized(const unsigned char *, size_t, PA_ClientCanonicalized *, size_t *);
void   free_PA_ClientCanonicalized  (PA_ClientCanonicalized *);
size_t length_PA_ClientCanonicalized(const PA_ClientCanonicalized *);
int    copy_PA_ClientCanonicalized  (const PA_ClientCanonicalized *, PA_ClientCanonicalized *);


/*
AD-LoginAlias ::= SEQUENCE {
  login-alias     [0] PrincipalName,
  checksum        [1] Checksum,
}
*/

typedef struct AD_LoginAlias {
  PrincipalName login_alias;
  Checksum checksum;
} AD_LoginAlias;

int    encode_AD_LoginAlias(unsigned char *, size_t, const AD_LoginAlias *, size_t *);
int    decode_AD_LoginAlias(const unsigned char *, size_t, AD_LoginAlias *, size_t *);
void   free_AD_LoginAlias  (AD_LoginAlias *);
size_t length_AD_LoginAlias(const AD_LoginAlias *);
int    copy_AD_LoginAlias  (const AD_LoginAlias *, AD_LoginAlias *);


/*
PA-SvrReferralData ::= SEQUENCE {
  referred-name    [1] PrincipalName OPTIONAL,
  referred-realm   [0] Realm,
}
*/

typedef struct PA_SvrReferralData {
  PrincipalName *referred_name;
  Realm referred_realm;
} PA_SvrReferralData;

int    encode_PA_SvrReferralData(unsigned char *, size_t, const PA_SvrReferralData *, size_t *);
int    decode_PA_SvrReferralData(const unsigned char *, size_t, PA_SvrReferralData *, size_t *);
void   free_PA_SvrReferralData  (PA_SvrReferralData *);
size_t length_PA_SvrReferralData(const PA_SvrReferralData *);
int    copy_PA_SvrReferralData  (const PA_SvrReferralData *, PA_SvrReferralData *);


#endif /* __krb5_asn1_h__ */
