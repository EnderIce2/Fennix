prepare:
	make -C BIOS prepare
	make -C UEFI prepare

build:
	make -C BIOS build
	make -C UEFI build
	cp BIOS/loader.bin .
	cp UEFI/efi-loader.bin .

clean:
	make -C BIOS clean
	make -C UEFI clean
	rm -f loader.bin efi-loader.bin
