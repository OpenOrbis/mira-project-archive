#include "orbisutils_plugin.h"
#include "orbisutils_messages.h"

//
// Oni-Framework includes
//
#include <oni/config.h>
#include <oni/messaging/messagemanager.h>
#include <oni/messaging/messagecontainer.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/logger.h>
#include <oni/utils/sys_wrappers.h>
#include <oni/utils/cpu.h>
#include <oni/rpc/rpcserver.h>
#include <oni/plugins/pluginmanager.h>

//
// Mira specific
//
#include <mira/miraframework.h>

enum OrbisUtilsCommands
{
	OrbisUtils_GetHddKeys = 0x0411E4B9,
	OrbisUtils_ShutdownMira = 0x7FDF47D5,
	OrbisUtils_SetAslr = 0x554BB3F1,
};

uint8_t orbisutils_load(struct orbisutils_plugin_t* plugin);
uint8_t orbisutils_unload(struct orbisutils_plugin_t* plugin);

void orbisutils_dumpHddKeys_callback(struct messagecontainer_t* container);
void orbisutils_toggleASLR(struct messagecontainer_t* container);
void orbisutils_shutdownMira_callback(struct messagecontainer_t* container);

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

	messagemanager_registerCallback(mira_getFramework()->framework.messageManager, MessageCategory_System, OrbisUtils_ShutdownMira, orbisutils_shutdownMira_callback);


	return true;
}

uint8_t orbisutils_unload(struct orbisutils_plugin_t* plugin)
{
	if (!plugin)
		return false;

	messagemanager_unregisterCallback(mira_getFramework()->framework.messageManager, MessageCategory_System, OrbisUtils_ShutdownMira, orbisutils_shutdownMira_callback);

	return true;
}

void orbisutils_shutdownMira_callback(struct messagecontainer_t* container)
{
	if (container == NULL)
		return;

	messagecontainer_acquire(container);

	if (container->size < sizeof(struct orbisutils_shutdownMiraRequest_t))
	{
		WriteLog(LL_Error, "malformed message");
		messagemanager_sendErrorResponse(MessageCategory_File, -ENOBUFS);
		goto cleanup;
	}

	struct orbisutils_shutdownMiraRequest_t* request = (struct orbisutils_shutdownMiraRequest_t*)container->payload;
	
	if (request->rebootConsole)
	{
		// TODO: call reboot
		kreboot(0);

		WriteLog(LL_Debug, "rebooting console");
		goto cleanup;
	}
	
	if (request->shutdownMira)
	{
		// Stop the RPC server
		WriteLog(LL_Info, "stopping RPC server.");
		if (!rpcserver_shutdown(mira_getFramework()->framework.rpcServer))
			WriteLog(LL_Error, "there was an error stopping the rpc server.");

		// Stop the klog server
		WriteLog(LL_Info, "Shutting down plugin manager");
		pluginmanager_shutdown(mira_getFramework()->framework.pluginManager);
		goto cleanup;
	}

cleanup:
	messagecontainer_release(container);
}

void orbisutils_dumpHddKeys_callback(struct messagecontainer_t* container)
{
//	// This is how we decrypt the EAP Internal partition key for usage with mounting on PC
//	//int(*sceSblGetEAPInternalPartitionKey)(unsigned char *encBuffer, unsigned char *decBzffer) = kdlsym(sceSblGetEAPInternalPartitionKey);
//	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
//	uint8_t* sbl_eap_internal_partition_key = kdlsym(sbl_eap_internal_partition_key);
//	void* (*memcpy)(void* dest, const void* src, size_t n) = kdlsym(memcpy);
//
//#if ONI_PLATFORM>=ONI_PLATFORM_ORBIS_BSD_500
//	// icc functions, 5.00+ does not have bank_id
//	int(*icc_nvs_read)(uint64_t block_id, uint64_t offset, uint64_t size, uint8_t *data_ptr) = kdlsym(icc_nvs_read);
//#else
//	int(*icc_nvs_read)(uint64_t bank_id, uint64_t block_id, uint64_t offset, uint64_t size, uint8_t *data_ptr) = kdlsym(icc_nvs_read);
//#endif
//
//	WriteLog(LL_Error, "here");
//
//	if (!reference)
//	{
//		WriteLog(LL_Error, "invalid ref");
//		return;
//	}
//
//	WriteLog(LL_Error, "here");
//
//	struct message_header_t* message = ref_getDataAndAcquire(reference);
//	if (!message)
//	{
//		WriteLog(LL_Error, "invalid message");
//		return;
//	}
//
//	WriteLog(LL_Error, "here");
//
//	// Verify that our reference has enough space for our payload
//	if (ref_getSize(reference) < sizeof(struct utility_dumphddkeys_t))
//	{
//		WriteLog(LL_Error, "not enough space to hold payload");
//		messagemanager_sendResponse(reference, -ENOMEM);
//		goto cleanup;
//	}
//
//	WriteLog(LL_Error, "here");
//
//	struct utility_dumphddkeys_t* request = message_getData(message);
//
//	// Zero our buffers
//	memset(request->key, 0, sizeof(request->key));
//	memset(request->encrypted, 0, sizeof(request->encrypted));
//
//	//memset(sbl_eap_internal_partition_key, 0, 0x60);
//	memcpy(request->encrypted, sbl_eap_internal_partition_key, 0x60);
//
//	WriteLog(LL_Error, "here");
//
//	int32_t result = -1; 
//#if ONI_PLATFORM>=ONI_PLATFORM_ORBIS_BSD_500
//	result = icc_nvs_read(4, 0x200, 0x60, request->encrypted);
//#else
//	result = icc_nvs_read(0, 4, 0x200, 0x60, request->encrypted);
//#endif
//
//	WriteLog(LL_Info, "icc_nvs_read returned %d", result);
//
//	// Check if there was an error returned
//	if (result < 0)
//	{
//		WriteLog(LL_Error, "icc_nvs_sread returned %d", result);
//		messagemanager_sendResponse(reference, result);
//		goto cleanup;
//	}
//		
//	WriteLog(LL_Error, "here");
//
//
//	// Get 'le keys, TODO: This returns some fucked up data, figure out why
//	//result = sceSblGetEAPInternalPartitionKey(request->encrypted, request->key);
//	//if (result < 0)
//	//{
//	//	WriteLog(LL_Error, "sceSblGetEAPInternalPartitionKey failed (%d).", result);
//	//	messagemanager_sendResponse(reference, result);
//	//	goto cleanup;
//	//}	
//
//	// Copy over the key, this is pre-decrypted
//	memcpy(request->key, (const void*)sbl_eap_internal_partition_key, 0x20);
//
//	WriteLog(LL_Error, "here");
//
//	messagemanager_sendResponse(reference, 0);
//
//cleanup:
//	ref_release(reference);
}

void orbisutils_toggleASLR(struct messagecontainer_t* container)
{
//	void(*critical_enter)(void) = kdlsym(critical_enter);
//	void(*critical_exit)(void) = kdlsym(critical_exit);
//
//	if (!reference)
//	{
//		WriteLog(LL_Error, "invalid ref");
//		return;
//	}
//
//	struct message_header_t* message = ref_getDataAndAcquire(reference);
//	if (!message)
//	{
//		WriteLog(LL_Error, "invalid message");
//		return;
//	}
//
//	// Verify that our reference has enough space for our payload
//	if (ref_getSize(reference) < sizeof(struct orbisutils_toggleaslr_t))
//	{
//		WriteLog(LL_Error, "not enough space to hold payload");
//		messagemanager_sendResponse(reference, -ENOMEM);
//		goto cleanup;
//	}
//
//	struct orbisutils_toggleaslr_t* payload = message_getData(message);
//
//	WriteLog(LL_Debug, "%s ASLR for Userland Executables.", payload->aslrEnabled ? "Enabling" : "Disabling");
//
//	critical_enter();
//	cpu_disable_wp();
//
//#if ONI_PLATFORM==ONI_PLATFORM_ORBIS_BSD_501
//	* (uint8_t*)(gKernelBase + 0x00389555) = payload->aslrEnabled ? 0x74 : 0xEB;
//#elif ONI_PLATFORM==ONI_PLATFORM_ORBIS_BSD_505
//	* (uint8_t*)(gKernelBase + 0x00389925) = payload->aslrEnabled ? 0x74 : 0xEB;
//#else
//#error ASLR Platform not supported
//#endif
//
//	cpu_enable_wp();
//	critical_exit();
//
//cleanup:
//	ref_release(reference);
}