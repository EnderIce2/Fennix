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
	$(info Nothing to prepare)

clean:
