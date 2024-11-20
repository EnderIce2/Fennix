# Config file
include ../../../Makefile.conf

NAME=gcc

OBJECT_NAME=lib$(NAME).a

OUTPUT_DIR=../../out/lib/

CC = ../../../$(COMPILER_PATH)/$(COMPILER_ARCH)gcc

CRTBEGIN_PATH = $(shell $(CC) -print-file-name=libgcc.a)

build: $(OBJECT_NAME)

$(OBJECT_NAME):
	cp $(CRTBEGIN_PATH) $(OUTPUT_DIR)

clean:

