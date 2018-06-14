#include "ctlrunpayload.h"
#include <sys/errno.h>

#include <oni/utils/memory/allocator.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/logger.h>

int ctlrunpayload_userland(struct thread* td, struct ctlrunpayload_t* payloadInfo);
int ctlrunpayload_kernel(struct thread* td, struct ctlrunpayload_t* payloadInfo);

int ctlrunpayload(struct thread* td, struct ctlrunpayload_t* payloadInfo)
{
	// Verify that our thread and payload information are valid
	if (!td || !payloadInfo)
		return -EINVAL;

	int(*copyin)(const void* uaddr, void* kaddr, size_t len) = kdlsym(copyin);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);

	// The incoming structure is userland memory, which if deref'ed causes a panic
	struct ctlrunpayload_t payloadData;
	memset(&payloadData, 0, sizeof(payloadData));
	
	// Copyin the new data
	copyin(payloadInfo, &payloadData, sizeof(payloadData));

	// Check the payload start and the payload size
	if (!payloadInfo->payloadStart || payloadInfo->payloadSize == 0)
	{
		WriteLog(LL_Error, "invalid payload data or size");
		return -ENOEXEC;
	}

	uint8_t* kernelPayload = (uint8_t*)kmalloc(payloadInfo->payloadSize);
	if (!kernelPayload)
	{
		WriteLog(LL_Error, "could not allocate kernel payload");
		return -ENOMEM;
	}
	memset(kernelPayload, 0, payloadInfo->payloadSize);

	// Copy the payload from userland memory
	copyin(payloadInfo->payloadStart, kernelPayload, payloadInfo->payloadSize);

	// Set our kernel start to the newly created kernel payload
	payloadData.payloadStart = kernelPayload;
	payloadData.payloadSize = payloadInfo->payloadSize;

	// TODO: Maximum size check?

	// Handle userland execution
	if (payloadInfo->executeAsUserland)
		return ctlrunpayload_userland(td, &payloadData);

	return ctlrunpayload_kernel(td, &payloadData);
}

int ctlrunpayload_userland(struct thread* td, struct ctlrunpayload_t* payloadInfo)
{
	// TODO: Support userland
	WriteLog(LL_Error, "userland execution not supported *yet*");
	return -ENOTCAPABLE;
}

int ctlrunpayload_kernel(struct thread* td, struct ctlrunpayload_t* payloadInfo)
{
	int(*kproc_create)(void(*func)(void*), void* arg, struct proc** newpp, int flags, int pages, const char* fmt, ...) = kdlsym(kproc_create);

	struct proc* createdProc = NULL;
	int result = kproc_create((void(*)(void*))payloadInfo->payloadStart, NULL, &createdProc, 0, 0, "rec_payload");
	
	WriteLog(LL_Info, "kproc_create returned %p %d", createdProc, result);

	// Return no error
	return 0;
}