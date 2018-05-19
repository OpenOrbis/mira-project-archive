# The Mira Project

The Mira Project is a set of Framework, Plugins, Tools, Payloads, that will gives you more power to control your jailbroken PS4 console.
It is a CFW framework for PS4, something like TaiHEN for PS Vita, It is not as robust we are used to back in PS3 days, so it can not be installed permenentaly through modifyed PUP file like what we used to do in PS3 for example Rebug CFW PUP, but it will gives you the CFW functionality once installed and run.
The Mira Project is the result of the hard work from the OpenOrbis team. We believe in quality over quantity releases. There are numerious components to the project and each one will be explained below.

## Components

Here is the list of components with a description of what they do

### Firmware

Previously known as MiraHEN, this has been expanded into a full expansion custom firmware.

### Tools

There are many tools included in the Mira project and they are listed out below.

#### newlib-ps4

This is the libc implementation ported for PS4. It's a pain in the ass to compile, just take the pre-built binaries seriously.

#### ld-ps4

This is the linker to be used with newlib-ps4 to create Orbis compatible ELF files, or you could use CrazyVoid's elfFixupTool.

#### MiraLib

This is the communications library that will be specialized for operation with the Mira firmware.

#### Open Orbis Store

Community driven store and application that will allow you to download new plugins, payloads, tweaks, trainers from a trusted centralized source without ever leaving the comfort of your console.

Inspired by vitaDB <3 Rin
#### PS4 Payload Sender

A simple Android application to send payloads to your PS4.

### Plugins

Mira offer two types of plugins, one is defualt plugins that will be embedded inside MiraFW it self (file transfer, log server, debugger), and the other type is external modules that can be loaded from internel/external HDD for example PS4 Linux Loader.

#### PS4 Linux Loader

A simple plugin that let you run the Linux kernel from a remote device without webkit.

## User Guide

The users guide is found at [USERS.md](https://github.com/OpenOrbis/mira-project/blob/master/USERS.md) yamsayin' check it out if you want instructions on how to build, install, and use the firmware once it has been installed on the console with any of the methods described within

## Developer Guide
You will need VS2017, with Linux plugin and WSL (Ubuntu on Windows) or real Linux machine or Linux VM at least to be able to build the current project.We will post more details about building process here shortly.

## Special Thanks To

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

* CTurT

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

* Anonymous Contributors (You know who you are)