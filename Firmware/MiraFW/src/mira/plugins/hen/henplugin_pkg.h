#pragma once
#include "henplugin.h"

#define EKPFS_SIZE 0x20
#define EEKPFS_SIZE 0x100
#define PFS_SEED_SIZE 0x10
#define PFS_FINAL_KEY_SIZE 0x20
#define SIZEOF_PFS_KEY_BLOB 0x140
#define CONTENT_KEY_SEED_SIZE 0x10
#define SELF_KEY_SEED_SIZE 0x10
#define EEKC_SIZE 0x20
#define MAX_FAKE_KEYS 32
#define SIZEOF_RSA_KEY 0x48
#define PFS_FAKE_OBF_KEY_ID 0x1337
#define SIZEOF_PFS_HEADER 0x5A0

struct fake_key_desc
{
	uint8_t key[0x20];
	int occupied;
};

struct fake_key_d
{
	uint32_t index;
	uint8_t seed[PFS_SEED_SIZE];
};

struct ekc
{
	uint8_t content_key_seed[CONTENT_KEY_SEED_SIZE];
	uint8_t self_key_seed[SELF_KEY_SEED_SIZE];
};

union pfs_key_blob {
	struct {
		uint8_t eekpfs[EEKPFS_SIZE];
		struct ekc eekc;
		uint32_t pubkey_ver; /* 0x1/0x80000001/0xC0000001 */
		uint32_t key_ver;    /* 1 (if (rif_ver_major & 0x1) != 0, then pfs_key_ver=1, otherwise pfs_key_ver=0) */
		uint64_t header_gva;
		uint32_t header_size;
		uint32_t type;
		uint32_t finalized;
		uint32_t is_disc;
	} in;
	struct {
		uint8_t escrowed_keys[0x40];
	} out;
};

typedef union pfs_key_blob pfs_key_blob_t;
//TYPE_CHECK_SIZE(pfs_key_blob_t, SIZEOF_PFS_KEY_BLOB);

struct rsa_buffer
{
	uint8_t* ptr;
	size_t size;
};

struct pfs_header
{
	uint8_t unknown00[0x370];
	uint8_t crypt_seed[0x10];
};
//TYPE_BEGIN(struct pfs_header, SIZEOF_PFS_HEADER);
//TYPE_FIELD(uint8_t crypt_seed[0x10], 0x370);
//TYPE_END();

#define SCE_SBL_ERROR_NPDRM_ENOTSUP 0x800F0A25
#define SIZEOF_SBL_KEY_RBTREE_ENTRY 0xA8 // sceSblKeymgrSetKey
#define SIZEOF_SBL_MAP_LIST_ENTRY 0x50 // sceSblDriverMapPages
#define TYPE_SBL_KEY_RBTREE_ENTRY_DESC_OFFSET 0x04
#define TYPE_SBL_KEY_RBTREE_ENTRY_LOCKED_OFFSET 0x80
#define SIZEOF_SBL_KEY_DESC 0x7C // sceSblKeymgrSetKey
#define SBL_MSG_SERVICE_MAILBOX_MAX_SIZE 0x80
#define SBL_MSG_CCP 0x8

struct sbl_mapped_page_group;

union sbl_key_desc {
	struct {
		uint16_t obf_key_id;
		uint16_t key_size;
		uint8_t escrowed_key[0x20];
	} pfs;
	struct {
		uint16_t cmd;
		uint16_t pad;
		uint16_t key_id;
	} portability;
	uint8_t raw[SIZEOF_SBL_KEY_DESC];
};
//TYPE_CHECK_SIZE(union sbl_key_desc, SIZEOF_SBL_KEY_DESC);

struct sbl_key_rbtree_entry
{
	uint32_t handle;						// 0x00
	union sbl_key_desc desc;				// 0x04
	uint8_t unknown08[0x78];				// 0x08
	uint32_t locked;						// 0x80
	uint8_t unknown84[4];					// 0x84
	struct sbl_key_rbtree_entry* left;		// 0x88
	struct sbl_key_rbtree_entry* right;		// 0x90
	struct sbl_key_rbtree_entry* parent;	// 0x98
	uint32_t set;							// 0xA0
};

struct sbl_map_list_entry
{
	struct sbl_map_list_entry* next;			// 0x00
	struct sbl_map_list_entry* prev;			// 0x08
	uint32_t cpu_va;							// 0x10
	uint8_t unknown14[0x4];						// 0x14
	uint32_t num_page_groups;					// 0x18
	uint8_t unknown1C[4];						// 0x1C
	uint32_t gpu_va;							// 0x20
	uint8_t unknown24[4];						// 0x24
	struct sbl_mapped_page_group* page_groups;	// 0x28
	uint32_t num_pages;							// 0x30
	uint8_t unknown34[4];						// 0x34
	uint32_t flags;								// 0x38
	uint8_t unknown3C[4];						// 0x3C
	struct proc* proc;							// 0x40
	void* vm_page;								// 0x48
};

//
//	RIF
//
#define RIF_DIGEST_SIZE 0x10
#define RIF_DATA_SIZE 0x90
#define RIF_KEY_TABLE_SIZE 0x230
#define RIF_MAX_KEY_SIZE 0x20
#define RIF_PAYLOAD_SIZE (RIF_DIGEST_SIZE + RIF_DATA_SIZE)
#define SIZEOF_ACTDAT 0x200
#define SIZEOF_RSA_KEY 0x48
#define SIZEOF_RIF 0x400

struct rif_key_blob
{
	struct ekc eekc;
	uint8_t entitlement_key[0x10];
};

union keymgr_response
{
	struct
	{
		uint32_t type;
		uint8_t key[RIF_MAX_KEY_SIZE];
		uint8_t data[RIF_DIGEST_SIZE + RIF_DATA_SIZE];
	} decrypt_rif;
	struct
	{
		uint8_t raw[SIZEOF_RIF];
	} decrypt_entire_rif;
};

union keymgr_payload
{
	struct
	{
		uint32_t cmd;
		uint32_t status;
		uint64_t data;
	};
	uint8_t buf[0x80];
};

struct rsa_key
{
	uint8_t unknown00[0x20];	// 0x00
	uint8_t* p;					// 0x20
	uint8_t* q;					// 0x28
	uint8_t* dmp1;				// 0x30
	uint8_t* dmq1;				// 0x38
	uint8_t* iqmp;				// 0x40
};

struct actdat
{
	uint32_t magic;								// 0x00
	uint16_t version_major;						// 0x04
	uint16_t version_minor;						// 0x06
	uint64_t account_id;						// 0x08
	uint64_t start_time;						// 0x10
	uint64_t end_time;							// 0x18
	uint64_t flags;								// 0x20
	uint8_t unk28[4];							// 0x28
	uint8_t unk2C[4];							// 0x2C
	uint8_t unk30[0x30];						// 0x30
	uint8_t open_psid_hash[0x20];				// 0x60
	uint8_t static_per_console_data_1[0x20];	// 0x80
	uint8_t digest[0x10];						// 0xA0
	uint8_t key_table[0x20];					// 0xB0
	uint8_t static_per_console_data_2[0x10];	// 0xD0
	uint8_t static_per_console_data_3[0x20];	// 0xE0
	uint8_t signature[0x100];					// 0x100
};

struct rif
{
	uint32_t magic;					// 0x00
	uint16_t version_major;			// 0x04
	uint16_t version_minor;			// 0x06
	uint64_t account_id;			// 0x08
	uint64_t start_time;			// 0x10
	uint64_t end_time;				// 0x18
	char content_id[0x30];			// 0x20
	uint16_t format;				// 0x50
	uint16_t drm_type;				// 0x52
	uint16_t content_type;			// 0x54
	uint16_t sku_flag;				// 0x56
	uint64_t content_flags;			// 0x58
	uint32_t iro_tag;				// 0x60
	uint32_t ekc_version;			// 0x64
	uint8_t unk68[2];				// 0x68
	uint8_t unk6A[2];				// 0x6A
	uint8_t unk6C[2];				// 0x6C
	uint8_t unk6E[0x1F2];			// 0x6E
	uint8_t digest[0x10];			// 0x260
	uint8_t data[RIF_DATA_SIZE];	// 0x270
	uint8_t signature[0x100];		// 0x300
};

union keymgr_request
{
	struct
	{
		uint32_t type;
		uint8_t key[RIF_MAX_KEY_SIZE];
		uint8_t data[RIF_DIGEST_SIZE + RIF_DATA_SIZE];
	} decrypt_rif;
	struct
	{
		struct rif rif;
		uint8_t key_table[RIF_KEY_TABLE_SIZE];
		uint64_t timestamp;
		int status;
	} decrypt_entire_rif;
};


#define CCP_MAX_PAYLOAD_SIZE 0x88
#define CCP_OP(cmd) (cmd >> 24)
#define CCP_OP_AES  0
#define CCP_OP_XTS  2
#define CCP_OP_HMAC 9
#define CCP_USE_KEY_FROM_SLOT    (1 << 18)
#define CCP_GENERATE_KEY_AT_SLOT (1 << 19)
#define CCP_USE_KEY_HANDLE       (1 << 20)

struct ccp_link
{
	void* p;
};

union ccp_op
{
	struct
	{
		uint32_t cmd;
		uint32_t status;
	} common;
	struct {
		uint32_t cmd;
		uint32_t status;
		uint64_t data_size;
		uint64_t in_data;
		uint64_t out_data;
		union {
			uint32_t key_index;
			uint8_t key[0x20];
		};
		uint8_t iv[0x10];
	} aes;
	uint8_t buf[CCP_MAX_PAYLOAD_SIZE];
};

struct ccp_msg
{
	union ccp_op op;
	uint32_t index;
	uint32_t result;
	TAILQ_ENTRY(ccp_msg) next;
	uint64_t message_id;
	LIST_ENTRY(ccp_link) links;
};

struct ccp_req
{
	TAILQ_HEAD(, ccp_msg) msgs;
	void(*cb)(void* arg, int result);
	void* arg;
	uint64_t message_id;
	LIST_ENTRY(ccp_link) links;
};

union sbl_msg_service {
	struct {
		union ccp_op op;
	} ccp;
};

struct sbl_msg_hdr {
	uint32_t cmd;
	uint32_t status;
	uint64_t message_id;
	uint64_t extended_msgs;
};

struct sbl_msg {
	struct sbl_msg_hdr hdr;
	union {
		union sbl_msg_service service;
		uint8_t raw[0x1000];
	};
};