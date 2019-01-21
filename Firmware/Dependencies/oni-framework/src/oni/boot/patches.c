#include <oni/boot/patches.h>
#include <oni/utils/patches.h>
#include <oni/config.h>

void oni_installPrePatches()
{
	switch (ONI_PLATFORM)
	{
		case ONI_PLATFORM_ORBIS_BSD_405:
			install_prerunPatches_405();
			break;
		case ONI_PLATFORM_ORBIS_BSD_455:
			install_prerunPatches_455();
			break;
		case ONI_PLATFORM_ORBIS_BSD_474:
			install_prerunPatches_474();
			break;
		case ONI_PLATFORM_ORBIS_BSD_501:
			install_prerunPatches_501();
			break;
		case ONI_PLATFORM_ORBIS_BSD_505:
			install_prerunPatches_505();
			break;
		default:
			break;
	}
}
