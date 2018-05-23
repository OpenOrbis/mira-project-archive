# Mira Users Guide

This user guide will help you build, install and use the Mira Framework on your Playstation 4 console.

## How to install

You can install Mira firmware via several methods. You should choose the method that works best for you. Typically, this will be via a webkit exploit in the web browser.

### Webkit exploit (non-permanent installation)

This involves using a webkit userland exploit to elevate the kernel to install MiraCFW. The changes will be lost upon reboot, but should survive sleep mode.

### Permanent installation

There is no permanent install option at this time, nor will one be offered in the near future.

#### Preparing

TODO: Instructions on how to prepare

#### Installing

In order to install Mira you will need a few things...

1. An exploitable PlayStation
2. Support within Mira
3. Way to send payloads
    1. Windows
    2. Linux
    3. Android

Start by downloading the latest release of Mira from the github Releases page. If there are no current releases for your current firmware, then you will need to compile the payloads from source code. If this is the case, refer to the developers documentation.

* We are also working towards preparing full stack webpage loaders in the near future for those who don't want to send payloads themselves.

##### WSL / Linux

After the payload has either been downloaded from the Releases page on github, or compiled from source. You can send the payload to a console at "waiting for payloads" by using the following command.

`nc {IP ADDRESS OF DEVICE} 9020 < ~/mira/Firmware/MiraFW/MiraFW_Orbis.bin`

Provided you are using the default build paths of Mira.

##### Windows

Currently there are no tools provided by the Mira-Project that will send payloads to the console. You may use your favorite tool for sending TCP payloads and it should work the same.

#### Using MiraCFW

Once Mira has been installed, it will automatically self-elevate, and check by default in the `/user/mira` folder for `config.ini` which contains the rest of the configuration for Mira. It will load plugins from the `/user/mira/plugins` directory and initialize them.

Once Mira has been installed, there will be a notification prompting you to which ports have been opened on the device. You can now read the kernel log by connecting to the IP address if your PS4 on port `9998`, and access the RPC interface on the console via `9999`.

#### Using MiraLib

Currently there is no application or tools that support using MiraLib, there will be tools avaiable in the future.

#### PS4 Payload Sender

To send payloads from your Android device without webkit, you can follow [these instructions](https://github.com/valentinbreiz/PS4-Payload-Sender-Android/blob/master/DOCUMENTATION.md).

### Plugins

Mira supports 2 types of plugins which both use the same framework.

* Internal / integrated plugins
* External plugins

All external plugins must be copied over via FTP to the `/user/mira/plugins` directory in order for the plugin to be loaded on startup. External plugins have a specific linker format that they must follow, meanwhile internal or integrated plugins can just call the initialization routine directly.
