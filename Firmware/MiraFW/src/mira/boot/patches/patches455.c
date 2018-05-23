#include <mira/boot/patches.h>
#include <oni/utils/kdlsym.h>
#include <oni/utils/cpu.h>

// Patches done by CrazyVoid
// Thanks to
// WildCard for helping with patches
// Joonie for helping with memcpy patch
// LightingMod for being a tester

void install_prerunPatches_455()
{

	if(!gKernelBase)
		return;
		
	void(*critical_enter)(void) = kdlsym(critical_enter);
	void(*critical_exit)(void) = kdlsym(critical_exit);
	
	critical_enter();
	cpu_disable_wp();
	
	// enable UART
	// Done by WildCard
	gKernelBase[0x1997BC8] = 0;
	
	// Verbose Panics patch
	// Done by WildCard
	uint8_t *kmem = (uint8_t *)&gKernelBase[0x3DBDC7];
	kmem[0] = 0x90; kmem[1] = 0x90; kmem[2] = 0x90; kmem[3] = 0x90;
	kmem[4] = 0x90;
	//kmem[5] = 0x65; kmem[6] = 0x8B; kmem[7] = 0x34;

	// Enable rwx - By WildCard
	kmem = (uint8_t*)&gKernelBase[0x16ed8c];
	kmem[0] = 0x07;
	kmem = (uint8_t*)&gKernelBase[0x16eda2];
	kmem[0] = 0x07;
	
	// Map Self Patch - IDC 
	gKernelBase[0x143BF2] = 0x90;
	gKernelBase[0x143BF3] = 0xE9;
	gKernelBase[0x143E0E] = 0x90;
	gKernelBase[0x143E0F] = 0x90;
	
	// Copyin Patch - Copyin Offset = (0x14A890)
	// Done by CrazyVoid
	gKernelBase[0x14A8E7] = 0x90;
	gKernelBase[0x14A8E7 + 1] = 0x90;

	// Copyout Patch  - Copyout offset = (0x14A7B0)
	// Done by CrazyVoid
	gKernelBase[0x14A802] = 0x90;
	gKernelBase[0x14A802 + 1] = 0x90;
	
	// Patch copyinstr - CrazyVoid
	gKernelBase[0x14AD53] = 0x90;
	gKernelBase[0x14AD53 + 1] = 0x90;
	gKernelBase[0x14AD83] = 0x90;
	gKernelBase[0x14AD83 + 1] = 0x90;
	
	
	// Patch memcpy stack - By CrazyVoid
	gKernelBase[0x14A6BD] = 0xEB;
	
	// ptrace patches - By WildCard
	gKernelBase[0x17D2EE] = 0x90;
	gKernelBase[0x17D2EE + 1] = 0x90;
	gKernelBase[0x17D2EE + 2] = 0x90;
	gKernelBase[0x17D2EE + 3] = 0x90;
	gKernelBase[0x17D2EE + 4] = 0x90;
	gKernelBase[0x17D2EE + 5] = 0x90;

	cpu_enable_wp();
	critical_exit();

}


