#include "henplugin_pkg.h"
#include <oni/utils/kdlsym.h>
#include <oni/utils/hook.h>
#include <oni/utils/logger.h>

static const uint8_t s_ypkg_p[0x80] =
{
	0x2D, 0xE8, 0xB4, 0x65, 0xBE, 0x05, 0x78, 0x6A, 0x89, 0x31, 0xC9, 0x5A, 0x44, 0xDE, 0x50, 0xC1,
	0xC7, 0xFD, 0x9D, 0x3E, 0x21, 0x42, 0x17, 0x40, 0x79, 0xF9, 0xC9, 0x41, 0xC1, 0xFC, 0xD7, 0x0F,
	0x34, 0x76, 0xA3, 0xE2, 0xC0, 0x1B, 0x5A, 0x20, 0x0F, 0xAF, 0x2F, 0x52, 0xCD, 0x83, 0x34, 0x72,
	0xAF, 0xB3, 0x12, 0x33, 0x21, 0x2C, 0x20, 0xB0, 0xC6, 0xA0, 0x2D, 0xB1, 0x59, 0xE3, 0xA7, 0xB0,
	0x4E, 0x1C, 0x4C, 0x5B, 0x5F, 0x10, 0x9A, 0x50, 0x18, 0xCC, 0x86, 0x79, 0x25, 0xFF, 0x10, 0x02,
	0x8F, 0x90, 0x03, 0xA9, 0x37, 0xBA, 0xF2, 0x1C, 0x13, 0xCC, 0x09, 0x45, 0x15, 0xB8, 0x55, 0x74,
	0x0A, 0x28, 0x24, 0x04, 0xD1, 0x19, 0xAB, 0xB3, 0xCA, 0x44, 0xB6, 0xF8, 0x3D, 0xB1, 0x2A, 0x72,
	0x88, 0x35, 0xE4, 0x86, 0x6B, 0x55, 0x47, 0x08, 0x25, 0x16, 0xAB, 0x69, 0x1D, 0xBF, 0xF6, 0xFE,
};

static const uint8_t s_ypkg_q[0x80] =
{
	0x23, 0x80, 0x77, 0x84, 0x4D, 0x6F, 0x9B, 0x24, 0x51, 0xFE, 0x2A, 0x6B, 0x28, 0x80, 0xA1, 0x9E,
	0xBD, 0x6D, 0x18, 0xCA, 0x8D, 0x7D, 0x9E, 0x79, 0x5A, 0xE0, 0xB8, 0xEB, 0xD1, 0x3D, 0xF3, 0xD9,
	0x02, 0x90, 0x2A, 0xA7, 0xB5, 0x7E, 0x9A, 0xA2, 0xD7, 0x2F, 0x21, 0xA8, 0x50, 0x7D, 0x8C, 0xA1,
	0x91, 0x2F, 0xBF, 0x97, 0xBE, 0x92, 0xC2, 0xC1, 0x0D, 0x8C, 0x0C, 0x1F, 0xDE, 0x31, 0x35, 0x15,
	0x39, 0x90, 0xCC, 0x97, 0x47, 0x2E, 0x7F, 0x09, 0xE9, 0xC3, 0x9C, 0xCE, 0x91, 0xB2, 0xC8, 0x58,
	0x76, 0xE8, 0x70, 0x1D, 0x72, 0x5F, 0x4A, 0xE6, 0xAA, 0x36, 0x22, 0x94, 0xC6, 0x52, 0x90, 0xB3,
	0x9F, 0x9B, 0xF0, 0xEF, 0x57, 0x8E, 0x53, 0xC3, 0xE3, 0x30, 0xC9, 0xD7, 0xB0, 0x3A, 0x0C, 0x79,
	0x1B, 0x97, 0xA8, 0xD4, 0x81, 0x22, 0xD2, 0xB0, 0x82, 0x62, 0x7D, 0x00, 0x58, 0x47, 0x9E, 0xC7,
};

static const uint8_t s_ypkg_dmp1[0x80] =
{
	0x25, 0x54, 0xDB, 0xFD, 0x86, 0x45, 0x97, 0x9A, 0x1E, 0x17, 0xF0, 0xE3, 0xA5, 0x92, 0x0F, 0x12,
	0x2A, 0x5C, 0x4C, 0xA6, 0xA5, 0xCF, 0x7F, 0xE8, 0x5B, 0xF3, 0x65, 0x1A, 0xC8, 0xCF, 0x9B, 0xB9,
	0x2A, 0xC9, 0x90, 0x5D, 0xD4, 0x08, 0xCF, 0xF6, 0x03, 0x5A, 0x5A, 0xFC, 0x9E, 0xB6, 0xDB, 0x11,
	0xED, 0xE2, 0x3D, 0x62, 0xC1, 0xFC, 0x88, 0x5D, 0x97, 0xAC, 0x31, 0x2D, 0xC3, 0x15, 0xAD, 0x70,
	0x05, 0xBE, 0xA0, 0x5A, 0xE6, 0x34, 0x9C, 0x44, 0x78, 0x2B, 0xE5, 0xFE, 0x38, 0x56, 0xD4, 0x68,
	0x83, 0x13, 0xA4, 0xE6, 0xFA, 0xD2, 0x9C, 0xAB, 0xAC, 0x89, 0x5F, 0x10, 0x8F, 0x75, 0x6F, 0x04,
	0xBC, 0xAE, 0xB9, 0xBC, 0xB7, 0x1D, 0x42, 0xFA, 0x4E, 0x94, 0x1F, 0xB4, 0x0A, 0x27, 0x9C, 0x6B,
	0xAB, 0xC7, 0xD2, 0xEB, 0x27, 0x42, 0x52, 0x29, 0x41, 0xC8, 0x25, 0x40, 0x54, 0xE0, 0x48, 0x6D,
};

static const uint8_t s_ypkg_dmq1[0x80] =
{
	0x4D, 0x35, 0x67, 0x38, 0xBC, 0x90, 0x3E, 0x3B, 0xAA, 0x6C, 0xBC, 0xF2, 0xEB, 0x9E, 0x45, 0xD2,
	0x09, 0x2F, 0xCA, 0x3A, 0x9C, 0x02, 0x36, 0xAD, 0x2E, 0xC1, 0xB1, 0xB2, 0x6D, 0x7C, 0x1F, 0x6B,
	0xA1, 0x8F, 0x62, 0x20, 0x8C, 0xD6, 0x6C, 0x36, 0xD6, 0x5A, 0x54, 0x9E, 0x30, 0xA9, 0xA8, 0x25,
	0x3D, 0x94, 0x12, 0x3E, 0x0D, 0x16, 0x1B, 0xF0, 0x86, 0x42, 0x72, 0xE0, 0xD6, 0x9C, 0x39, 0x68,
	0xDB, 0x11, 0x80, 0x96, 0x18, 0x2B, 0x71, 0x41, 0x48, 0x78, 0xE8, 0x17, 0x8B, 0x7D, 0x00, 0x1F,
	0x16, 0x68, 0xD2, 0x75, 0x97, 0xB5, 0xE0, 0xF2, 0x6D, 0x0C, 0x75, 0xAC, 0x16, 0xD9, 0xD5, 0xB1,
	0xB5, 0x8B, 0xE8, 0xD0, 0xBF, 0xA7, 0x1F, 0x61, 0x5B, 0x08, 0xF8, 0x68, 0xE7, 0xF0, 0xD1, 0xBC,
	0x39, 0x60, 0xBF, 0x55, 0x9C, 0x7C, 0x20, 0x30, 0xE8, 0x50, 0x28, 0x44, 0x02, 0xCE, 0x51, 0x2A,
};

static const uint8_t s_ypkg_iqmp[0x80] =
{
	0xF5, 0x73, 0xB8, 0x7E, 0x5C, 0x98, 0x7C, 0x87, 0x67, 0xF1, 0xDA, 0xAE, 0xA0, 0xF9, 0x4B, 0xAB,
	0x77, 0xD8, 0xCE, 0x64, 0x6A, 0xC1, 0x4F, 0xA6, 0x9B, 0xB9, 0xAA, 0xCC, 0x76, 0x09, 0xA4, 0x3F,
	0xB9, 0xFA, 0xF5, 0x62, 0x84, 0x0A, 0xB8, 0x49, 0x02, 0xDF, 0x9E, 0xC4, 0x1A, 0x37, 0xD3, 0x56,
	0x0D, 0xA4, 0x6E, 0x15, 0x07, 0x15, 0xA0, 0x8D, 0x97, 0x9D, 0x92, 0x20, 0x43, 0x52, 0xC3, 0xB2,
	0xFD, 0xF7, 0xD3, 0xF3, 0x69, 0xA2, 0x28, 0x4F, 0x62, 0x6F, 0x80, 0x40, 0x5F, 0x3B, 0x80, 0x1E,
	0x5E, 0x38, 0x0D, 0x8B, 0x56, 0xA8, 0x56, 0x58, 0xD8, 0xD9, 0x6F, 0xEA, 0x12, 0x2A, 0x40, 0x16,
	0xC1, 0xED, 0x3D, 0x27, 0x16, 0xA0, 0x63, 0x97, 0x61, 0x39, 0x55, 0xCC, 0x8A, 0x05, 0xFA, 0x08,
	0x28, 0xFD, 0x55, 0x56, 0x31, 0x94, 0x65, 0x05, 0xE7, 0xD3, 0x57, 0x6C, 0x0D, 0x1C, 0x67, 0x0B,
};

static const uint8_t rif_debug_key[0x10] =
{
	0x96, 0xC2, 0x26, 0x8D, 0x69, 0x26, 0x1C, 0x8B, 0x1E, 0x3B, 0x6B, 0xFF, 0x2F, 0xE0, 0x4E, 0x12
};

// we mark our key using some pattern that we can check later
static const uint8_t s_fake_key_seed[0x10] =
{
	0x46, 0x41, 0x4B, 0x45, 0x46, 0x41, 0x4B, 0x45, 0x46, 0x41, 0x4B, 0x45, 0x46, 0x41, 0x4B, 0x45,
};

//
//	a common function to generate a final key for PFS
//
static inline void pfs_gen_crypto_key(uint8_t* ekpfs, uint8_t seed[PFS_SEED_SIZE], unsigned int index, uint8_t key[PFS_FINAL_KEY_SIZE])
{
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void* fpu_ctx = kdlsym(fpu_ctx);
	void(*Sha256Hmac)(uint8_t hash[0x20], const uint8_t* data, size_t data_size, const uint8_t* key, int key_size) = kdlsym(Sha256Hmac);
	int(*fpu_kern_enter)(struct thread *td, void *ctx, uint32_t flags) = kdlsym(fpu_kern_enter);
	int(*fpu_kern_leave)(struct thread *td, void *ctx) = kdlsym(fpu_kern_leave);

	struct thread* td = curthread;

	// Allocate and set the fake key id
	struct fake_key_d d;
	memset(&d, 0, sizeof(d));
	d.index = index;
	memcpy(d.seed, seed, PFS_SEED_SIZE);

	// Calculate the hmac
	fpu_kern_enter(td, fpu_ctx, 0);
	Sha256Hmac(key, (uint8_t *)&d, sizeof(d), ekpfs, EKPFS_SIZE);
	fpu_kern_leave(td, fpu_ctx);
}

static inline void pfs_generate_enc_key(uint8_t* ekpfs, uint8_t seed[PFS_SEED_SIZE], uint8_t key[PFS_FINAL_KEY_SIZE])
{
	pfs_gen_crypto_key(ekpfs, seed, 1, key);
}

static inline void pfs_generate_sign_key(uint8_t* ekpfs, uint8_t seed[PFS_SEED_SIZE], uint8_t key[PFS_FINAL_KEY_SIZE])
{
	pfs_gen_crypto_key(ekpfs, seed, 2, key);
}

static inline int npdrm_decrypt_debug_rif(unsigned int type, uint8_t* data)
{
	void* fpu_ctx = kdlsym(fpu_ctx);
	int(*fpu_kern_enter)(struct thread *td, void *ctx, uint32_t flags) = kdlsym(fpu_kern_enter);
	int(*fpu_kern_leave)(struct thread *td, void *ctx) = kdlsym(fpu_kern_leave);
	int(*AesCbcCfb128Decrypt)(uint8_t* out, const uint8_t* in, size_t data_size, const uint8_t* key, int key_size, uint8_t* iv) = kdlsym(AesCbcCfb128Decrypt);

	struct thread* td = curthread;
	int ret = 0;

	fpu_kern_enter(td, fpu_ctx, 0);

	// decrypt fake rif manually using a key from publishing tools 
	ret = AesCbcCfb128Decrypt(data + RIF_DIGEST_SIZE, data + RIF_DIGEST_SIZE, RIF_DATA_SIZE, rif_debug_key, sizeof(rif_debug_key) * 8, data);
	if (ret)
		ret = SCE_SBL_ERROR_NPDRM_ENOTSUP;

	fpu_kern_leave(td, fpu_ctx);

	return ret;
}

static inline struct sbl_map_list_entry* sceSblDriverFindMappedPageListByGpuVa(vm_offset_t gpu_va)
{
	struct sbl_map_list_entry *entry = kdlsym(gpu_va_page_list);

	if (!gpu_va)
		return NULL;

	while (entry)
	{
		if (entry->gpu_va == gpu_va)
			return entry;

		entry = entry->next;
	}

	return NULL;
}

static inline vm_offset_t sceSblDriverGpuVaToCpuVa(vm_offset_t gpu_va, size_t* num_page_groups)
{
	struct sbl_map_list_entry* entry = sceSblDriverFindMappedPageListByGpuVa(gpu_va);
	if (!entry)
	{
		return 0;
	}
	if (num_page_groups)
	{
		*num_page_groups = entry->num_page_groups;
	}
	return entry->cpu_va;
}

int hen_sceSblDriverSendMsg(struct sbl_msg* msg, size_t size)
{
	//WriteLog(LL_Debug, "here");

	struct henplugin_t* plugin = hen_getHenPlugin();
	if (!plugin)
	{
		WriteLog(LL_Error, "could not get hen plugin");
		return -1;
	}

	if (!plugin->sceSblDriverSendMsgHook)
	{
		WriteLog(LL_Error, "hook is invalid");
		return -1;
	}

	int(*sceSblDriverSendMsg)(struct sbl_msg* msg, size_t size) = hook_getFunctionAddress(plugin->sceSblDriverSendMsgHook);

	int ret = 0;

	// If we do not have a valid message
	if (!msg)
	{
		WriteLog(LL_Error, "msg is invalid");
		goto call_orig;
	}

	// Something
	if (msg->hdr.cmd == 8)
	{
		union ccp_op* op = NULL;
		unsigned int cmd_mask = 0;
		size_t key_len = 0;
		size_t i = 0;

		if (msg->hdr.cmd != SBL_MSG_CCP)
			goto done;

		op = &msg->service.ccp.op;
		if (CCP_OP(op->common.cmd) != CCP_OP_AES)
			goto done;

		cmd_mask = CCP_USE_KEY_FROM_SLOT | CCP_GENERATE_KEY_AT_SLOT;
		if ((op->aes.cmd & cmd_mask) != cmd_mask || (op->aes.key_index != PFS_FAKE_OBF_KEY_ID))
			goto done;

		op->aes.cmd &= ~CCP_USE_KEY_FROM_SLOT;

		key_len = 16;

		/* reverse key bytes */
		for (i = 0; i < key_len; ++i)
			op->aes.key[i] = s_fake_key_seed[key_len - i - 1];

	done:
		hook_disable(plugin->sceSblDriverSendMsgHook);
		ret = sceSblDriverSendMsg(msg, size);
		hook_enable(plugin->sceSblDriverSendMsgHook);

		return ret;
	}


call_orig:
	hook_disable(plugin->sceSblDriverSendMsgHook);
	ret = sceSblDriverSendMsg(msg, size);
	hook_enable(plugin->sceSblDriverSendMsgHook);

	return ret;
}

int hen_sceSblPfsSetKeys(uint32_t* ekh, uint32_t* skh, uint8_t* eekpfs, struct ekc* eekc, unsigned int pubkey_ver, unsigned int key_ver, struct pfs_header* hdr, size_t hdr_size, unsigned int type, unsigned int finalized, unsigned int is_disc)
{
	WriteLog(LL_Debug, "here");

	struct henplugin_t* plugin = hen_getHenPlugin();

	struct sx* sbl_pfs_sx = kdlsym(sbl_pfs_sx);
	void* fpu_ctx = kdlsym(fpu_ctx);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	int(*RsaesPkcs1v15Dec2048CRT)(struct rsa_buffer* out, struct rsa_buffer* in, struct rsa_key* key) = kdlsym(RsaesPkcs1v15Dec2048CRT);
	int(*sceSblPfsSetKeys)(uint32_t* ekh, uint32_t* skh, uint8_t* eekpfs, struct ekc* eekc, unsigned int pubkey_ver, unsigned int key_ver, struct pfs_header* hdr, size_t hdr_size, unsigned int type, unsigned int finalized, unsigned int is_disc) = kdlsym(sceSblPfsSetKeys);
	int(*sx_xunlock)(struct sx *sx) = kdlsym(_sx_xunlock);
	int(*fpu_kern_enter)(struct thread *td, void *ctx, uint32_t flags) = kdlsym(fpu_kern_enter);
	int(*fpu_kern_leave)(struct thread *td, void *ctx) = kdlsym(fpu_kern_leave);
	int(*sceSblKeymgrSetKeyForPfs)(union sbl_key_desc* key, unsigned int* handle) = kdlsym(sceSblKeymgrSetKeyForPfs);
	int(*AesCbcCfb128Encrypt)(uint8_t* out, const uint8_t* in, size_t data_size, const uint8_t* key, int key_size, uint8_t* iv) = kdlsym(AesCbcCfb128Encrypt);
	int(*sceSblKeymgrClearKey)(uint32_t kh) = kdlsym(sceSblKeymgrClearKey);
	int(*sx_xlock)(struct sx *sx, int opts) = kdlsym(_sx_xlock);

	struct thread* td;
	struct rsa_buffer in_data;
	struct rsa_buffer out_data;
	struct rsa_key key;
	uint8_t ekpfs[EKPFS_SIZE];
	uint8_t iv[16];
	union sbl_key_desc enc_key_desc;
	union sbl_key_desc sign_key_desc;
	int orig_ret, ret;

	hook_disable(plugin->sceSblPfsSetKeysHook);
	ret = orig_ret = sceSblPfsSetKeys(ekh, skh, eekpfs, eekc, pubkey_ver, key_ver, hdr, hdr_size, type, finalized, is_disc);
	hook_enable(plugin->sceSblPfsSetKeysHook);

	if (ret) {
		if (!finalized) {
			memset(&in_data, 0, sizeof(in_data));
			in_data.ptr = eekpfs;
			in_data.size = EEKPFS_SIZE;

			memset(&out_data, 0, sizeof(out_data));
			out_data.ptr = ekpfs;
			out_data.size = EKPFS_SIZE;

			memset(&key, 0, sizeof(key));
			key.p = (uint8_t*)s_ypkg_p;
			key.q = (uint8_t*)s_ypkg_q;
			key.dmp1 = (uint8_t*)s_ypkg_dmp1;
			key.dmq1 = (uint8_t*)s_ypkg_dmq1;
			key.iqmp = (uint8_t*)s_ypkg_iqmp;

			td = curthread;

			fpu_kern_enter(td, fpu_ctx, 0);
			{
				ret = RsaesPkcs1v15Dec2048CRT(&out_data, &in_data, &key);
			}
			fpu_kern_leave(td, fpu_ctx);

			if (ret) {
				ret = orig_ret;
				goto err;
			}

			sx_xlock(sbl_pfs_sx, 0);
			{
				memset(&enc_key_desc, 0, sizeof(enc_key_desc));
				{
					enc_key_desc.pfs.obf_key_id = PFS_FAKE_OBF_KEY_ID;
					enc_key_desc.pfs.key_size = sizeof(enc_key_desc.pfs.escrowed_key);

					pfs_generate_enc_key(ekpfs, hdr->crypt_seed, enc_key_desc.pfs.escrowed_key);

					fpu_kern_enter(td, fpu_ctx, 0);
					{
						memset(iv, 0, sizeof(iv));
						ret = AesCbcCfb128Encrypt(enc_key_desc.pfs.escrowed_key, enc_key_desc.pfs.escrowed_key, sizeof(enc_key_desc.pfs.escrowed_key), s_fake_key_seed, sizeof(s_fake_key_seed) * 8, iv);
					}
					fpu_kern_leave(td, fpu_ctx);
				}
				if (ret) {
					sx_xunlock(sbl_pfs_sx);
					ret = orig_ret;
					goto err;
				}

				memset(&sign_key_desc, 0, sizeof(sign_key_desc));
				{
					sign_key_desc.pfs.obf_key_id = PFS_FAKE_OBF_KEY_ID;
					sign_key_desc.pfs.key_size = sizeof(sign_key_desc.pfs.escrowed_key);

					pfs_generate_sign_key(ekpfs, hdr->crypt_seed, sign_key_desc.pfs.escrowed_key);

					fpu_kern_enter(td, fpu_ctx, 0);
					{
						memset(iv, 0, sizeof(iv));
						ret = AesCbcCfb128Encrypt(sign_key_desc.pfs.escrowed_key, sign_key_desc.pfs.escrowed_key, sizeof(sign_key_desc.pfs.escrowed_key), s_fake_key_seed, sizeof(s_fake_key_seed) * 8, iv);
					}
					fpu_kern_leave(td, fpu_ctx);
				}
				if (ret) {
					sx_xunlock(sbl_pfs_sx);
					ret = orig_ret;
					goto err;
				}

				ret = sceSblKeymgrSetKeyForPfs(&enc_key_desc, ekh);
				if (ret) {
					if (*ekh != -1)
						sceSblKeymgrClearKey(*ekh);
					sx_xunlock(sbl_pfs_sx);
					ret = orig_ret;
					goto err;
				}

				ret = sceSblKeymgrSetKeyForPfs(&sign_key_desc, skh);
				if (ret) {
					if (*skh != -1)
						sceSblKeymgrClearKey(*skh);
					sx_xunlock(sbl_pfs_sx);
					ret = orig_ret;
					goto err;
				}
			}
			sx_xunlock(sbl_pfs_sx);

			ret = 0;
		}
	}
err:
	return ret;
}

int hen_sceSblKeymgrSmCallfunc(union keymgr_payload* payload)
{
	WriteLog(LL_Debug, "here");

	struct henplugin_t* plugin = hen_getHenPlugin();
	
	int(*sceSblKeymgrSmCallfunc)(union keymgr_payload* payload) = hook_getFunctionAddress(plugin->sceSblKeymgrSmCallfuncHook);

	// npdrm_decrypt_isolated_rif = 0x303
	if (payload->cmd == 0x303)
	{
		WriteLog(LL_Debug, "npdrm_decrypt_isolated_rif called");
		//int(*sceSblKeymgrSmCallfunc)(union keymgr_payload* payload) = kdlsym(sceSblKeymgrSmCallfunc);

		// it's SM request, thus we have the GPU address here, so we need to convert it to the CPU address
		union keymgr_request* request = (union keymgr_request*)sceSblDriverGpuVaToCpuVa(payload->data, NULL);
		int ret;

		// try to decrypt rif normally 
		hook_disable(plugin->sceSblKeymgrSmCallfuncHook);
		ret = sceSblKeymgrSmCallfunc(payload);
		hook_enable(plugin->sceSblKeymgrSmCallfuncHook);

		// and if it fails then we check if it's fake rif and try to decrypt it by ourselves 
		if ((ret != 0 || payload->status != 0) && request)
		{
			if (request->decrypt_rif.type == 0x200)
			{ // fake?
				ret = npdrm_decrypt_debug_rif(request->decrypt_rif.type, request->decrypt_rif.data);
				payload->status = ret;
				ret = 0;
			}
		}
		return ret;
	}
	else if (payload->cmd == 0x307) // npdrm_decrypt_rif_new = 0x307
	{
		WriteLog(LL_Debug, "npdrm_decrypt_rif_new called");
		void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);
		void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

		uint64_t buf_gpu_va = payload->data;

		// it's SM request, thus we have the GPU address here, so we need to convert it to the CPU address 
		union keymgr_request* request = (union keymgr_request*)sceSblDriverGpuVaToCpuVa(buf_gpu_va, NULL);
		union keymgr_response* response = (union keymgr_response*)request;
		int orig_ret, ret;

		// try to decrypt rif normally
		hook_disable(plugin->sceSblKeymgrSmCallfuncHook);
		ret = orig_ret = sceSblKeymgrSmCallfunc(payload);
		hook_enable(plugin->sceSblKeymgrSmCallfuncHook);

		// and if it fails then we check if it's fake rif and try to decrypt it by ourselves
		if ((ret != 0 || payload->status != 0) && request)
		{
			if (request->decrypt_entire_rif.rif.format != 2)
			{ // not fake?
				ret = orig_ret;
				goto err;
			}

			ret = npdrm_decrypt_debug_rif(request->decrypt_entire_rif.rif.format, request->decrypt_entire_rif.rif.digest);

			if (ret)
			{
				ret = orig_ret;
				goto err;
			}

			/* XXX: sorry, i'm lazy to refactor this crappy code :D basically, we're copying decrypted data to proper place,
			consult with kernel code if offsets needs to be changed */
			memcpy(response->decrypt_entire_rif.raw, request->decrypt_entire_rif.rif.digest, sizeof(request->decrypt_entire_rif.rif.digest) + sizeof(request->decrypt_entire_rif.rif.data));

			memset(response->decrypt_entire_rif.raw +
				sizeof(request->decrypt_entire_rif.rif.digest) +
				sizeof(request->decrypt_entire_rif.rif.data),
				0,
				sizeof(response->decrypt_entire_rif.raw) -
				(sizeof(request->decrypt_entire_rif.rif.digest) +
					sizeof(request->decrypt_entire_rif.rif.data)));

			payload->status = ret;
			ret = 0;
		}

	err:
		return ret;
	}
	else // Any other type
	{
		hook_disable(plugin->sceSblKeymgrSmCallfuncHook);
		int ret = sceSblKeymgrSmCallfunc(payload);
		hook_enable(plugin->sceSblKeymgrSmCallfuncHook);

		return ret;
	}
}