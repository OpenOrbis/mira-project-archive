#include "device.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/sysent.h>

#include <oni/utils/logger.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/cpu.h>
#include <oni/utils/memory/allocator.h>
#include <oni/utils/hook.h>
#include <oni/utils/hook.h>

#include "ctls/ctlrunpayload.h"
#include "trap.h"

#if ONI_PLATFORM >= ONI_PLATFORM_ORBIS_BSD_501 && ONI_PLATFORM <= ONI_PLATFORM_ORBIS_BSD_505
#define kdlsym_addr_self_orbis_sysvec						 0x019bbcd0
#else
#error sysvec not set
#endif

int sys_jailbreak(struct thread* td, register void* unused);
uint8_t* gMiraPayload = NULL;
size_t gMiraPayloadSize = 0;
struct miraframework_t* gMiraFramework = NULL;

struct logger_t* gLogger = NULL;

// Device driver structure
static struct cdev *recovery_dev = NULL;
static struct cdevsw recovery_cdevsw = {
	.d_version = D_VERSION,
	.d_open = recovery_open,
	.d_close = recovery_close,
	.d_read = recovery_read,
	.d_write = recovery_write,
	.d_ioctl = recovery_ioctl,
	.d_name = "recovery",
};

void recovery_init_device()
{
	//void(*printf)(char*, ...) = kdlsym(printf);
	void(*kthread_exit)(void) = kdlsym(kthread_exit);
	void(*critical_enter)(void) = kdlsym(critical_enter);
	void(*critical_exit)(void) = kdlsym(critical_exit);
	struct sysentvec* sv = kdlsym(self_orbis_sysvec);
	void* (*memset)(void *s, int c, size_t n) = kdlsym(memset);
	void(*printf)(char *format, ...) = kdlsym(printf);
	void(*kthread_exit)(void) = kdlsym(kthread_exit);

	// If we already have a device created, don't allow re-registering
	if (recovery_dev)
		return;

	gLogger = (struct logger_t*)kmalloc(sizeof(struct logger_t));
	if (!gLogger)
	{
		printf("err: could not allocate logger\n");
		kthread_exit();
		return;
	}
	logger_init(gLogger);

	// Create our device
	int error = make_dev_p(MAKEDEV_CHECKNAME | MAKEDEV_WAITOK,
		&recovery_dev,
		&recovery_cdevsw,
		0,
		UID_ROOT,
		GID_WHEEL,
		0777,
		"rec");

	// Check to see if our device was installed successfully
	WriteLog(LL_Info, "make_dev_p returned %d", error);

	WriteLog(LL_Info, "installing auto-escape syscall");
	struct sysent* syscall = &sv->sv_table[8];

	critical_enter();
	cpu_disable_wp();
	memset(syscall, 0, sizeof(*syscall));
	syscall->sy_narg = 0; // No arguments
	syscall->sy_call = sys_jailbreak;

	cpu_enable_wp();
	critical_exit();

	// Set up the fatal hook trap
	gTrapQueen = hook_create(kdlsym(trap_fatal), trap_fatal_hook);
	if (gTrapQueen)
		hook_enable(gTrapQueen);

	WriteLog(LL_Info, "done installing device");

	kthread_exit();
}

void recovery_uninit_device()
{
	if (!recovery_dev)
		return;

	// Destroy the device
	//destroy_dev(recovery_dev);

	WriteLog(LL_Info, "recovery device destroyed");
}

// This is where we will take care of everything
int recovery_ioctl(struct cdev* dev, u_long cmd, caddr_t data, int fflag, struct thread* td)
{
	if (!dev || !td)
		return EINVAL;

	switch (cmd)
	{
	case CtlLoadFrameworkFromSocket:
		return ctlrunpayload(td, (struct ctlrunpayload_t*)data);
	case CtlEnableTrap:
		gTrappin = data ? TRUE : FALSE;
		WriteLog(LL_Info, "%s trap_fatal enabled", gTrappin ? "freebsd" : "mira");
		return 0;
	}
	return 0;
}

int
recovery_write(struct cdev *dev __unused, struct uio *uio, int ioflag __unused)
{
	// Ignored
	return 0;
}

int
recovery_read(struct cdev *dev __unused, struct uio *uio, int ioflag __unused)
{
	// Ignored
	return 0;
}

int
recovery_close(struct cdev *dev __unused, int fflag __unused, int devtype __unused,
	struct thread *td __unused)
{
	// Ignored
	return 0;
}

int recovery_open(struct cdev *dev __unused, int oflags __unused, int devtype __unused,
	struct thread *td __unused)
{
	// Ignored
	return 0;
}