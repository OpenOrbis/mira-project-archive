#!/bin/bash

# Is the ONI_PLATFORM environment variable set
hasPlatform=true;

# The final ONI_PLATFORM string
oniPlatform="";

# Check to see if we are running within the scripts directory, if we are tell the user
executingDirectory=${PWD##*/}

if [ $executingDirectory == "Scripts" ] ; then
	echo "Run this from mira's root, not the script directory.";
	exit 1;
fi

# Check to see if the platform has been set
if [ -z "$ONI_PLATFORM" ] ; then
	echo 'ONI_PLATFORM environment variable not set';
	hasPlatform=false;
fi

# Debugging
echo "IsPlatformSet: $hasPlatform";

# Check to see if the platform environment variable is set
if [ $hasPlatform == false ]; then
	echo "Please choose a firmware:";
	echo "\t[0] 1.76 Firmware (Unsupported)";
	echo "\t[1] 3.55 Firmware (Unsupported)";
	echo "\t[2] 4.00 Firmware (Unsupported)";
	echo "\t[3] 4.05 Firmware (Unsupported)";
	echo "\t[4] 4.07 Firmware (Unsupported)";
	echo "\t[5] 4.55 Firmware (Community Supported)";
	echo "\t[6] 4.74 Firmware (Community Supported)";
	echo "\t[7] 5.01 Firmware (Deprecated)";
	echo "\t[8] 5.05 Firmware (Latest Supported)";
	echo "\t[9] 5.50 Firmware (Prototype)";
	echo "\t[10] 5.53 Firmware (Prototype)";
	echo "\t[11] 5.55 Firmware (Prototype)";
	read -p "Option? " firmwareVersionSelection;

	# Handle the firmware version selection
	case $firmwareVersionSelection in
		"0")
		oniPlatform="ONI_PLATFORM_ORBIS_BSD_176";
		;;
		"1")
		oniPlatform="ONI_PLATFORM_ORBIS_BSD_355";
		;;
		"2")
		oniPlatform="ONI_PLATFORM_ORBIS_BSD_400";
		;;
		"3")
		oniPlatform="ONI_PLATFORM_ORBIS_BSD_405";
		;;
		"4")
		oniPlatform="ONI_PLATFORM_ORBIS_BSD_407";
		;;
		"5")
		oniPlatform="ONI_PLATFORM_ORBIS_BSD_455";
		;;
		"6")
		oniPlatform="ONI_PLATFORM_ORBIS_BSD_474";
		;;
		"7")
		oniPlatform="ONI_PLATFORM_ORBIS_BSD_501";
		;;
		"8")
		oniPlatform="ONI_PLATFORM_ORBIS_BSD_505";
		;;
		"9")
		oniPlatform="ONI_PLATFORM_ORBIS_BSD_550";
		;;
		"10")
		oniPlatform="ONI_PLATFORM_ORBIS_BSD_553";
		;;
		"11")
		oniPlatform="ONI_PLATFORM_ORBIS_BSD_555";
		;;
	esac

	# Export this as our environment variable
	# echo "export ONI_PLATFORM=${oniPlatform}" >> ~/.bashrc;
	export ONI_PLATFORM=${oniPlatform}
	# source ~/.bashrc
fi

# Prompt the user that we are going to download and configure Mira
echo "-----------------------------------------------";
echo "Installing Mira with '$oniPlatform' selected...";
echo "-----------------------------------------------";

# Navigate to where the freebsd-headers include directory is and set the BSD_INC variable
cd ./Firmware/Dependencies/freebsd-headers/include;
export BSD_INC="$(pwd)";
# echo "export BSD_INC=\"$(pwd)\"" >> ~/.bashrc;
# . -c 'source ~/.bashrc'

# Next nagivate into the oni-framework dependency
cd ../../oni-framework;
export ONI_FRAMEWORK="$(pwd)";

# Create the default directories needed for oni-framework
make create;
make clean;
scan-build make;
if [ $? -eq 0 ]; then
    echo OK
else
    echo FAIL
	exit 1
fi

# Navigate to the Mira core directory
cd ../../MiraFW;
make create;
make clean;
scan-build make;
if [ $? -eq 0 ]; then
    echo OK
else
    echo FAIL
	exit 1
fi

# Navigate to the Mira Loader directory
cd ../MiraLoader;
make create;
make clean;
scan-build make;
if [ $? -eq 0 ]; then
    echo OK
else
    echo FAIL
	exit 1
fi

echo "-----------------------------------------------";
echo "Done!";
exit 0;
