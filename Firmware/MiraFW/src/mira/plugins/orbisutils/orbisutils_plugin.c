#include "orbisutils_plugin.h"
//
// Oni-Framework includes
//
#include <oni/config.h>
#include <oni/messaging/message.h>
#include <oni/messaging/messagemanager.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/logger.h>
#include <oni/utils/ref.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/cpu.h>

//
// Mira specific
//
#include <mira/miraframework.h>

struct utility_dumphddkeys_t
{
	uint8_t encrypted[0x60];
	uint8_t key[0x20];
};

struct orbisutils_toggleaslr_t
{
	uint8_t aslrEnabled;
};

uint8_t orbisutils_load(struct orbisutils_plugin_t* plugin);
uint8_t orbisutils_unload(struct orbisutils_plugin_t* plugin);

void orbisutils_dumpHddKeys_callback(struct ref_t* message);
void orbisutils_toggleASLR(struct ref_t* message);

enum UtilityCmds
{
	OrbisUtils_DumpHddKeys = 0xA5020F62,
	OrbisUtils_ToggleASLR = 0xE6572B02,
};


void orbisutils_init(struct orbisutils_plugin_t* plugin)
{
	if (!plugin)
		return;

	plugin->plugin.name = "OrbisUtils";
	plugin->plugin.description = "System utilities";
	plugin->plugin.plugin_load = (uint8_t(*)(void*))orbisutils_load;
	plugin->plugin.plugin_unload = (uint8_t(*)(void*))orbisutils_unload;
}

uint8_t orbisutils_load(struct orbisutils_plugin_t* plugin)
{
	if (!plugin)
		return false;

	messagemanager_registerCallback(gFramework->messageManager, RPCCAT_SYSTEM, OrbisUtils_DumpHddKeys, orbisutils_dumpHddKeys_callback);


	return true;
}

uint8_t orbisutils_unload(struct orbisutils_plugin_t* plugin)
{
	if (!plugin)
		return false;

	messagemanager_unregisterCallback(gFramework->messageManager, RPCCAT_SYSTEM, OrbisUtils_DumpHddKeys, orbisutils_dumpHddKeys_callback);

	return true;
}

void orbisutils_dumpHddKeys_callback(struct ref_t* reference)
{
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	if (!reference)
	{
		WriteLog(LL_Error, "invalid ref");
		return;
	}

	struct message_t* message = ref_getIncrement(reference);
	if (!message)
	{
		WriteLog(LL_Error, "invalid message");
		return;
	}

	if (message->header.request != true)
	{
		WriteLog(LL_Error, "response ignored");
		goto cleanup;
	}

	if (!message->payload)
	{
		WriteLog(LL_Error, "no payload");
  messagemanager_sendResponse(reference, -ENOMEM);
		goto cleanup;
	}

	 messagemanager_sendResponse(reference, 0);

	struct utility_dumphddkeys_t* request = (struct utility_dumphddkeys_t*)message->payload;

	// This is how we decrypt the EAP Internal partition key for usage with mounting on PC
	int(*sceSblGetEAPInternalPartitionKey)(unsigned char *encBuffer, unsigned char *decBzffer) = kdlsym(sceSblGetEAPInternalPartitionKey);

	memset(request->key, 0, sizeof(request->key));
	memset(request->encrypted, 0, sizeof(request->encrypted));

	int result = -1000;

#if ONI_PLATFORM>=ONI_PLATFORM_ORBIS_BSD_500
	// icc functions, 5.00+ does not have bank_id
	int(*icc_nvs_read)(uint64_t block_id, uint64_t offset, uint64_t size, uint8_t *data_ptr) = kdlsym(icc_nvs_read);
	result = icc_nvs_read(4, 0x200, 0x60, request->encrypted);
#else
	int(*icc_nvs_read)(uint64_t bank_id, uint64_t block_id, uint64_t offset, uint64_t size, uint8_t *data_ptr) = kdlsym(icc_nvs_read);
	result = icc_nvs_read(0, 4, 0x200, 0x60, request->encrypted);
#endif

	if (result < 0)
		WriteLog(LL_Debug, "icc_nvs_sread returned %d", result);

	result = sceSblGetEAPInternalPartitionKey(request->encrypted, request->key);
	WriteLog(LL_Debug, "sceSblGetEAPInternalPartitionKey returned %d", result);

	//kmemcpy(request->key, gKernelBase + 0x02790C90, 0x20);

	message->header.request = false;
	messagemanager_sendResponse(reference, 0);

cleanup:
	ref_release(reference);
}

void orbisutils_toggleASLR(struct ref_t* reference)
{
	if (!reference)
	{
		WriteLog(LL_Error, "invalid ref");
		return;
	}

	struct message_t* message = ref_getIncrement(reference);
	if (!message)
	{
		WriteLog(LL_Error, "invalid message");
		return;
	}

	if (message->header.request != true)
	{
		WriteLog(LL_Error, "response ignored");
		goto cleanup;
	}

	if (!message->payload)
	{
		WriteLog(LL_Error, "no payload");
  messagemanager_sendResponse(reference, -ENOMEM);
		goto cleanup;
	}

	struct orbisutils_toggleaslr_t* payload = (struct orbisutils_toggleaslr_t*)message->payload;

	void(*critical_enter)(void) = kdlsym(critical_enter);
	void(*critical_exit)(void) = kdlsym(critical_exit);

	WriteLog(LL_Debug, "%s ASLR for Userland Executables.", payload->aslrEnabled ? "Enabling" : "Disabling");

	critical_enter();
	cpu_disable_wp();

#if ONI_PLATFORM==ONI_PLATFORM_ORBIS_BSD_501
	* (uint8_t*)(gKernelBase + 0x00389555) = payload->aslrEnabled ? 0x74 : 0xEB;
#elif ONI_PLATFORM==ONI_PLATFORM_ORBIS_BSD_505
	* (uint8_t*)(gKernelBase + 0x00389925) = payload->aslrEnabled ? 0x74 : 0xEB;
#endif

	cpu_enable_wp();
	critical_exit();

cleanup:
	ref_release(reference);
}