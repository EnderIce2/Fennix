CC = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)gcc
CPP = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)g++
LD = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)ld
AS = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)as
OBJDUMP = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)objdump

DRIVER_LDFLAGS := -nostdlib -nodefaultlibs -nolibc -zmax-page-size=0x1000	\
			-Wl,-Map file.map -fvisibility=hidden -Wl,--dynamic-linker=/boot/fennix.elf

ifeq ($(OSARCH), amd64)

DRIVER_CFLAGS := -fPIC -fPIE -pie -mno-80387 -mno-mmx -mno-3dnow	\
		  -mno-red-zone -mno-sse -mno-sse2							\
		  -march=x86-64 -pipe -ffunction-sections					\
		  -msoft-float -fno-builtin

else ifeq ($(OSARCH), i386)

DRIVER_CFLAGS := -fPIC -fPIE -pie -mno-80387 -mno-mmx -mno-3dnow	\
		  -mno-red-zone -mno-sse -mno-sse2 -ffunction-sections		\
		  -march=i386 -pipe -msoft-float -fno-builtin

else ifeq ($(OSARCH), aarch64)

DRIVER_CFLAGS += -pipe -fno-builtin -fPIC

endif

DRIVER_CFLAGS += -I../../include

ifeq ($(DEBUG), 1)
	DRIVER_CFLAGS += -DDEBUG -ggdb3 -O0 -fdiagnostics-color=always -fstack-usage
ifeq ($(OSARCH), amd64)
	DRIVER_CFLAGS += -fverbose-asm
endif
ifneq ($(OSARCH), aarch64)
	DRIVER_CFLAGS += -fstack-check
endif
	DRIVER_LDFLAGS += -ggdb3 -O0
endif

WARNCFLAG = -Wall -Wextra

%.o: %.c $(HEADERS)
	$(info Compiling $<)
	$(CC) $(DRIVER_CFLAGS) $(WARNCFLAG) -std=c17 -c $< -o $@

%.o: %.cpp $(HEADERS)
	$(info Compiling $<)
	$(CPP) $(DRIVER_CFLAGS) $(WARNCFLAG) -std=c++20 -fno-exceptions -fno-rtti -c $< -o $@

%.o: %.S
	$(info Compiling $<)
	$(AS) -o $@ $<
