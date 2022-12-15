# Config file
include ../../../Makefile.conf

FILENAME = Realtek-8139.fex

CC = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)gcc
CPP = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)g++
LD = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)ld
AS = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)as
OBJDUMP = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)objdump

GIT_COMMIT = $(shell git rev-parse HEAD)
GIT_COMMIT_SHORT = $(shell git rev-parse --short HEAD)

ifeq ($(OSARCH), amd64)
S_SOURCES = $(shell find ./ -type f -name '*.S' -not -path "./arch/i686/*" -not -path "./arch/aarch64/*")
C_SOURCES = $(shell find ./ -type f -name '*.c' -not -path "./arch/i686/*" -not -path "./arch/aarch64/*")
CPP_SOURCES = $(shell find ./ -type f -name '*.cpp' -not -path "./arch/i686/*" -not -path "./arch/aarch64/*")
else ifeq ($(OSARCH), i686)
S_SOURCES = $(shell find ./ -type f -name '*.S' -not -path "./arch/amd64/*" -not -path "./arch/aarch64/*")
C_SOURCES = $(shell find ./ -type f -name '*.c' -not -path "./arch/amd64/*" -not -path "./arch/aarch64/*")
CPP_SOURCES = $(shell find ./ -type f -name '*.cpp' -not -path "./arch/amd64/*" -not -path "./arch/aarch64/*")
else ifeq ($(OSARCH), aarch64)
S_SOURCES = $(shell find ./ -type f -name '*.S' -not -path "./arch/amd64/*" -not -path "./arch/i686/*")
C_SOURCES = $(shell find ./ -type f -name '*.c' -not -path "./arch/amd64/*" -not -path "./arch/i686/*")
CPP_SOURCES = $(shell find ./ -type f -name '*.cpp' -not -path "./arch/amd64/*" -not -path "./arch/i686/*")
endif
HEADERS = $(sort $(dir $(wildcard ../../include/*)))
OBJ = $(C_SOURCES:.c=.o) $(CPP_SOURCES:.cpp=.o) $(ASM_SOURCES:.asm=.o) $(S_SOURCES:.S=.o) $(PSF_SOURCES:.psf=.o) $(BMP_SOURCES:.bmp=.o)
INCLUDE_DIR = ../../include

LDFLAGS := 															\
	-fPIC -fPIE -pie -Wl,-eDriverEntry								\
	-Wl,-static,--no-dynamic-linker,-ztext,--no-warn-rwx-segment 	\
	-nostdlib -nodefaultlibs -nolibc  								\
	-zmax-page-size=0x1000											\
 	-Wl,-Map file.map -static -Tlinker.ld

WARNCFLAG = -Wall -Wextra

CFLAGS :=										\
	-I$(INCLUDE_DIR)							\
	-DGIT_COMMIT='"$(GIT_COMMIT)"'				\
	-DGIT_COMMIT_SHORT='"$(GIT_COMMIT_SHORT)"'

ifeq ($(OSARCH), amd64)

CFLAGS += -fPIC -fPIE -pie -mno-80387 -mno-mmx -mno-3dnow	\
		  -mno-red-zone -mno-sse -mno-sse2					\
		  -march=x86-64 -pipe -ffunction-sections			\
		  -msoft-float -fno-builtin

else ifeq ($(OSARCH), i686)

CFLAGS += -fPIC -fPIE -pie -mno-80387 -mno-mmx -mno-3dnow		\
		  -mno-red-zone -mno-sse -mno-sse2 -ffunction-sections	\
		  -march=i686 -pipe -msoft-float -fno-builtin

else ifeq ($(OSARCH), aarch64)

CFLAGS += -pipe -fno-builtin -fPIC

endif

build: $(FILENAME)
ifeq ($(OSARCH), amd64)
	$(OBJDUMP) -b binary -D -m i386:x86-64 -d $(FILENAME) > file_dump.map
else ifeq ($(OSARCH), i686)

else ifeq ($(OSARCH), aarch64)

endif
	mv $(FILENAME) ../../out/$(FILENAME)

$(FILENAME): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $@

%.o: %.c $(HEADERS)
	$(info Compiling $<)
	$(CC) $(CFLAGS) $(WARNCFLAG) -std=c17 -c $< -o $@

%.o: %.cpp $(HEADERS)
	$(info Compiling $<)
	$(CPP) $(CFLAGS) $(WARNCFLAG) -std=c++20 -fexceptions -c $< -o $@ -fno-rtti

%.o: %.S
	$(info Compiling $<)
ifeq ($(OSARCH), amd64)
	$(AS) -o $@ $<
else ifeq ($(OSARCH), i686)
	$(AS) -o $@ $<
else ifeq ($(OSARCH), aarch64)
	$(AS) -o $@ $<
endif

clean:
	rm -f *.o file.map file_dump.map $(OBJ)
