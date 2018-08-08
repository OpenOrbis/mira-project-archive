#include <mira/boot/patches.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/cpu.h>

void install_prerunPatches_505()
{
	// You must assign the kernel base pointer before anything is done
	if (!gKernelBase)
		return;

	void(*critical_enter)(void) = kdlsym(critical_enter);
	void(*critical_exit)(void) = kdlsym(critical_exit);

	// Apply patches
	critical_enter();
	cpu_disable_wp();
	
	// enable UART
  	gKernelBase[0x19ECEB0] = 0;

	// Verbose Panics
	uint8_t *kmem = (uint8_t *)&gKernelBase[0x00171580];
	kmem[0] = 0x90; kmem[1] = 0x90; kmem[2] = 0x90; kmem[3] = 0x90;
	kmem[4] = 0x90; kmem[5] = 0x65; kmem[6] = 0x8B; kmem[7] = 0x34;

	// sceSblACMgrIsAllowedSystemLevelDebugging
	kmem = (uint8_t *)&gKernelBase[0x00010FC0];
	kmem[0] = 0xB8; kmem[1] = 0x01; kmem[2] = 0x00; kmem[3] = 0x00;
	kmem[4] = 0x00; kmem[5] = 0xC3; kmem[6] = 0x90; kmem[7] = 0x90;

	kmem = (uint8_t *)&gKernelBase[0x00011756];
	kmem[0] = 0xB8; kmem[1] = 0x01; kmem[2] = 0x00; kmem[3] = 0x00;
	kmem[4] = 0x00; kmem[5] = 0xC3; kmem[6] = 0x90; kmem[7] = 0x90;

	// Enable rwx
	kmem = (uint8_t*)&gKernelBase[0x000FCD48];
	kmem[0] = 0x07;
	kmem = (uint8_t*)&gKernelBase[0x000FCD56];
	kmem[0] = 0x07;

	// Patch copy(in/out)
	uint16_t *copyinpatch = (uint16_t*)&gKernelBase[0x001EA767];
	uint16_t *copyoutpatch = (uint16_t*)&gKernelBase[0x001EA682];

	*copyinpatch = 0x9090;
	*copyoutpatch = 0x9090;

	// Enable MAP_SELF
	kmem = (uint8_t*)&gKernelBase[0x000117b0];
	kmem[0] = 0xB8;
	kmem[1] = 0x01;
	kmem[2] = 0x00;
	kmem[3] = 0x00;
	kmem[4] = 0x00;
	kmem[5] = 0xC3;
	kmem = (uint8_t*)&gKernelBase[0x117c0];
	kmem[0] = 0xB8;
	kmem[1] = 0x01;
	kmem[2] = 0x00;
	kmem[3] = 0x00;
	kmem[4] = 0x00;
	kmem[5] = 0xC3;
	kmem = (uint8_t*)&gKernelBase[0x0013F03F];
	kmem[0] = 0x31;
	kmem[1] = 0xC0;
	kmem[2] = 0x90;
	kmem[3] = 0x90;
	kmem[4] = 0x90;

	
        // Allow sys_dynlib_dlsym in all processes.
	gKernelBase[0x237F3B] = 0x01C1;

	// Don't restrict dynlib information.
	uint64_t *dynlib = (uint16_t*)&gKernelBase[0x2B2620];

	*dynlib = 0x9090909090C3C031;

	// Allow usage of mangled symbols in dynlib_do_dlsym().
	gKernelBase[0x2AFB47] = 0x9090;
	gKernelBase[0x2AFB47 + 2] = 0x9090;
	gKernelBase[0x2AFB47 + 4] = 0x9090;
	
	// Patch copyinstr
	gKernelBase[0x001EAB93] = 0x90;
	gKernelBase[0x001EAB93+1] = 0x90;

	gKernelBase[0x001EABC3] = 0x90;
	gKernelBase[0x001EABC3+1] = 0x90;

	// Patch memcpy stack
	gKernelBase[0x001EA53D] = 0xEB;

	// ptrace patches
	gKernelBase[0x0030D9C3] = 0x90;
	gKernelBase[0x0030D9C3 + 1] = 0x90;
	gKernelBase[0x0030D9C3 + 2] = 0x90;
	gKernelBase[0x0030D9C3 + 3] = 0x90;
	gKernelBase[0x0030D9C3 + 4] = 0x90;
	gKernelBase[0x0030D9C3 + 5] = 0x90;

	// setlogin patch (for autolaunch check)
        gKernelBase[0x5775C] = 0x48;
        gKernelBase[0x5775C + 1] = 0x31;
        gKernelBase[0x5775C + 2] = 0xC0;
        gKernelBase[0x5775C + 3] = 0x90;
	gKernelBase[0x5775C + 4] = 0x90;

	cpu_enable_wp();
	critical_exit();
}
