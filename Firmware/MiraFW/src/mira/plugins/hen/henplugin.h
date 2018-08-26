#pragma once
#include <oni/plugins/plugin.h>

#define LOCK_PROFILING
#include <sys/param.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/elf64.h>

struct hook_t;
struct self_auth_info;
struct self_auth_info;

struct henplugin_t
{
	struct plugin_t plugin;

	struct hook_t* sceSblAuthMgrIsLoadable2Hook;
	struct hook_t* sceSblAuthMgrVerifyHeaderHook;
	struct hook_t* sceSblAuthMgrSmLoadSelfBlockHook;
	struct hook_t* sceSblAuthMgrSmLoadSelfSegmentHook;

	struct hook_t* sceSblKeymgrSmCallfuncHook;
	struct hook_t* sceSblDriverSendMsgHook;
	struct hook_t* sceSblPfsSetKeysHook;
};
#define ELF_ET_EXEC          0x2
#define ELF_ET_SCE_EXEC      0xFE00
#define ELF_ET_SCE_EXEC_ASLR 0xFE10
#define ELF_ET_SCE_DYNAMIC   0xFE18

#define SELF_DIGEST_SIZE		0x20
#define SELF_CONTENT_ID_SIZE	0x13
#define SELF_RANDOM_PAD_SIZE	0x0D
#define SELF_MAX_HEADER_SIZE	0x4000

enum self_format
{
	SELF_FORMAT_NONE,
	SELF_FORMAT_ELF,
	SELF_FORMAT_SELF,
};

// Should be the same from 4.55-5.01
struct self_context
{
	enum self_format format;			// 0x0
	int32_t elfAuthType;				// 0x4
	uint32_t totalHeaderSize;			// 0x8
	uint8_t unkC[0x10];					// 0xC
	int32_t ctxId;						// 0x1C
	uint64_t svcId;						// 0x20
	uint8_t unk28[8];					// 0x28
	uint32_t bufId;						// 0x30
	uint8_t unk34[4];					// 0x34
	uint8_t* header;					// 0x38
	struct mtx lock;					// 0x40
};

// Should be the same from 4.55-5.01
struct self_entry
{
	uint64_t props;						// 0x0
	uint64_t offset;					// 0x8
	uint64_t fileSize;					// 0x10
	uint64_t memorySize;				// 0x18
};

#define SELF_PTYPE_FAKE			0x1
struct self_ex_info
{
	uint64_t paid;						// 0x0
	uint64_t ptype;						// 0x8
	uint64_t appVersion;				// 0x10
	uint64_t fwVersion;					// 0x18
	uint8_t digest[SELF_DIGEST_SIZE];	// 0x20
};

struct self_header
{
	uint32_t magic;						// 0x0
	uint8_t version;					// 0x4
	uint8_t mode;						// 0x5
	uint8_t endian;						// 0x6
	uint8_t attributes;					// 0x7
	uint32_t keyType;					// 0x8
	uint16_t headerSize;				// 0xC
	uint16_t metaSize;					// 0xE
	uint64_t fileSize;					// 0x10
	uint16_t numEntries;				// 0x18
	uint16_t flags;						// 0x1A
};

struct self_auth_info
{
	uint64_t paid;						// 0x0
	uint64_t caps[4];					// 0x8
	uint64_t attributes[4];				// 0x28
	uint8_t keyBlob[0x40];				// 0x48
};

struct self_fake_auth_info
{
	uint64_t size;						// 0x0
	struct self_auth_info info;			// 0x8
};

//
// Plugin functions
//
void henplugin_init(struct henplugin_t* plugin);
uint8_t hen_load(struct henplugin_t * plugin);
uint8_t hen_unload(struct henplugin_t * plugin);

//
// Hooked functions
//
// FSELF
int hen_sceSblAuthMgrIsLoadable2(struct self_context* ctx, struct self_auth_info* oldAuthInfo, int pathId, struct self_auth_info* newAuthInfo);
int hen_sceSblAuthMgrVerifyHeader(struct self_context* ctx);

// FPKG
union keymgr_payload;
struct pfs_header;
struct sbl_msg;
struct ekc;

int hen_sceSblKeymgrSmCallfunc(union keymgr_payload* payload);
int hen_sceSblPfsSetKeys(uint32_t* ekh, uint32_t* skh, uint8_t* eekpfs, struct ekc* eekc, unsigned int pubkey_ver, unsigned int key_ver, struct pfs_header* hdr, size_t hdr_size, unsigned int type, unsigned int finalized, unsigned int is_disc);
int hen_sceSblDriverSendMsg(struct sbl_msg* msg, size_t size);

//
// Utility functions
//
int hen_authSelfHeader(struct self_context* ctx);
int hen_buildFakeSelfAuthInfo(struct self_context* ctx, struct self_auth_info* parentAuthInfo, struct self_auth_info* authInfo);
int hen_sceSblAuthMgrGetFakeSelfAuthInfo(struct self_context* ctx, struct self_auth_info* authInfo);
int hen_sceSblAuthMgrGetElfHeader(struct self_context* ctx, Elf64_Ehdr** ehdr);


uint8_t hen_isFakeSelf(struct self_context* ctx);

extern struct henplugin_t* hen_getHenPlugin();