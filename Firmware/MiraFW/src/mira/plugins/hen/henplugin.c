#include "henplugin.h"
#include <oni/utils/hook.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/logger.h>

#include <mira/miraframework.h>

static const uint8_t s_auth_info_for_exec[] = {
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x20,
	0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00,
	0x00, 0x40, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t s_auth_info_for_dynlib[] = {
	0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x30, 0x00, 0x30,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00,
	0x00, 0x40, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void henplugin_init(struct henplugin_t* plugin)
{
	if (!plugin)
		return;

	plugin->plugin.name = "hen";
	plugin->plugin.description = "homebrew enabler";

	plugin->plugin.plugin_load = (uint8_t(*)(void*)) hen_load;
	plugin->plugin.plugin_unload = (uint8_t(*)(void*)) hen_unload;
}

uint8_t hen_load(struct henplugin_t* plugin)
{
	if (!plugin)
		return false;

	//
	// fself hooks
	//
	plugin->sceSblAuthMgrVerifyHeaderHook = hook_create(kdlsym(sceSblAuthMgrVerifyHeader), hen_sceSblAuthMgrVerifyHeader);
	plugin->sceSblAuthMgrIsLoadable2Hook = hook_create(kdlsym(sceSblAuthMgrIsLoadable2), hen_sceSblAuthMgrIsLoadable2);

	//
	// fpkg hooks
	//

	return true;
}

uint8_t hen_unload(struct henplugin_t* plugin)
{
	if (!plugin)
		return false;

	return true;
}

int hen_sceSblAuthMgrVerifyHeader(struct self_context* ctx)
{
	void(*sceSblAuthMgrSmStart)() = kdlsym(sceSblAuthMgrSmStart);

	sceSblAuthMgrSmStart();

	return hen_authSelfHeader(ctx);
}

int hen_authSelfHeader(struct self_context* ctx)
{
	int(*sceSblAuthMgrSmVerifyHeader)(struct self_context* ctx) = kdlsym(sceSblAuthMgrSmVerifyHeader);

	void* mini_syscore_self_binary = kdlsym(mini_syscore_self_binary);
	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);

	struct henplugin_t* plugin = mira_getFramework()->henPlugin;
	if (!plugin)
	{
		WriteLog(LL_Error, "could not get the hen plugin reference");
		return ENOMEM;
	}

	uint8_t isUnsigned = ctx->format == SELF_FORMAT_ELF || hen_isFakeSelf(ctx);

	// If the self file is signed, then we just pass it through without modification
	if (!isUnsigned)
	{
		return sceSblAuthMgrSmVerifyHeader(ctx);
	}
		

	WriteLog(LL_Debug, "unsigned executable header detected");

	// Save the previous format and header size
	int32_t oldFormat = ctx->format;
	uint32_t oldTotalHeaderSize = ctx->totalHeaderSize;

	// Get the header of mini-syscore
	struct self_header* header = (struct self_header*)mini_syscore_self_binary;
	uint32_t newTotalHeaderSize = header->headerSize + header->metaSize;

	uint8_t* buffer = (uint8_t*)kmalloc(newTotalHeaderSize);
	if (!buffer)
	{
		WriteLog(LL_Error, "could not allocate temporary buffer");
		return ENOMEM;
	}

	// Backup the previous header
	memcpy(buffer, ctx->header, newTotalHeaderSize);

	// Write the mini_syscore_self_binary header
	memcpy(ctx->header, header, newTotalHeaderSize);

	// Now we are officially a SELF file
	ctx->format = SELF_FORMAT_SELF;
	ctx->totalHeaderSize = newTotalHeaderSize;

	// Call the original function
	int(*sceSblAuthMgrVerifyHeader)(struct self_context* ctx) = hook_getFunctionAddress(plugin->sceSblAuthMgrVerifyHeaderHook);
	
	hook_disable(plugin->sceSblAuthMgrVerifyHeaderHook);
	int32_t result = sceSblAuthMgrVerifyHeader(ctx);
	hook_enable(plugin->sceSblAuthMgrVerifyHeaderHook);

	// Restore everything
	memcpy(ctx->header, buffer, newTotalHeaderSize);
	ctx->format = oldFormat;
	ctx->totalHeaderSize = oldTotalHeaderSize;

	// Free the buffer we allocated
	kfree(buffer, newTotalHeaderSize);

	// Return the final result
	return result;
}

// Check to see if the current self context is a fake signed self
uint8_t hen_isFakeSelf(struct self_context* ctx)
{
	int(*_sceSblAuthMgrGetSelfInfo)(struct self_context* ctx, void *exInfo) = kdlsym(_sceSblAuthMgrGetSelfInfo);

	if (!ctx)
		return false;

	if (ctx->format != SELF_FORMAT_SELF)
		return false;

	struct self_ex_info* exInfo = NULL;
	int32_t ret = _sceSblAuthMgrGetSelfInfo(ctx, &exInfo);
	if (ret)
		return false;

	return exInfo->ptype == SELF_PTYPE_FAKE;
}

int hen_sceSblAuthMgrGetFakeSelfAuthInfo(struct self_context* ctx, struct self_auth_info* authInfo)
{
	int(*memcpy)(void *to, const void *from, size_t len) = kdlsym(memcpy);

	if (ctx->format != SELF_FORMAT_SELF)
		return -35;

	struct self_header* header = (struct self_header*)ctx->header;
	struct self_fake_auth_info* fakeInfo = (struct self_fake_auth_info*)(ctx->header + header->headerSize + header->metaSize - 0x100);
	if (fakeInfo->size != sizeof(fakeInfo->info))
		return -37;

	memcpy(authInfo, &fakeInfo->info, sizeof(*authInfo));
	return 0;
}

int hen_buildFakeSelfAuthInfo(struct self_context* ctx, struct self_auth_info* parentAuthInfo, struct self_auth_info* authInfo)
{
	int(*sceSblAuthMgrGetSelfInfo)(struct self_context* ctx, void *exInfo) = kdlsym(_sceSblAuthMgrGetSelfInfo);
	int(*memcpy)(void *to, const void *from, size_t len) = kdlsym(memcpy);

	if (!ctx || !parentAuthInfo || !authInfo)
		return EINVAL;

	if (!hen_isFakeSelf(ctx))
		return EINVAL;

	struct self_ex_info* exInfo = NULL;
	int32_t ret = sceSblAuthMgrGetSelfInfo(ctx, &exInfo);
	if (ret)
		return ret;

	Elf64_Ehdr* header = NULL;
	ret = hen_sceSblAuthMgrGetElfHeader(ctx, &header);
	if (ret)
		return ret;

	if (!header)
		return ESRCH;

	struct self_auth_info fakeAuthInfo;
	ret = hen_sceSblAuthMgrGetFakeSelfAuthInfo(ctx, &fakeAuthInfo);

	if (ret)
	{
		switch (header->e_type)
		{
		case ELF_ET_EXEC:
		case ELF_ET_SCE_EXEC:
		case ELF_ET_SCE_EXEC_ASLR:
			memcpy(&fakeAuthInfo, s_auth_info_for_exec, sizeof(fakeAuthInfo));
			ret = 0;
			break;
		case ELF_ET_SCE_DYNAMIC:
			memcpy(&fakeAuthInfo, s_auth_info_for_dynlib, sizeof(fakeAuthInfo));
			ret = 0;
			break;
		default:
			return ENOTSUP;
		}
	}

	if (authInfo)
		memcpy(authInfo, &fakeAuthInfo, sizeof(*authInfo));

	return ret;
}

int hen_sceSblAuthMgrIsLoadable2(struct self_context* ctx, struct self_auth_info* oldAuthInfo, int pathId, struct self_auth_info* newAuthInfo)
{
	struct henplugin_t* plugin = mira_getFramework()->henPlugin;
	if (!plugin)
	{
		WriteLog(LL_Error, "could not get the hen plugin reference");
		return ENOMEM;
	}


	int32_t ret = 0;
	if (ctx->format == SELF_FORMAT_ELF || hen_isFakeSelf(ctx))
		ret = hen_buildFakeSelfAuthInfo(ctx, oldAuthInfo, newAuthInfo);
	else
	{
		int(*sceSblAuthMgrIsLoadable2)(struct self_context*, struct self_auth_info*, int, struct self_auth_info*) = hook_getFunctionAddress(plugin->sceSblAuthMgrIsLoadable2Hook);
		hook_disable(plugin->sceSblAuthMgrIsLoadable2Hook);
		ret = sceSblAuthMgrIsLoadable2(ctx, oldAuthInfo, pathId, newAuthInfo);
		hook_enable(plugin->sceSblAuthMgrIsLoadable2Hook);
	}

	return ret;
}

int hen_sceSblAuthMgrGetElfHeader(struct self_context* ctx, Elf64_Ehdr** ehdr)
{
	if (ctx->format == SELF_FORMAT_ELF)
	{
		Elf64_Ehdr* header = (Elf64_Ehdr*)ctx->header;
		if (header)
			*ehdr = header;
		return 0;
	}
	else if (ctx->format == SELF_FORMAT_SELF)
	{
		struct self_header* selfHeader = (struct self_header*)ctx->header;

		size_t dataSize = selfHeader->headerSize - sizeof(struct self_entry) * selfHeader->numEntries - sizeof(struct self_header);
		if (dataSize >= sizeof(Elf64_Ehdr) && (dataSize & 0xF) == 0)
		{
			Elf64_Ehdr* elf_hdr = (Elf64_Ehdr*)((uint8_t*)selfHeader + sizeof(struct self_header) + sizeof(struct self_entry) * selfHeader->numEntries);
			if (ehdr)
				*ehdr = elf_hdr;
			return 0;
		}
	}

	return -35;
}