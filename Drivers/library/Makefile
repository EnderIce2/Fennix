# Config file
include ../../config.mk

FILENAME = libkernel.so

CC = ../../$(COMPILER_PATH)/$(COMPILER_ARCH)gcc
CPP = ../../$(COMPILER_PATH)/$(COMPILER_ARCH)g++
LD = ../../$(COMPILER_PATH)/$(COMPILER_ARCH)ld
AS = ../../$(COMPILER_PATH)/$(COMPILER_ARCH)as
AR = ../../$(COMPILER_PATH)/$(COMPILER_ARCH)ar

S_SOURCES = $(shell find ./ -type f -name '*.S' -not -path "./crt/*")
C_SOURCES = $(shell find ./ -type f -name '*.c' -not -path "./crt/*")
CPP_SOURCES = $(shell find ./ -type f -name '*.cpp' -not -path "./crt/*")
HEADERS = $(sort $(dir $(wildcard ../include/*)))
OBJ = $(C_SOURCES:.c=.o) $(CPP_SOURCES:.cpp=.o) $(ASM_SOURCES:.asm=.o) $(S_SOURCES:.S=.o) $(PSF_SOURCES:.psf=.o) $(BMP_SOURCES:.bmp=.o)
STACK_USAGE_OBJ = $(C_SOURCES:.c=.su) $(CPP_SOURCES:.cpp=.su)
INCLUDE_DIR = ../include

LDFLAGS := -fPIC -fPIE -pie -nostdlib -nodefaultlibs -nolibc	\
	-zmax-page-size=0x1000 -Wl,-Map libkernel.map -shared

WARNCFLAG = -Wall -Wextra

CFLAGS := -I$(INCLUDE_DIR) -shared

ifeq ($(OSARCH), amd64)

CFLAGS += -fPIC -fPIE -pie -mno-80387 -mno-mmx -mno-3dnow	\
		  -mno-red-zone -mno-sse -mno-sse2					\
		  -march=x86-64 -pipe -ffunction-sections			\
		  -msoft-float -fno-builtin

else ifeq ($(OSARCH), i386)

CFLAGS += -fPIC -fPIE -pie -mno-80387 -mno-mmx -mno-3dnow		\
		  -mno-red-zone -mno-sse -mno-sse2 -ffunction-sections	\
		  -march=i386 -pipe -msoft-float -fno-builtin

else ifeq ($(OSARCH), aarch64)

CFLAGS += -pipe -fno-builtin -fPIC

endif

CRT_CFLAGS := -fPIC -fPIE -pie -mno-red-zone -std=c++20 -I../include

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -ggdb3 -O0 -fdiagnostics-color=always -fstack-usage
	CRT_CFLAGS += -DDEBUG -ggdb3 -O0 -fdiagnostics-color=always -fstack-usage
ifeq ($(OSARCH), amd64)
	CFLAGS += -fverbose-asm
	CRT_CFLAGS += -fverbose-asm
endif
ifneq ($(OSARCH), aarch64)
	CFLAGS += -fstack-check
	CRT_CFLAGS += -fstack-check
endif
	LDFLAGS += -ggdb3 -O0
endif

build: $(FILENAME)
	$(CPP) $(CRT_CFLAGS) -c crt/crt0.cpp -o dcrt0.o
	mv dcrt0.o ../out/dcrt0.o

$(FILENAME): $(OBJ)
	$(info Linking $@)
	$(CC) $(LDFLAGS) $(OBJ) -o ../out/$(FILENAME)

%.o: %.c $(HEADERS)
	$(info Compiling $<)
	$(CC) $(CFLAGS) $(WARNCFLAG) -std=c17 -c $< -o $@

%.o: %.cpp $(HEADERS)
	$(info Compiling $<)
	$(CPP) $(CFLAGS) $(WARNCFLAG) -std=c++20 -fexceptions -c $< -o $@

%.o: %.S
	$(info Compiling $<)
	$(AS) -o $@ $<

clean:
	rm -f libkernel.map $(OBJ) $(FILENAME) $(STACK_USAGE_OBJ) dcrt0.su
