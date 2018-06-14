# If ONI_FRAMEWORK is not set, we will use a default path of same directory
ifeq ($(ONI_FRAMEWORK),)
ONI_FRAMEWORK := ../Dependencies/oni-framework
endif

# If the FREEBSD headers path is not set we will try to use the relative path
ifeq ($(BSD_INC),)
BSD_INC := ../Dependencies/freebsd-headers
endif

# Project name
PROJ_NAME := ExamplePlugin

# C++ compiler
CPPC	:=	g++

# Linker
LNK		:= gcc # ps4-ld, we are compiling for the kernel so this is not going to use the OpenOrbis userland linker

# C compiler
CC		:=	gcc

# Archiver
AS		:=	gcc

# Objcopy
OBJCOPY	:=	objcopy

# Output directory, by default is build
ifeq ($(OUT_DIR),)
OUT_DIR	:=	build
endif

# Source directory
SRC_DIR	:=	src

# Include directory paths
I_DIRS	:=	-I. -IInclude -I$(SRC_DIR) -I$(ONI_FRAMEWORK)/depends/include -I$(ONI_FRAMEWORK)/include -I$(BSD_INC)/include

# Library directory paths
L_DIRS	:=	-L.	-Llib -L$(ONI_FRAMEWORK)

# Included libraries
LIBS	:= -l:OniFramework.a

# Orbis flags
ORB_VER	:= -D_KERNEL -D_DEBUG=1

# C++ Flags
CFLAGS	:= $(I_DIRS) $(ORB_VER) -std=c11 -O2 -fno-builtin -nostartfiles -nodefaultlibs -nostdlib -nostdinc -fcheck-new -ffreestanding -fno-strict-aliasing -fno-exceptions -fno-asynchronous-unwind-tables -Wall -m64 -fPIC -Werror=implicit-function-declaration -Wno-unknown-pragmas

# Assembly flags
SFLAGS	:= -nostartfiles -nodefaultlibs -nostdlib -fPIC

# Linker flags
LFLAGS	:= $(L_DIRS) -nostdlib -T $(ONI_FRAMEWORK)/link.x -Wl,--build-id=none

# Calculate the listing of all file paths
CFILES	:=	$(wildcard $(SRC_DIR)/*.c)
CPPFILES :=	$(wildcard $(SRC_DIR)/*.cpp)
SFILES	:=	$(wildcard $(SRC_DIR)/*.s)
OBJS	:=	$(patsubst $(SRC_DIR)/%.s, $(OUT_DIR)/$(SRC_DIR)/%.o, $(SFILES)) $(patsubst $(SRC_DIR)/%.c, $(OUT_DIR)/$(SRC_DIR)/%.o, $(CFILES)) $(patsubst $(SRC_DIR)/%.cpp, $(OUT_DIR)/$(SRC_DIR)/%.o, $(CPPFILES))

ALL_CPP := $(shell find $(SRC_DIR)/ -type f -name '*.cpp')
ALL_C	:= $(shell find $(SRC_DIR)/ -type f -name '*.c')
ALL_S	:= $(shell find $(SRC_DIR)/ -type f -name '*.s')

ALL_SOURCES :=  $(ALL_S) $(ALL_C) $(ALL_CPP)
TO_BUILD := $(ALL_S:$(SRC_DIR)%=$(OUT_DIR)/$(SRC_DIR)%) $(ALL_C:$(SRC_DIR)%=$(OUT_DIR)/$(SRC_DIR)%) $(ALL_CPP:$(SRC_DIR)%=$(OUT_DIR)/$(SRC_DIR)%)
ALL_OBJ_CPP := $(TO_BUILD:.cpp=.o)
ALL_OBJ_C := $(ALL_OBJ_CPP:.c=.o)
ALL_OBJ := $(ALL_OBJ_C:.s=.o)

# Target elf name
TARGET = $(PROJ_NAME)_Orbis.elf

# Target payload name (data + text only, no elf)
PAYLOAD = $(PROJ_NAME)_Orbis.bin

$(TARGET): $(ALL_OBJ)
	@echo Compiling $(PROJ_NAME)...
	@$(LNK) $(ALL_OBJ) -o $(TARGET) $(LFLAGS) $(LIBS)
	@echo "Creating Payload..."
	@$(OBJCOPY) -O binary $(TARGET) $(PAYLOAD)

$(OUT_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $< ..."
	@$(CC) $(CFLAGS) $(IDIRS) -c $< -o $@

$(OUT_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "Compiling $< ..."
	@$(CPPC) $(CFLAGS) $(IDIRS) -c $< -o $@

$(OUT_DIR)/$(SRC_DIR)/%.o: $(SRC_DIR)/%.s
	@echo "Compiling $< ..."
	@$(CC) -c -o $@ $< $(SFLAGS)

.PHONY: clean

clean:
	@echo "Cleaning project..."
	@rm -f $(TARGET) $(PAYLOAD) $(shell find $(OUT_DIR)/ -type f -name '*.o')

create:
	@echo "Creating directories..."
	@mkdir -p $(shell find '$(SRC_DIR)/' -type d -printf '$(OUT_DIR)/%p\n')
