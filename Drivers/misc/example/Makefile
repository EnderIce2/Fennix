# Config files
include ../../../Makefile.conf
include ../../config.mk

S_SOURCES = $(shell find ./ -type f -name '*.S')
C_SOURCES = $(shell find ./ -type f -name '*.c')
CPP_SOURCES = $(shell find ./ -type f -name '*.cpp')
HEADERS = $(sort $(dir $(wildcard ../../include/*)))
OBJ = $(C_SOURCES:.c=.o) $(CPP_SOURCES:.cpp=.o) $(S_SOURCES:.S=.o)
STACK_USAGE_OBJ = $(C_SOURCES:.c=.su) $(CPP_SOURCES:.cpp=.su)

FILENAME = example.drv

build: $(FILENAME)
	mv $(FILENAME) ../../out/$(FILENAME)

$(FILENAME): $(OBJ)
	$(info Linking $@)
	$(CC) $(DRIVER_LDFLAGS) $(OBJ) ../../out/dcrt0.o -L../../out -lkernel -o $@

clean:
	rm -f file.map $(OBJ) $(STACK_USAGE_OBJ)
