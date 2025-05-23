KERNEL_FILENAME = fennix.elf

GIT_COMMIT = $(shell git rev-parse HEAD)
GIT_COMMIT_SHORT = $(shell git rev-parse --short HEAD)

HEADERS := $(sort $(dir $(wildcard ./include/*))) $(sort $(dir $(wildcard ./include_std/*)))
INCLUDE_DIR = -I./include -I./include_std

define find-sources
	$(shell find ./ -type f -name '$1' $(shell echo $(foreach arch,$(filter-out $(OSARCH),$(AVAILABLE_ARCHS)), -not -path \"./arch/$(arch)/*\")) -print0 | xargs -0)
endef

PNG_SOURCES := $(call find-sources,*.png)
PSF_SOURCES := $(call find-sources,*.psf)
S_SOURCES := $(call find-sources,*.S)
s_SOURCES := $(call find-sources,*.s)
C_SOURCES := $(call find-sources,*.c)
CXX_SOURCES := $(call find-sources,*.cpp)

OBJ = $(PNG_SOURCES:.png=.o) $(PSF_SOURCES:.psf=.o) $(s_SOURCES:.s=.o) $(S_SOURCES:.S=.o) $(C_SOURCES:.c=.o) $(CXX_SOURCES:.cpp=.o)
STACK_USAGE_OBJ = $(C_SOURCES:.c=.su) $(CXX_SOURCES:.cpp=.su)
GCNO_OBJ = $(C_SOURCES:.c=.gcno) $(CXX_SOURCES:.cpp=.gcno)

LDFLAGS = -static -nostdlib -nodefaultlibs -nolibc \
	-Wl,-static,--no-dynamic-linker,-ztext 	\
	-zmax-page-size=0x1000					\
	-Wl,-Map kernel.map -fno-pic -fno-pie

# Disable all warnings by adding "-w" in WARNCFLAG and if you want to treat the warnings as errors, add "-Werror"
# -Wconversion this may be re-added later
WARNCFLAG = -Wall -Wextra \
			-Wfloat-equal -Wpointer-arith -Wcast-align \
			-Wredundant-decls -Winit-self -Wswitch-default \
			-Wstrict-overflow=5 -Wno-error=cpp -Werror \
			-Wno-unused-parameter -Wno-error=format

CFLAG_STACK_PROTECTOR := -fstack-protector-all

# https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html
CFLAGS :=										\
	$(INCLUDE_DIR)								\
	-D__kernel__='1'							\
	-DKERNEL_NAME='"$(OSNAME)"' 				\
	-DKERNEL_ARCH='"$(OSARCH)"' 				\
	-DKERNEL_VERSION='"$(KERNEL_VERSION)"'		\
	-DGIT_COMMIT='"$(GIT_COMMIT)"'				\
	-DGIT_COMMIT_SHORT='"$(GIT_COMMIT_SHORT)"'	\
	-fno-pic -fno-pie -fno-builtin

ifeq ($(OSARCH), amd64)
CFLAGS += -march=core2 -mcmodel=kernel -m64 -mno-red-zone
LDFLAGS += -Tarch/amd64/linker.ld
else ifeq ($(OSARCH), i386)
CFLAGS += -march=pentium -m32 -mno-red-zone
LDFLAGS += -Tarch/i386/linker.ld
else ifeq ($(OSARCH), arm)
CFLAGS += -march=armv7-a -mfloat-abi=softfp -ggdb3
LDFLAGS += -Tarch/arm/linker.ld
WARNCFLAG += -w
else ifeq ($(OSARCH), aarch64)
CFLAGS += -march=armv9.4-a -mtune=cortex-a72 -mlittle-endian -mcmodel=large
LDFLAGS += -Tarch/aarch64/linker.ld
endif # OSARCH

# -finstrument-functions for __cyg_profile_func_enter & __cyg_profile_func_exit. Used for profiling and debugging.
ifeq ($(DEBUG), 1)
#	CFLAGS += --coverage
#	CFLAGS += -pg
#	CFLAGS += -finstrument-functions
	CFLAGS += -DDEBUG -ggdb3 -O0 -fdiagnostics-color=always -fstack-usage -fsanitize=undefined
	CXXFLAGS += -fdiagnostics-all-candidates
ifeq ($(OSARCH), amd64)
	CFLAGS += -fverbose-asm
endif # amd64
ifeq ($(OSARCH), arm)
	CFLAGS += -fstack-check -fverbose-asm
endif # arm
ifeq ($(OSARCH), aarch64)
	CFLAGS += -fstack-check -fverbose-asm
endif # aarch64
	LDFLAGS += -ggdb3 -O0
	ASFLAGS += -g --gstabs+ --gdwarf-5 -D
	WARNCFLAG += -Wno-unused-function -Wno-maybe-uninitialized -Wno-builtin-declaration-mismatch -Wno-unknown-pragmas -Wno-unused-parameter -Wno-unused-variable
endif # DEBUG

default:
	$(error Do not run this Makefile directly!)

prepare:
	$(info Nothing to prepare)

build: $(KERNEL_FILENAME)

dump:
ifeq (,$(wildcard $(KERNEL_FILENAME)))
	$(error $(KERNEL_FILENAME) does not exist)
endif
	$(info Dumping $(KERNEL_FILENAME) in AT T syntax...)
	$(__CONF_OBJDUMP) -D -g -s -d $(KERNEL_FILENAME) > kernel_dump.map
	$(info Dumping $(KERNEL_FILENAME) in Intel syntax...)
	$(__CONF_OBJDUMP) -M intel -D -g -s -d $(KERNEL_FILENAME) > kernel_dump_intel.map

$(KERNEL_FILENAME): $(OBJ)
	$(info Linking $@)
	$(__CONF_CC) $(LDFLAGS) $(OBJ) -o $@
#	$(__CONF_CC) $(LDFLAGS) $(OBJ) -mno-red-zone -lgcc -o $@

%.o: %.c $(HEADERS)
	$(info Compiling $<)
	$(__CONF_CC) $(CFLAGS) $(CFLAG_STACK_PROTECTOR) $(WARNCFLAG) -std=c17 -c $< -o $@

# https://gcc.gnu.org/projects/cxx-status.html
%.o: %.cpp $(HEADERS)
	$(info Compiling $<)
	$(__CONF_CXX) $(CFLAGS) $(CXXFLAGS) -fcoroutines $(CFLAG_STACK_PROTECTOR) $(WARNCFLAG) -std=c++20 -c $< -o $@ -fno-rtti

%.o: %.S
	$(info Compiling $<)
	$(__CONF_AS) $(ASFLAGS) -c $< -o $@

%.o: %.s
	$(info Compiling $<)
	$(__CONF_AS) $(ASFLAGS) -c $< -o $@

%.o: %.psf
ifeq ($(OSARCH), amd64)
	$(__CONF_OBJCOPY) -O elf64-x86-64 -I binary $< $@
else ifeq ($(OSARCH), i386)
	$(__CONF_OBJCOPY) -O elf32-i386 -I binary $< $@
else ifeq ($(OSARCH), arm)
	$(__CONF_OBJCOPY) -O elf32-littlearm -I binary $< $@
else ifeq ($(OSARCH), aarch64)
	$(__CONF_OBJCOPY) -O elf64-littleaarch64 -I binary $< $@
endif
	$(__CONF_NM) $@

%.o: %.png
ifeq ($(OSARCH), amd64)
	$(__CONF_OBJCOPY) -O elf64-x86-64 -I binary $< $@
else ifeq ($(OSARCH), i386)
	$(__CONF_OBJCOPY) -O elf32-i386 -I binary $< $@
else ifeq ($(OSARCH), arm)
	$(__CONF_OBJCOPY) -O elf32-littlearm -I binary $< $@
else ifeq ($(OSARCH), aarch64)
	$(__CONF_OBJCOPY) -O elf64-littlearch64 -I binary $< $@
endif
	$(__CONF_NM) $@

clean:
	rm -f kernel.map kernel_dump.map kernel_dump_intel.map $(OBJ) $(STACK_USAGE_OBJ) $(GCNO_OBJ) $(KERNEL_FILENAME)
