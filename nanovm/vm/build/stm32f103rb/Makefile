#
#  Makefile for STM32 test version of the NanoVM
#

TOOLCHAIN_BIN_PATH =/usr/local/xtools/Mentor/Sourcery_CodeBench/ARM_EABI/bin
TOOLCHAIN_LIB_PATH =/usr/local/xtools/Mentor/Sourcery_CodeBench/ARM_EABI/lib
PREFIX =$(TOOLCHAIN_BIN_PATH)/arm-none-eabi-

CPU := cortex-m3
INSTRUCTION_MODE := thumb
TARGET := NanoVM
TARGET_EXT := elf
LD_SCRIPT := ./ldscripts/stm32.ld
#LD_SCRIPT := ../ldscripts/STM32F103xB_rom.ld

LIBS := 
LIBS += -L ../../../bsp/STM32F10x_StdPeriph_Lib/Libraries/STM32F10x_StdPeriph_Driver/inc

INCLUDE += -I .
INCLUDE += -I ./startup
#INCLUDE += -I ../startups
INCLUDE += -I ../../src
INCLUDE += -I ../../src/stm32
INCLUDE += -I ../../../bsp/Emu
INCLUDE += -I ../../../bsp/STM32F10x_StdPeriph_Lib/Libraries/CMSIS/CM3/CoreSupport
INCLUDE += -I ../../../bsp/STM32F10x_StdPeriph_Lib/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
INCLUDE += -I ../../../bsp/STM32F10x_StdPeriph_Lib/Libraries/STM32F10x_StdPeriph_Driver/inc
INCLUDE += -I $(TOOLCHAIN_LIB_PATH)/gcc/arm-none-eabi/4.6.1/include

OBJ_FOLDER := ./objs

COMPILER_OPTIONS = -O0 -g -ggdb -Wall -fno-strict-aliasing \
-fmessage-length=0 -fno-builtin -m$(INSTRUCTION_MODE) \
-mcpu=$(CPU) -MMD -MP -DSTM32 -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER

DEPEND_OPTS = -MF $(OBJ_FOLDER)/$(patsubst %.o,%.d,$(notdir $@)) \
-MT $(OBJ_FOLDER)/$(patsubst %.o,%.d,$(notdir $@))

CC = $(PREFIX)gcc
CFLAGS = $(COMPILER_OPTIONS) $(INCLUDE) $(DEPEND_OPTS) -c

CXX = $(PREFIX)g++
CXXFLAGS = $(COMPILER_OPTIONS) $(INCLUDE) $(DEPEND_OPTS) -c
AS = $(PREFIX)gcc
ASFLAGS = $(COMPILER_OPTIONS) $(INCLUDE) $(DEPEND_OPTS) -c

LD = $(PREFIX)gcc
LD_OPTIONS = -Wl,--gc-sections,-Map=$(TARGET).map,--cref -T $(LD_SCRIPT) $(INCLUDE)

OBJCP = $(PREFIX)objcopy
OBJCPFLAGS = -O ihex

AR = $(PREFIX)ar
ARFLAGS = cr

RM := rm -rf

USER_OBJS :=
C_SRCS :=
S_SRCS :=
C_OBJS :=
S_OBJS :=

# Every subdirectory with source files must be described here
SUBDIRS := ./startup
#SUBDIRS := ../startups
SUBDIRS += ../../src
SUBDIRS += ../../src/stm32
SUBDIRS += ../../../bsp/Emu
SUBDIRS += ../../../bsp/STM32F10x_StdPeriph_Lib/Libraries/CMSIS/CM3/CoreSupport
SUBDIRS += ../../../bsp/STM32F10x_StdPeriph_Lib/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x
SUBDIRS += ../../../bsp/STM32F10x_StdPeriph_Lib/Libraries/STM32F10x_StdPeriph_Driver/src

C_SRCS := $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.c))
C_OBJS := $(patsubst %.c,$(OBJ_FOLDER)/%.o,$(notdir $(C_SRCS)))
S_SRCS := $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.s))
S_OBJS := $(patsubst %.s,$(OBJ_FOLDER)/%.o,$(notdir $(S_SRCS)))
#S_SRCS := $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.S))
#S_OBJS := $(patsubst %.S,$(OBJ_FOLDER)/%.o,$(notdir $(S_SRCS)))

VPATH := $(SUBDIRS)

$(OBJ_FOLDER)/%.o : %.c
	@echo ‘Building file: $(@F)’
	@echo ‘Invoking: MCU C Compiler’
	$(CC) $(CFLAGS) $< -o $@
	@echo 'Finished building: $(@F)'
	@echo ' '

$(OBJ_FOLDER)/%.o : %.s
#$(OBJ_FOLDER)/%.o : %.S
	@echo 'Building file: $(@F)'
	@echo 'Invoking: MCU Assembler'
	$(AS) $(ASFLAGS) $< -o $@
	@echo 'Finished building: $(@F)'
	@echo ' '

# All Target
all: $(TARGET).$(TARGET_EXT)

# Tool invocations
$(TARGET).$(TARGET_EXT): $(C_OBJS) $(S_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	$(LD) $(LD_OPTIONS) $(C_OBJS) $(S_OBJS) $(USER_OBJS) $(LIBS) -o$(TARGET).$(TARGET_EXT)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(TARGET).$(TARGET_EXT) $(TARGET).bin $(TARGET).map $(OBJ_FOLDER)/*.*
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	$(PREFIX)size --totals $(TARGET).$(TARGET_EXT); $(PREFIX)objcopy -O binary $(TARGET).$(TARGET_EXT) $(TARGET).bin ;
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build 