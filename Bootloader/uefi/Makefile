BOOT_FILENAME = BOOTX64.EFI

GNUEFI_RELEASE_VERSION=3.0.18

GIT_COMMIT = $(shell git rev-parse HEAD)
GIT_COMMIT_SHORT = $(shell git rev-parse --short HEAD)

HEADERS := $(sort $(dir $(wildcard ../include/*)))
INCLUDE_DIR = -I../include

define find-sources
	$(shell find ./ -type f -name '$1' -print0 | xargs -0)
endef

BMP_SOURCES := $(call find-sources,*.bmp)
PSF_SOURCES := $(call find-sources,*.psf)
S_SOURCES := $(call find-sources,*.S)
s_SOURCES := $(call find-sources,*.s)
C_SOURCES := $(call find-sources,*.c)
CXX_SOURCES := $(call find-sources,*.cpp)

OBJ = $(BMP_SOURCES:.bmp=.o) $(PSF_SOURCES:.psf=.o) $(s_SOURCES:.s=.o) $(S_SOURCES:.S=.o) $(C_SOURCES:.c=.o) $(CXX_SOURCES:.cpp=.o)

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
LDFLAGS += -T../linker.ld
else ifeq ($(OSARCH), i386)
CFLAGS += -march=pentium -m32 -mno-red-zone
LDFLAGS += -T../linker.ld
else ifeq ($(OSARCH), arm)
CFLAGS += -march=armv7-a -mfloat-abi=softfp -ggdb3
LDFLAGS += -T../linker.ld
WARNCFLAG += -w
else ifeq ($(OSARCH), aarch64)
CFLAGS += -march=armv9.4-a -mtune=cortex-a72 -mlittle-endian -mcmodel=large
LDFLAGS += -T../linker.ld
endif # OSARCH

# -finstrument-functions for __cyg_profile_func_enter & __cyg_profile_func_exit. Used for profiling and debugging.
ifeq ($(DEBUG), 1)
#	CFLAGS += --coverage
#	CFLAGS += -pg
#	CFLAGS += -finstrument-functions
	CFLAGS += -DDEBUG -ggdb3 -O0 -fdiagnostics-color=always -fstack-usage -fsanitize=undefined
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

build: $(BOOT_FILENAME)

$(BOOT_FILENAME): $(OBJ)
	$(info Linking $@)
	$(LD) -shared -Bsymbolic -L../gnu-efi/x86_64/lib -L../gnu-efi/x86_64/gnuefi -T../gnu-efi/gnuefi/elf_x86_64_efi.lds ../gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o $(OBJ) -o tmp.so -lgnuefi -lefi
	objcopy -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 tmp.so BOOTX64.EFI
	rm tmp.so

%.o: %.c $(HEADERS)
	$(info Compiling $<)
	$(CC) -I../include -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c $< -o $@

%.o: %.cpp $(HEADERS)
	$(info Compiling $<)
	$(CC) -I../include -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c $< -o $@

%.o: %.S
	$(info Compiling $<)
	$(AS) $(ASFLAGS) -c $< -o $@

%.o: %.s
	$(info Compiling $<)
	$(AS) $(ASFLAGS) -c $< -o $@

%.o: %.psf
ifeq ($(OSARCH), amd64)
	$(OBJCOPY) -O elf64-x86-64 -I binary $< $@
else ifeq ($(OSARCH), i386)
	$(OBJCOPY) -O elf32-i386 -I binary $< $@
else ifeq ($(OSARCH), arm)
	$(OBJCOPY) -O elf32-littlearm -I binary $< $@
else ifeq ($(OSARCH), aarch64)
	$(OBJCOPY) -O elf64-littleaarch64 -I binary $< $@
endif
	$(NM) $@

%.o: %.bmp
ifeq ($(OSARCH), amd64)
	$(OBJCOPY) -O elf64-x86-64 -I binary $< $@
else ifeq ($(OSARCH), i386)
	$(OBJCOPY) -O elf32-i386 -I binary $< $@
else ifeq ($(OSARCH), arm)
	$(OBJCOPY) -O elf32-littlearm -I binary $< $@
else ifeq ($(OSARCH), aarch64)
	$(OBJCOPY) -O elf64-littlearch64 -I binary $< $@
endif
	$(NM) $@

clean:
	rm -f kernel.map kernel_dump.map kernel_dump_intel.map $(OBJ) $(STACK_USAGE_OBJ) $(GCNO_OBJ) $(BOOT_FILENAME)
