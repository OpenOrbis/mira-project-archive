# Oni-Framework


Oni framework is a platform for embedded device software development. It is used in order to remotely debug/add functionality to an embedded device. As of right now, the main concepts are a fully modular plugin system, with local and remote rpc. This was designed to run at ring-0 privilege level, but with some tweaks can also be used in ring-3 levels. Be warned, this is *NOT* for production use, only for developers who want an easy building platform for research.

  - Currently this project is in development, and may not be fully tested
  - Will need decent knowledge of C/C#
  - Magic

# New Features!

### Currently supported features

  - Fully robust local and remote RPC system
  - All used functions are stubbed out in order to ease porting to different platforms
  - File transfer utility
  - Color logging system (works over UART with PUTTY)

Features that are planned:
  - Debugging framework
  - Remote package management and install
  - Communication between devices
  - Hot-swappable loading and unloading of plugin modules
  - Self-escalating from ring-3 to ring-0 (BYO-EntryPoint)

### Building and Installation

Oni-Framework just requires that you check out this directory and implement the `oni/utils/kdlsym/default.h` file with all of the implementations that are required. After this, it should compile just fine with gcc or clang and produce an executable file. You will need to BYO-execution method, but for most embedded devices this can be accomplished via a ssh shell, or a shell over telnet, or any shell for that matter. Other devices may have to load into running memory context, then jump to `main` which should take care of all of the initialization issues.

1. Clone the repository `git clone https://github.com/kiwidoggie/oni-framework.git`
    a. Optional: Set FreeBSD headers location `export BSD_INC="/path/to/freebsd/headers/include"`
2. If you are building for the first time you must run `make create` which will create the folder structure
3. Build using `make`
    a. If you have not set the environment variable you can build with `make BSD_INC=/path/to/freebsd/headers/include`

### Porting to different platforms
Oni-Framework does not rely on a fully functional libc, and is built to build, and run completely standalone (provided all pre-requisites are met). Instead needed functionality is pieced together from the bare minimum of existing functions either in an running process's context, or kernel context (the latter is the only one tested).

1. Create a new kdlsym (kernel dynamic symbol resolution) target by creating a new header in `include/oni/utils/kdlsym/my_platform.h`
2. Copy and paste the *contents* of `include/oni/utils/kdlsym/default.h` to your newly created header file
3. Add your new target to `include/oni/utils/kdlsym.h`

```
#if ONI_PLATFORM==ONI_UNKNOWN_PLATFORM
#include "kdlsym/default.h"
#elif ...
#elif ONI_PLATFORM==ONI_MY_PLATFORM
#include "kdlsym/my_platform.h"
#endif
```

4. Add your target to the configuration file which is located at `include/oni/config.h`

```
// Unknown device
#define ONI_UNKNOWN_PLATFORM		-1
#define ONI_MY_PLATFORM		5
```

5. Change the current build target by changing 
`#define ONI_PLATFORM ONI_UNKNOWN_PLATFORM` to `#define ONI_PLATFORM ONI_MY_PLATFORM`


### Developers

If you want to contribute, just submit a pull request. Otherwise, there is no real support for this at this point. This may change in the future

![twitter](http://i.imgur.com/tXSoThF.png) [@diwidog](https://twitter.com/diwidog)

#### Message Layout

|Offset	| Len	| Name   | Description   |Usage   |
|-------|-------|---|---|---|
| 0		| 2		| Magic  | This is a simple packet header magic in order to ensure that messages are being processed properly   | REQUIRED: Must be set to 10b or 2 (dec)   |
| 2		| 4		| Message Category  | This describes which category to route the message to on oni-framework side   | REQUIRED: Maximum of 14 categories (5 in use currently)   |
| 5		| 1		| Request   | This flag will be set to 1 if the message is a request, or 0 if it is a response  | REQUIRED   |
| 6		| 32	| Error/MessageType   | This field contains the error code, or the message type  | REQUESTS: This must be set to the message type which is calculated currently by CRC32(MessageName), RESPONSE: This is set to the error code/return value   |
| 38	| 16	| Payload Size   | This is the length of data that comes after the message header  | OPTIONAL: Set to 0 if no payload, otherwise will expect data   |
| 54	| *		| Reserved  | Reserved   | IGNORED: Set to 0   |
