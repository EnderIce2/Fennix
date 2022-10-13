include ../../Makefile.conf

NAME=efi-loader.bin
CC = gcc
LD = ld
OBJCOPY = objcopy

C_SOURCES = $(shell find ./src -type f -name '*.c')
OBJ = $(C_SOURCES:.c=.o)

GNUEFI_RELEASE_VERSION=3.0.14

gnuefi:
	wget https://archive.org/download/gnu-efi-$(GNUEFI_RELEASE_VERSION).tar/gnu-efi-$(GNUEFI_RELEASE_VERSION).tar.bz2
	tar -xf gnu-efi-$(GNUEFI_RELEASE_VERSION).tar.bz2
	rm gnu-efi-$(GNUEFI_RELEASE_VERSION).tar.bz2
	mv ./gnu-efi-$(GNUEFI_RELEASE_VERSION) ./gnu-efi
	mkdir -p include
	cp -a ./gnu-efi/inc/. ./include
	make -C gnu-efi

prepare: gnuefi

build: $(NAME)

$(NAME): BOOTX64
	dd if=/dev/zero of=$(NAME) bs=512 count=93750
	mformat -i $(NAME) ::
	mmd -i $(NAME) ::/EFI
	mmd -i $(NAME) ::/EFI/BOOT
	mcopy -i $(NAME) BOOTX64.EFI ::/EFI/BOOT

BOOTX64: $(OBJ)
	$(LD) -shared -Bsymbolic -Lgnu-efi/x86_64/lib -Lgnu-efi/x86_64/gnuefi -Tgnu-efi/gnuefi/elf_x86_64_efi.lds gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o $(OBJ) -o tmp.so -lgnuefi -lefi
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 tmp.so BOOTX64.EFI
	rm tmp.so

%.o: %.c
	$(info Compiling $<)
	$(CC) -Ignu-efi/inc -Ignu-efi/inc/x86_64 -Ignu-efi/inc/protocol -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c $< -o $@


clean:
	rm -f $(NAME) $(OBJ) BOOTX64.EFI
