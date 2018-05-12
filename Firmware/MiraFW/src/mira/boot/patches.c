#include "patches.h"
#include <oni/config.h>

void mira_installPrePatches()
{
	switch (ONI_PLATFORM)
	{
	case ONI_PLATFORM_ORBIS_BSD_501:
		//install_prerunPatches_501();
		break;

	// 4.55 and 5.05 need to be ported
	case ONI_PLATFORM_ORBIS_BSD_455:
	case ONI_PLATFORM_ORBIS_BSD_505:
	default:
		break;
	}
}