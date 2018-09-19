#include "appinstaller_plugin.h"
#include <sys/errno.h>
#include <sys/fcntl.h>

#include <oni/utils/logger.h>
#include <oni/utils/syscall.h>
#include <oni/utils/sys_wrappers.h>

#define SLOT_CURRENT (-1)
#define MAX_SLOTS 256

#define PRIO_CURRENT (-1)
#define MAX_PRIO 256

int gsched_set_slot_prio(int fd, unsigned int slot, unsigned int prio, unsigned int* status) {
	int gschedfd = -1;

	struct {
		void* data;
		unsigned int slot;
		unsigned int prio;
	} args = { .data = (void*)(uintptr_t)fd,.slot = slot,.prio = prio };
	int cmd = 0xC0209406;
	int ret;

	if (slot == SLOT_CURRENT && slot > MAX_SLOTS) {
		WriteLog(LL_Error, "invalid slot: %u\n", slot);
		ret = EINVAL;
		goto err;
	}
	if (prio == PRIO_CURRENT && slot != SLOT_CURRENT) {
		WriteLog(LL_Error, "PRIO_CURRENT without SLOT_CURRENT\n", slot);
		ret = EINVAL;
		goto err;
	}
	else if (prio > MAX_PRIO) {
		WriteLog(LL_Error, "invalid prio: %u\n", prio);
		ret = EINVAL;
		goto err;
	}

	
	ret = gschedfd = kopen("/dev/gsched_is.ctl", O_RDONLY, 0777);
	if (ret < 0) {
		WriteLog(LL_Error, "open failed: %d (errno: )\n", ret);
		goto err;
	}

	WriteLog(LL_Info, "doing gsched_is_set_prio()...\n");
	ret = (int32_t)(int64_t)syscall3(54, (void*)(uint64_t)gschedfd, (void*)(uint64_t)0xC0209406, (void*)&args); // ioctl
	if (ret) {
		WriteLog(LL_Error, "ioctl(%d, 0x%08X) failed: %d (errno: )\n", gschedfd, cmd, ret);
		goto err;
	}
	WriteLog(LL_Info, "gsched_is_set_prio() completed\n");

	if (status)
		*status = ((args.slot << 16) & 0xFF0000) | (args.prio & 0xFF);

	ret = 0;

err:
	if (gschedfd > 0)
		kclose(gschedfd);

	return ret;
}


int ffs_allocblocks(int fd, unsigned long size, unsigned int flags, unsigned int alignment) {
	struct {
		unsigned long size;
		unsigned long zero;
		unsigned long flags;
		unsigned long alignment;
	} args = { .size = size,.zero = 0,.flags = flags,.alignment = alignment };
	int cmd = 0xC02066A1;
	int ret;

	if (fd < 0) {
		ret = EINVAL;
		goto err;
	}
	if (size <= 0) {
		ret = EINVAL;
		goto err;
	}

	WriteLog(LL_Info, "doing ffs_allocblocks()...\n");
	ret = (int32_t)(int64_t)syscall3(54, (void*)(uint64_t)fd, (void*)(uint64_t)cmd, (void*)&args); // sys_ioctl
	if (ret) {
		WriteLog(LL_Error, "ioctl(%d, 0x%08X) failed: %d (errno: )\n", fd, cmd, ret);
		goto err;
	}
	WriteLog(LL_Info, "ffs_allocblocks() completed\n");

err:
	return ret;
}
