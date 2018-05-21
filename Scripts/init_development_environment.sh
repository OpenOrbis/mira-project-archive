#!/bin/bash
export BSD_INC=$(pwd)/Firmware/Dependencies/freebsd-headers/include
export ONI_FRAMEWORK=$(pwd)/Firmware/Dependencies/oni-framework
echo 'Making Oni..'
sh -c 'cd Firmware/Dependencies/oni-framework ; make create ; make'
echo 'Making Mira..'
sh -c 'cd Firmware/MiraFW ; make create ; make'
echo 'Completed.'
