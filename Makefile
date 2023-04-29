prepare:
	make -C BIOS prepare
	make -C UEFI prepare

loader:
	make -C FennixLoader build

build:
	make -C BIOS build
	make -C UEFI build
	cp BIOS/loader.bin .
	cp UEFI/efi-loader.bin .

clean:
	make -C BIOS clean
	make -C UEFI clean
	make -C FennixLoader clean
	rm -f loader.bin efi-loader.bin FLDR.elf
