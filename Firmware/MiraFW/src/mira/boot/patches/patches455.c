#include <mira/boot/patches.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/cpu.h>

void install_prerunPatches_455()
{
	// You must assign the kernel base pointer before anything is done
	if (!gKernelBase)
		return;

	void(*critical_enter)(void) = kdlsym(critical_enter);
	void(*crtical_exit)(void) = kdlsym(critical_exit);

	// Apply patches
	critical_enter();
	cpu_disable_wp();

  	// enable UART
	gKernelBase[0x1997BC8] = 0;
	
	// Verbose Panics
//	uint8_t *kmem = (uint8_t *)&gKernelBase[0x171517];
//	kmem[0] = 0x90; kmem[1] = 0x90; kmem[2] = 0x90; kmem[3] = 0x90;
//	kmem[4] = 0x90; kmem[5] = 0x65; kmem[6] = 0x8B; kmem[7] = 0x34;
//	kmem = (uint8_t *)&gKernelBase[0x11730];
	//kmem[0] = 0xB8; kmem[1] = 0x01; kmem[2] = 0x00; kmem[3] = 0x00;
	//kmem[4] = 0x00; kmem[5] = 0xC3; kmem[6] = 0x90; kmem[7] = 0x90;
	//kmem = (uint8_t *)&gKernelBase[0x11750];
	//kmem[0] = 0xB8; kmem[1] = 0x01; kmem[2] = 0x00; kmem[3] = 0x00;
	//kmem[4] = 0x00; kmem[5] = 0xC3; kmem[6] = 0x90; kmem[7] = 0x90;

	// Enable rwx
	//kmem = (uint8_t*)&gKernelBase[0xFCC38];
	//kmem[0] = 0x07;
	//kmem = (uint8_t*)&gKernelBase[0xFCC46];
//	kmem[0] = 0x07;

	// Patch copy(in/out) ?
	uint16_t *copyinpatch = (uint16_t*)&gKernelBase[0x14A890];
	uint16_t *copyoutpatch = (uint16_t*)&gKernelBase[0x14A7B0];

	// Enable MAP_SELF
	kmem = (uint8_t*)&gKernelBase[0x143BF2];
	kmem[0] = 0x90;
	kmem[1] = 0xE9;
	kmem[2] = 0x00;
	kmem[3] = 0x00;
	kmem[4] = 0x00;
	kmem[5] = 0x00;
	kmem = (uint8_t*)&gKernelBase[0x143E0E];
	kmem[0] = 0x90;
	kmem[1] = 0x90;
	kmem[2] = 0x00;
	kmem[3] = 0x00;
	kmem[4] = 0x00;
	kmem[5] = 0x00;

	// Patch copyinstr
	gKernelBase[0x14AD00] = 0x90;
  gKernelBase[0x14AD01] = 0x90;
  
	gKernelBase[0x14AD02] = 0x90;
	gKernelBase[0x14AD03] = 0x90;

	// Patch memcpy stack
	//gKernelBase[0x001EA42D] = 0xEB;

	// ptrace patches
	gKernelBase[0x17D2C1] = 0xEB;
	gKernelBase[0x17D2C1 + 1] = 0xEB;
	gKernelBase[0x17D2C1 + 2] = 0xEB;
	gKernelBase[0x17D2C1 + 3] = 0xEB;
	gKernelBase[0x17D2C1 + 4] = 0xEB;
	gKernelBase[0x17D2C1 + 5] = 0xEB;
	

	cpu_enable_wp();
	crtical_exit();
}
