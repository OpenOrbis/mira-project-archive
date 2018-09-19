# The Mira Project
![AppVeyor](https://ci.appveyor.com/api/projects/status/32r7s2skrgm9ubva?svg=true "Build Status")

The Mira Project is a set of tools that grants you more power and control over your jailbroken Playstation 4. It is the result of all the hard work by the OpenOrbis team.

It works differently to the custom firmware experience on Playstation 3, where CFW would be installed on the system via modified PUP files (e.g. Rebug), however once the framework is installed and ran it gives users the same functionality they were previously used to.

## WARNING

There are quite a few fake, scam OpenOrbis websites, youtube, twitter etc. Take note, ~~WE DO NOT HAVE A WEBSITE, OR ANY SOCIAL MEDIA. All information will be here on the GitHub repository ONLY!~~ We currently have a [OpenOrbis discord server](https://discord.gg/GQr8ydn) for those who want to help contribute and have no other way of communication. All other outlets about this project are fake, possibly contain viruses, and should be avoided at all costs. Once again, THIS GitHub repository is the only way to get official news, information, and releases about the Mira-Project. Discord bots will pull from this repository.

## Firmware

Formerly known as MiraHEN, this has now been developed into a full custom firmware. Thanks to everyone who helped Mira reach a 1.0 beta goal (May 20, 2018). There will be updated goals placed within the [GitHub issue tracker](https://github.com/OpenOrbis/mira-project/milestones) so the community can follow the internal progress, contribute and help provide the best platform possible for users.

## Tools

### [newlib-ps4](https://github.com/OpenOrbis/newlib)

This is the libc implementation ported for PS4. Check the repository for updates and more information. Currently is in development, and may not be ready to use.

### [lld-ps4](https://github.com/OpenOrbis/lld)

This is the linker to be used with newlib-ps4 to create Orbis compatible ELF files. Alternatively, you can use CrazyVoid's elfFixupTool. Currently in development, and may not ready to use.

### MiraLib

This is the communications library that will be specialized for operation with the Mira firmware.

### OpenOrbis Store

Community driven store and application that will allow you to download new plugins, payloads, tweaks, trainers from a trusted centralized source without ever leaving the comfort of your console.

Inspired by vitaDB <3 Rin

### [Mira Companion](https://github.com/OpenOrbis/mira-toolbox/tree/master/Mira-Companion)

An Android application to control MiraCFW and manage your PS4.

### [PS4 Payload Sender](https://github.com/valentinbreiz/PS4-Payload-Sender-Android)

A simple Android application to send payloads to your PS4.

## Plugins

The Mira Project offers two types of plugins: built-in plugins and external plugins. Default plugins are embedded inside Mira firmware and include file transfer, a log server and a debugger. External plugins can be loaded from internal/external HDD, for example PS4 Linux Loader.

### PS4 Linux Loader

A simple plugin that lets you run the Linux kernel from a remote device without webkit.

## User Guide

The users guide can be found at [USERS.md](https://github.com/OpenOrbis/mira-project/blob/master/USERS.md). This guide should be followed if you require instructions on how to build, install and use the firmware once it is installed on the console.

## Developer Guide

You will need VS2017, with Linux plugin and WSL (Ubuntu on Windows) or a physical Linux machine or Linux VM to be able to build the project. You may find the documentation for developers at [DEVELOPERS.md](https://github.com/OpenOrbis/mira-project/blob/master/DEVELOPERS.md)

Potential contributors: Please carefully read the [DEVELOPERS.md](https://github.com/OpenOrbis/mira-project/blob/master/DEVELOPERS.md) file, especially the section at the bottom for contribution - we have a formalized development pipeline now.

## Special Thanks and Friends

We want to give a special shout out to these people in no particular order.

* flatz
* SpecterDev
* EvilSperm
* Rogero
* Joonie
* AlexAltea
* Mistawes
* Abkarino
* qwertyoruiop
* CTurt
* Mathieulh
* Senaxx
* m0rph3us1987
* CrazyVoid
* xvortex
* bigboss
* ZeraTron
* xorloser
* AlAzif
* masterzorag
* fail0verflow
* idc
* valentinbreiz
* Anonymous Contributors (you know who you are)
