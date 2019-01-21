# If the FREEBSD headers path is not set we will try to use the relative path
ifeq ($(BSD_INC),)
BSD_INC := ../freebsd-headers
endif

ifeq ($(HDE_INC),)
HDE_INC := ../hde
endif

# Project name
PROJ_NAME := OniFramework

# C++ compiler
CPPC	:=	clang

# C compiler
CC		:=	clang

# Archiver
AS		:=	llvm-ar

# Linker
LNK		:=	ld

# Objcopy
OBJCOPY	:=	objcopy

# Output directory
ifeq ($(OUT_DIR),)
OUT_DIR	:=	build
endif

# Source directory
SRC_DIR	:=	src

# Include directory paths
I_DIRS	:=	-I. -Iinclude -I$(SRC_DIR) -Iinclude/nanopb -I"$(BSD_INC)" -I"$(HDE_INC)" -Idepends/include

# Library directory paths
L_DIRS	:=	-L.	-Llib

# Included libraries
LIBS	:= 

# C++ Flags don't use optimizations -O0 must be used for clang, gcc runs with 02 just fine
# Removed From GCC: -nostartfiles
CFLAGS	:= $(I_DIRS) -fPIC -D_DEBUG -D_KERNEL=1 -D_STANDALONE -D"ONI_PLATFORM=${ONI_PLATFORM}" -D__LP64__ -D_M_X64 -D__amd64__ -std=c11 -O0 -fno-builtin -nodefaultlibs -nostdlib -nostdinc -fcheck-new -ffreestanding -fno-strict-aliasing -fno-exceptions -fno-asynchronous-unwind-tables -Wall -m64 -Werror -Wno-unknown-pragmas

# Assembly flags
# Removed From GCC: -nostartfiles
SFLAGS	:= -nodefaultlibs -nostdlib -fPIC

# Linker flags
# Removed From GCC: -nostartfiles -Wl,--build-id=none  -Xlinker -T link.x
LFLAGS	:= $(L_DIRS) -nodefaultlibs -nostdlib -pic -gc-sections -nmagic

# Calculate the listing of all file paths
CFILES	:=	$(wildcard $(SRC_DIR)/*.c)
CPPFILES :=	$(wildcard $(SRC_DIR)/*.cpp)
SFILES	:=	$(wildcard $(SRC_DIR)/*.s)
OBJS	:=	$(patsubst $(SRC_DIR)/%.c, $(OUT_DIR)/$(SRC_DIR)/%.o, $(CFILES)) $(patsubst $(SRC_DIR)/%.s, $(OUT_DIR)/$(SRC_DIR)/%.o, $(SFILES)) $(patsubst $(SRC_DIR)/%.cpp, $(OUT_DIR)/$(SRC_DIR)/%.o, $(CPPFILES))

ALL_CPP := $(shell find $(SRC_DIR)/ -type f -name '*.cpp')
ALL_C	:= $(shell find $(SRC_DIR)/ -type f -name '*.c')
ALL_S	:= $(shell find $(SRC_DIR)/ -type f -name '*.s')

ALL_SOURCES :=  $(ALL_S) $(ALL_C) $(ALL_CPP)
#ALL_OBJ := $(addprefix $(OUT_DIR)/, $(SOURCES:%.s=%.0)) $(addprefix $(OUT_DIR)/, $(SOURCES:%.c=%.o)) $(addprefix $(OUT_DIR)/, $(SOURCES:%.cpp=%.o))
TO_BUILD := $(ALL_S:$(SRC_DIR)%=$(OUT_DIR)/$(SRC_DIR)%) $(ALL_C:$(SRC_DIR)%=$(OUT_DIR)/$(SRC_DIR)%) $(ALL_CPP:$(SRC_DIR)%=$(OUT_DIR)/$(SRC_DIR)%)
ALL_OBJ_CPP := $(TO_BUILD:.cpp=.o)
ALL_OBJ_C := $(ALL_OBJ_CPP:.c=.o)
ALL_OBJ := $(ALL_OBJ_C:.s=.o)

# Target name
TARGET = $(PROJ_NAME).a

$(TARGET): $(ALL_OBJ)
	@echo "Building for Firmware $(ONI_PLATFORM)...";
	@echo Compiling $(PROJ_NAME)...
	@$(AS) rcs $(TARGET) $(ALL_OBJ)

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