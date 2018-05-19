# Developers Guide

## Project Layout

At first glance, it may seem like the Mira-Project is very complicated, but once you understand the basic layout it is very clear and simple.

* `Tools/` - The tooling folder, which may contain additional libraries and tools to assist with Mira development. These tools are not required to build Mira, but will help developers.
* `Scripts/` - These are general puropse scripts that may get executed automatically by the build, or there for initializing development environments.
* `Output/` - When configured (by default it's not) will be the output directory for all builds
* `Firmware/` - This directory contains all the components to Mira's core
    * `Dependencies/` - The dependencies for the Mira project, usually just freebsd-headers, and oni-framework
    * `MiraFW/` - The actual source code for Mira, which handles auto-escalation and execution as a background kernel process
    * `Plugins/` - These are all of the *external* plugins that Mira will use. These can be built and compiled independently, and where all developers should create new projects in

## Setting up the development environment

In order to build Mira, you will need either a Windows machine with Visual Studio 2017, the C++ for Linux plugin and Windows Subsystem for Linux (highly reccomended), or a linux-based machine or VM with `build-essentials` (gcc, clang, etc).

### Windows Setup

This is the most reccomended way to develop for Mira. Since Mira is developed using Unix Makefiles, there is a direct compatibility with the Windows and Linux development environments.

1. [Download & Install](https://www.visualstudio.com/downloads/) Visual Studio 2017 WITH the [C++ for Linux support](https://blogs.msdn.microsoft.com/vcblog/2017/04/11/linux-development-with-c-in-visual-studio/)
2. [Enable](https://docs.microsoft.com/en-us/windows/wsl/install-win10) the Windows Subsystem for Linux
    1. You will need to configure the newly created WSL environment's ssh daemon.
    2. This can be done by editing `/etc/ssh/sshd_config` on most platforms.
        * You will need to change the port number to 2222 as Windows 10 may use port 22 for other reasons
3. Configure Windows Firewall to allow port 2222 on LAN (**_LAN ONLY OR LOCALHOST ONLY_**)
4. Configure Visual Studio's remote targets to use `localhost` port 2222 as the port number and your username/password created during the WSL install screen
    * If there's a red box error use `127.0.0.1` as the IP address 
5. Open the `Mira.sln` with Visual Studio 2017 and do a full rebuild, this should deploy the [freebsd-headers](https://github.com/OpenOrbis/freebsd-headers) and [modified oni-framework](https://github.com/OpenOrbis/oni-framework)

### Linux Setup

TODO: Implement Linux setup instructions

### Cloning the project

No matter if you are using Windows or Linux, these commands will be the same.

* If you are on Windows, you *MUST* clone inside of WSL and not within Windows itself
    * You can access all Windows partitions from within WSL by navigating to `/mnt/{windows drive letter}` (ex: `/mnt/c/Users/example/Desktop`)

1. Clone the repository
    * `git clone --recurse-submodules https://github.com/OpenOrbis/mira-project.git`
2. (Linux Users Only) Create all needed directories
    * Windows users using the Mira.sln, this will automatically get done for you when you do a Rebuild all on the solution.
    * `cd mira-project/Firmware/Dependencies/oni-framework`
    * `make create`
    * `cd ../../MiraFW`
    * `make create`

### Building the project

Build in this order
1. freebsd-headers
2. oni-framework
3. mira

TODO: Linux build instructions
