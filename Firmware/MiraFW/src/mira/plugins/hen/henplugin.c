#include "henplugin.h"
#include <oni/utils/hook.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/ref.h>
#include <oni/utils/logger.h>
#include <oni/utils/memory/allocator.h>

#include <mira/miraframework.h>

#include <oni/utils/kernel.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_map.h>

#include <sys/proc.h>

#include <sys/ptrace.h>
#include <sys/mman.h>

#include <oni/utils/sys_wrappers.h>

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

	plugin->sceSblAuthMgrIsLoadable2Hook = NULL;
	plugin->sceSblAuthMgrVerifyHeaderHook = NULL;
	plugin->sceSblAuthMgrSmLoadSelfBlockHook = NULL;
	plugin->sceSblServiceMailboxHook = NULL;

	plugin->sceSblKeymgrSmCallfuncHook = NULL;
	plugin->sceSblDriverSendMsgHook = NULL;
	plugin->sceSblPfsSetKeysHook = NULL;
}

static struct henplugin_t* henPlugin = NULL;
struct henplugin_t* hen_getHenPlugin()
{
	return henPlugin;
}

struct proc* proc_find_by_name(const char* name) {
	int(*_sx_slock)(struct sx *sx, int opts, const char *file, int line) = kdlsym(_sx_slock);
	void(*_sx_sunlock)(struct sx *sx, const char *file, int line) = kdlsym(_sx_sunlock);
	void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);

	struct sx* allproclock = (struct sx*)kdlsym(allproc_lock);
	struct proclist* allproc = (struct proclist*)*(uint64_t*)kdlsym(allproc);

	int(*strcmp)(const char *str1, const char* str2) = kdlsym(strcmp);

	struct proc* p;

	if (!name)
		return NULL;

	_sx_slock(allproclock, 0, __FILE__, __LINE__);

	FOREACH_PROC_IN_SYSTEM(p) {
		PROC_LOCK(p);


		if (strcmp(p->p_comm, name) == 0) {
			PROC_UNLOCK(p);
			goto done;
		}

		PROC_UNLOCK(p);
	}

	p = NULL;

done:
	_sx_sunlock(allproclock, __FILE__, __LINE__);

	return p;
}

struct proc_vm_map_entry
{
	char name[32];
	vm_offset_t start;
	vm_offset_t end;
	vm_offset_t offset;
	uint16_t prot;
};

//static int proc_get_vm_map(struct proc *p, struct proc_vm_map_entry **entries, size_t *num_entries)
//{
//	void(*vmspace_free)(struct vmspace *) = kdlsym(vmspace_free);
//	struct vmspace* (*vmspace_acquire_ref)(struct proc *) = kdlsym(vmspace_acquire_ref);
//	void(*_vm_map_lock_read)(vm_map_t map, const char *file, int line) = kdlsym(_vm_map_lock_read);
//	void(*_vm_map_unlock_read)(vm_map_t map, const char *file, int line) = kdlsym(_vm_map_unlock_read);
//	//void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);
//	boolean_t(*vm_map_lookup_entry)(vm_map_t, vm_offset_t, vm_map_entry_t *) = kdlsym(vm_map_lookup_entry);
//
//	struct proc_vm_map_entry *info = NULL;
//	struct vm_map_entry *entry = NULL;
//
//	struct vmspace *vm = p->p_vmspace; // vmspace_acquire_ref(p);
//	if (!vm) {
//		WriteLog(LL_Error, "could not get vmspace");
//		return -1;
//	}
//
//	struct vm_map *map = &vm->vm_map;
//
//	int num = map->nentries;
//	if (!num) {
//		WriteLog(LL_Error, "there are no entries");
//		//vmspace_free(vm);
//		return 0;
//	}
//
//	vm_map_lock_read(map);
//
//	if (vm_map_lookup_entry(map, 0, &entry)) {
//		WriteLog(LL_Error, "could not look up map entry");
//		vm_map_unlock_read(map);
//		//vmspace_free(vm);
//		return -1;
//	}
//
//	info = (struct proc_vm_map_entry *)kmalloc(num * sizeof(struct proc_vm_map_entry));
//	if (!info) {
//		WriteLog(LL_Error, "could not allocate proc vm map entries");
//		vm_map_unlock_read(map);
//		//vmspace_free(vm);
//		return -1;
//	}
//
//	for (int i = 0; i < num; i++) {
//		info[i].start = entry->start;
//		info[i].end = entry->end;
//		info[i].offset = entry->offset;
//		info[i].prot = entry->protection & (entry->protection >> 8);
//		//memcpy(info[i].name, entry->name, sizeof(info[i].name));
//
//		if (!(entry = entry->next)) {
//			break;
//		}
//	}
//
//	vm_map_unlock_read(map);
//	//vmspace_free(vm);
//
//	if (entries) {
//		*entries = info;
//	}
//
//	if (num_entries) {
//		*num_entries = num;
//	}
//
//	return 0;
//}

int proc_get_vm_map(struct proc *p, struct proc_vm_map_entry **entries, uint64_t *num_entries) 
{
	boolean_t(*vm_map_lookup_entry)(vm_map_t, vm_offset_t, vm_map_entry_t *) = kdlsym(vm_map_lookup_entry);
	//void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);

	void(*_vm_map_lock_read)(vm_map_t map, const char *file, int line) = kdlsym(_vm_map_lock_read);
	void(*_vm_map_unlock_read)(vm_map_t map, const char *file, int line) = kdlsym(_vm_map_unlock_read);

	struct proc_vm_map_entry *info = NULL;
	struct vm_map_entry *entry = NULL;
	int ret = 0;

	struct vmspace *vm = p->p_vmspace;
	struct vm_map *map = &vm->vm_map;

	vm_map_lock_read(map);

	int num = map->nentries;
	if (!num) {
		goto error;
	}

	ret = vm_map_lookup_entry(map, NULL, &entry);
	if (ret) {
		goto error;
	}

	info = (struct proc_vm_map_entry *)kmalloc(num * sizeof(struct proc_vm_map_entry));
	if (!info) {
		ret = -1;
		goto error;
	}

	for (int i = 0; i < num; i++) {
		info[i].start = entry->start;
		info[i].end = entry->end;
		info[i].offset = entry->offset;
		info[i].prot = entry->protection & (entry->protection >> 8);
		//memcpy(info[i].name, entry->name, sizeof(info[i].name));

		if (!(entry = entry->next)) {
			break;
		}
	}

error:
	WriteLog(LL_Info, "ret: (%d).", ret);
	vm_map_unlock_read(map);

	if (entries) {
		*entries = info;
	}

	if (num_entries) {
		*num_entries = num;
	}

	/*static_assert(sizeof(struct vm_map_entry) == 0xC0);
	static_assert(offsetof(struct vm_map_entry, name) == 0x8D);*/

	//offsetof_ct(struct vm_map_entry, name);

	return 0;
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
	plugin->sceSblServiceMailboxHook = hook_create(kdlsym(sceSblServiceMailbox), hen_sceSblServiceMailbox);
	WriteLog(LL_Debug, "here");

	//
	// fpkg hooks
	//
	plugin->sceSblKeymgrSmCallfuncHook = hook_create(kdlsym(sceSblKeymgrSmCallfunc), hen_sceSblKeymgrSmCallfunc);
	plugin->sceSblPfsSetKeysHook = hook_create(kdlsym(sceSblPfsSetKeys), hen_sceSblPfsSetKeys);
	plugin->sceSblDriverSendMsgHook = hook_create(kdlsym(sceSblDriverSendMsg), hen_sceSblDriverSendMsg);
	
	WriteLog(LL_Debug, "here");

	// Enable all the hooks and hope for the best
	hook_enable(plugin->sceSblAuthMgrVerifyHeaderHook);
	hook_enable(plugin->sceSblAuthMgrIsLoadable2Hook);
	hook_enable(plugin->sceSblServiceMailboxHook);

	hook_enable(plugin->sceSblKeymgrSmCallfuncHook);
	hook_enable(plugin->sceSblPfsSetKeysHook);
	hook_enable(plugin->sceSblDriverSendMsgHook);

	uint8_t xor__eax_eax[] = { 0x31, 0xC0, 0x90, 0x90, 0x90 };

	//void(*_mtx_unlock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_unlock_flags);
	//void(*_mtx_lock_flags)(struct mtx *m, int opts, const char *file, int line) = kdlsym(_mtx_lock_flags);


	WriteLog(LL_Debug, "here");

	struct proc* shellCoreProc = proc_find_by_name("SceShellCore");
	if (!shellCoreProc)
	{
		WriteLog(LL_Error, "could not find shellcore");
		return false;
	}
	WriteLog(LL_Debug, "here");
	
	// Get the vm map
	struct proc_vm_map_entry* entries = NULL;
	size_t numEntries = 0;
	int32_t ret = proc_get_vm_map(shellCoreProc, &entries, &numEntries);
	if (ret < 0)
	{
		WriteLog(LL_Error, "could not get vm map (%d).", ret);
		return false;
	}

	if (!entries || numEntries == 0)
	{
		WriteLog(LL_Error, "entries: %p numEntries: %d", entries, numEntries);
		return false;
	}

	uint8_t* entryStart = NULL;
	for (int i = 0; i < numEntries; i++) {
		if (entries[i].prot == (PROT_READ | PROT_EXEC)) {
			entryStart = (uint8_t *)entries[i].start;
			break;
		}
	}

	WriteLog(LL_Debug, "here");
	if (!entryStart)
	{
		WriteLog(LL_Error, "Could not find entry start");
		kfree(entries, sizeof(*entries) * numEntries);
		return false;
	}

	WriteLog(LL_Debug, "here");

	size_t bytesWritten = 4;
	ret = proc_rw_mem(shellCoreProc, (void*)(entryStart + 0xEA7B67), 4, (void*)"free", &bytesWritten, true);
	if (ret < 0)
		WriteLog(LL_Error, "could not write fake->free (%d).", ret);
	WriteLog(LL_Debug, "here");

	static uint64_t offsets[] =
	{
		0x16D05B,
		0x79941B,
		0x7E5623,
		0x946D5B,
		0x16D087,
		0x23747B,
		0x799447,
		0x946D87
	};
	WriteLog(LL_Debug, "here");

	for (uint32_t i = 0; i < ARRAYSIZE(offsets); ++i)
	{
		bytesWritten = sizeof(xor__eax_eax);
		void* addr = (void*)(entryStart + offsets[i]);
		WriteLog(LL_Info, "addr: %p", addr);

		ret = proc_rw_mem(shellCoreProc, addr, sizeof(xor__eax_eax), (void*)xor__eax_eax, &bytesWritten, true);
		WriteLog(LL_Debug, "ret: %d", ret);
		if (ret < 0)
			WriteLog(LL_Error, "could not write %d (%d).", i, ret);
	}

	WriteLog(LL_Debug, "finished patching shellcore");

	return true;
}

uint8_t hen_unload(struct henplugin_t* plugin)
{
	if (!plugin)
		return false;

	return true;
}

int hen_sceSblServiceMailbox(unsigned long service_id, uint8_t* request, void* response)
{
#if ONI_PLATFORM >= ONI_PLATFORM_ORBIS_BSD_500
	struct self_context* ctx = NULL;
	__asm__("mov %%r14, %0" : "=r" (ctx));

	//register struct self_context* ctx __asm__("r14");
#else
	uint8_t* frame = (uint8_t*)__builtin_frame_address(1);
	struct self_context* ctx = *(struct self_context**)(frame - 0x100);
#endif

	struct henplugin_t* plugin = mira_getFramework()->henPlugin;
	if (!plugin)
	{
		WriteLog(LL_Error, "could not get the hen plugin reference");
		return ENOMEM;
	}

	int(*sceSblServiceMailbox)(unsigned long service_id, uint8_t* request, void* response) = hook_getFunctionAddress(plugin->sceSblServiceMailboxHook);

	// If we do not have a valid context, then bail early
	if (!ctx)
	{
		hook_disable(plugin->sceSblServiceMailboxHook);
		int32_t ret = sceSblServiceMailbox(service_id, request, response);
		hook_enable(plugin->sceSblServiceMailboxHook);

		return ret;
	}

	int is_unsigned = hen_isFakeSelf(ctx);
	if (is_unsigned)
	{
		*(int*)(request + 0x04) = 0;
		return 0;
	}

	// Call original functions
	hook_disable(plugin->sceSblServiceMailboxHook);
	int32_t ret = sceSblServiceMailbox(service_id, request, response);
	hook_enable(plugin->sceSblServiceMailboxHook);
	return ret;
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