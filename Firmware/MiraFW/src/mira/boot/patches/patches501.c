#include <mira/boot/patches.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/cpu.h>

void install_prerunPatches_501()
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
	gKernelBase[0x09ECAE0] = 0;
	
	// Verbose Panics
	uint8_t *kmem = (uint8_t *)&gKernelBase[0x171517];
	kmem[0] = 0x90; kmem[1] = 0x90; kmem[2] = 0x90; kmem[3] = 0x90;
	kmem[4] = 0x90; kmem[5] = 0x65; kmem[6] = 0x8B; kmem[7] = 0x34;
	kmem = (uint8_t *)&gKernelBase[0x11730];
	kmem[0] = 0xB8; kmem[1] = 0x01; kmem[2] = 0x00; kmem[3] = 0x00;
	kmem[4] = 0x00; kmem[5] = 0xC3; kmem[6] = 0x90; kmem[7] = 0x90;
	kmem = (uint8_t *)&gKernelBase[0x11750];
	kmem[0] = 0xB8; kmem[1] = 0x01; kmem[2] = 0x00; kmem[3] = 0x00;
	kmem[4] = 0x00; kmem[5] = 0xC3; kmem[6] = 0x90; kmem[7] = 0x90;

	// Enable rwx
	kmem = (uint8_t*)&gKernelBase[0xFCC38];
	kmem[0] = 0x07;
	kmem = (uint8_t*)&gKernelBase[0xFCC46];
	kmem[0] = 0x07;

	// Patch copy(in/out) ?
	uint16_t *copyinpatch = (uint16_t*)&gKernelBase[0x1EA657];
	uint16_t *copyoutpatch = (uint16_t*)&gKernelBase[0x1EA572];

	*copyinpatch = 0x9090;
	*copyoutpatch = 0x9090;
	
	
        // Allow sys_dynlib_dlsym in all processes. 
	gKernelBase[0x237E2A] = 0x01C1; 

	// Don't restrict dynlib information. 
	uint64_t *dynlib = (uint16_t*)&gKernelBase[0x2B2350];

	*dynlib = 0x9090909090C3C031;

	// Allow usage of mangled symbols in dynlib_do_dlsym(). 
	gKernelBase[0x2AF877] = 0x9090; 
	gKernelBase[0x2AF877 + 2] = 0x9090; 
	gKernelBase[0x2AF877 + 4] = 0x9090; 

	// Enable MAP_SELF
	kmem = (uint8_t*)&gKernelBase[0x117b0];
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
	kmem = (uint8_t*)&gKernelBase[0x13ef2f];
	kmem[0] = 0x31;
	kmem[1] = 0xC0;
	kmem[2] = 0x90;
	kmem[3] = 0x90;
	kmem[4] = 0x90;

	// Patch copyinstr
	gKernelBase[0x001EAA83] = 0x90;
	gKernelBase[0x001EAA84] = 0x90;

	gKernelBase[0x001EAAB3] = 0x90;
	gKernelBase[0x001EAAB4] = 0x90;

	// Patch memcpy stack
	gKernelBase[0x001EA42D] = 0xEB;

	// ptrace patches
	gKernelBase[0x0030D633] = 0x90;
	gKernelBase[0x0030D633 + 1] = 0x90;
	gKernelBase[0x0030D633 + 2] = 0x90;
	gKernelBase[0x0030D633 + 3] = 0x90;
	gKernelBase[0x0030D633 + 4] = 0x90;
	gKernelBase[0x0030D633 + 5] = 0x90;
	
	// setlogin patch (for autolaunch check)
	gKernelBase[0x5775C] = 0x48;
	gKernelBase[0x5775C + 1] = 0x31;
	gKernelBase[0x5775C + 2] = 0xC0;
	gKernelBase[0x5775C + 3] = 0x90;
	gKernelBase[0x5775C + 4] = 0x90;

	cpu_enable_wp();
	critical_exit();
}
