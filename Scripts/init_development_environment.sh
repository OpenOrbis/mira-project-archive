#!/bin/bash

echo "Please choose a firmware:"
echo "\t[0] 5.01 Firmware"
echo "\t[1] 5.05 Firmware"
read -p "Option? " IFW

if [ "$IFW" = "0" ] || [ "$IFW" = "1" ]; then
	FW="5.01"

	ONI_PLATFORM="ONI_PLATFORM_ORBIS_BSD_501"

	if [ "$IFW" = "1" ]; then
		ONI_PLATFORM="ONI_PLATFORM_ORBIS_BSD_505"
		FW="5.05"
	fi

	echo "-----------------------------------------------"
	echo "Installing Mira with firmware '$FW' selected..."
	echo "-----------------------------------------------"
	
	cd ../Firmware/Dependencies/freebsd-headers/include

	export BSD_INC=$PWD

	cd ../../oni-framework/
	sed -i "1 i\#define ONI_PLATFORM $ONI_PLATFORM\n" ./include/oni/config.h
	make create
	make clean
	make

	cd ../../MiraFW
	make create
	make clean
	make

	echo "-----------------------------------------------"
	echo "Done!"
else
	echo "Invalid firmware selected."
fi
