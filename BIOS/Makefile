include ../../Makefile.conf

NAME=loader.bin
NASM = /usr/bin/nasm

ASM_SOURCES = $(shell find ./ -type f -name '*.asm')
OBJ = $(ASM_SOURCES:.asm=.o)

prepare:
	$(info Nothing to prepare)

$(NAME): $(OBJ)
	cat boot.o second.o > $@

build: $(NAME)

%.o: %.asm
	$(info Compiling $<)
	$(NASM) $< -f bin -o $@

clean:
	rm -f $(OBJ) $(NAME)
