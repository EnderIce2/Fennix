
include Makefile.conf

.PHONY: default tools clean

# First rule
default:
	$(error Please specify a target)

# For tap0
# -netdev tap,id=usernet0,ifname=tap0,script=no,downscript=no
QEMU = ./$(QEMU_PATH)$(QEMU_ARCH)
QEMUFLAGS := -display gtk

ifeq ($(OSARCH), amd64)
QEMUFLAGS += -device vmware-svga -M q35 \
			 -usb \
			 -device qemu-xhci,id=xhci \
			 -device usb-mouse,bus=xhci.0,pcap=mousex.pcap \
			 -device usb-kbd,bus=xhci.0,pcap=kbdx.pcap \
			 -device usb-mouse,pcap=mouse.pcap \
			 -device usb-kbd,pcap=kbd.pcap \
			 -net user \
			 -netdev user,id=usernet0 \
			 -device e1000,netdev=usernet0,mac=00:69:96:00:42:00 \
			 -object filter-dump,id=usernet0,netdev=usernet0,file=network.dmp,maxlen=1024 \
			 -serial file:serial.log \
			 -serial file:profiler.log \
			 -serial file:serial3.dmp \
			 -serial stdio \
			 -parallel file:parallel.log \
			 -device ahci,id=ahci \
			 -drive id=bootdsk,file=$(OSNAME).iso,format=raw,if=none \
			 -device ide-hd,drive=bootdsk,bus=ahci.0 \
			 -drive id=disk,file=qemu-disk.qcow2,if=none \
			 -device ide-hd,drive=disk,bus=ahci.1 \
			 -audiodev pa,id=pa1,server=/run/user/1000/pulse/native \
			 -machine pcspk-audiodev=pa1 \
			 -device AC97,audiodev=pa1 \
			 -device intel-hda \
			 -device ich9-intel-hda \
			 -acpitable file=tools/SSDT1.dat
else ifeq ($(OSARCH), i386)
QEMUFLAGS += -M q35 \
			 -usb \
			 -device qemu-xhci,id=xhci \
			 -device usb-mouse,bus=xhci.0,pcap=mousex.pcap \
			 -device usb-kbd,bus=xhci.0,pcap=kbdx.pcap \
			 -device usb-mouse,pcap=mouse.pcap \
			 -device usb-kbd,pcap=kbd.pcap \
			 -net user \
			 -netdev user,id=usernet0 \
			 -device e1000,netdev=usernet0,mac=00:69:96:00:42:00 \
			 -object filter-dump,id=usernet0,netdev=usernet0,file=network.dmp,maxlen=1024 \
			 -serial file:serial.log \
			 -serial file:profiler.log \
			 -serial file:serial3.dmp \
			 -serial stdio \
			 -parallel file:parallel.log \
			 -hda $(OSNAME).iso \
			 -audiodev pa,id=pa1,server=/run/user/1000/pulse/native \
			 -machine pcspk-audiodev=pa1 \
			 -device AC97,audiodev=pa1 \
			 -device intel-hda \
			 -device ich9-intel-hda \
			 -acpitable file=tools/SSDT1.dat
else ifeq ($(OSARCH), aarch64)
QEMUFLAGS += -M raspi3b \
			 -cpu cortex-a57 \
			 -serial file:serial.log \
			 -serial file:profiler.log \
			 -serial file:serial3.dmp \
			 -serial stdio \
			 -kernel $(OSNAME).img \
			 -acpitable file=tools/SSDT1.dat
endif

doxygen:
	mkdir -p doxygen-doc
	doxygen Doxyfile
	doxygen Kernel/Doxyfile
	doxygen Lynx/Doxyfile
	doxygen Userspace/Doxyfile
	doxygen Drivers/Doxyfile

qemu_vdisk:
ifneq (,$(wildcard ./qemu-disk.qcow2))
	$(info qemu-disk.qcow2 Already exists)
else
	qemu-img create -f qcow2 qemu-disk.qcow2 1G
endif

# Install necessary packages, build cross-compiler etc...
tools:
	make --quiet -C tools all
	make --quiet -C Kernel prepare
	make --quiet -C Lynx prepare
	make --quiet -C Userspace prepare
	make --quiet -C Drivers prepare

build: build_lynx build_kernel build_userspace build_drivers build_image

dump:
	make --quiet -C Kernel dump

rebuild: clean build

ifeq ($(QUIET_BUILD), 1)
MAKE_QUIET_FLAG = --quiet
endif

build_lynx:
ifeq ($(BOOTLOADER), lynx)
	make $(MAKE_QUIET_FLAG) -C Lynx build
endif

build_kernel:
ifeq ($(BUILD_KERNEL), 1)
	make -j$(shell nproc) $(MAKE_QUIET_FLAG) -C Kernel build
endif

build_userspace:
ifeq ($(BUILD_USERSPACE), 1)
	make $(MAKE_QUIET_FLAG) -C Userspace build
endif

build_drivers:
ifeq ($(BUILD_DRIVERS), 1)
	make $(MAKE_QUIET_FLAG) -C Drivers build
endif

build_image:
	mkdir -p iso_tmp_data
	mkdir -p initrd_tmp_data
	cp -r initrd/* initrd_tmp_data/
ifeq ($(BUILD_DRIVERS), 1)
	cp -r Drivers/out/* initrd_tmp_data/usr/lib/drivers/
endif
ifeq ($(BUILD_USERSPACE), 1)
	cp -r Userspace/out/* initrd_tmp_data/
endif
#	tar czf initrd.tar -C initrd_tmp_data/ ./ --format=ustar
	tar cf initrd.tar -C initrd_tmp_data/ ./ --format=ustar
	cp Kernel/fennix.elf initrd.tar \
		iso_tmp_data/
ifeq ($(BOOTLOADER), lynx)
	cp tools/lynx.cfg Lynx/loader.bin Lynx/efi-loader.bin iso_tmp_data/
	xorriso -as mkisofs -b loader.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot efi-loader.bin -V FENNIX \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_tmp_data -o $(OSNAME).iso
endif
ifeq ($(BOOTLOADER), limine)
	cp tools/limine.cfg $(LIMINE_FOLDER)/limine-bios.sys \
						$(LIMINE_FOLDER)/limine-bios-cd.bin \
						$(LIMINE_FOLDER)/limine-uefi-cd.bin \
						iso_tmp_data/
	mkdir -p iso_tmp_data/EFI/BOOT
	cp  $(LIMINE_FOLDER)/BOOTX64.EFI \
		$(LIMINE_FOLDER)/BOOTIA32.EFI \
		iso_tmp_data/EFI/BOOT/
	xorriso -as mkisofs -quiet -b limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot limine-uefi-cd.bin -V FENNIX \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_tmp_data -o $(OSNAME).iso
endif
ifeq ($(BOOTLOADER), grub)
# TODO: Add custom language support for GRUB or detect the system language using "echo $LANG | cut -d . -f 1" and set "lang" variable inside grub.cfg
	mkdir -p iso_tmp_data/boot
	mkdir -p iso_tmp_data/boot/grub
	cp tools/grub.cfg iso_tmp_data/boot/grub/
	grub-mkrescue -o $(OSNAME).iso iso_tmp_data
endif
ifeq ($(OSARCH), aarch64)
	$(COMPILER_PATH)/$(COMPILER_ARCH)objcopy Kernel/fennix.elf -O binary $(OSNAME).img
endif

ifeq ($(OSARCH), amd64)
QEMU_UEFI_BIOS = -bios /usr/share/qemu/OVMF.fd
endif
# ifeq ($(OSARCH), aarch64)
# QEMU_UEFI_BIOS = -bios -bios /usr/share/AAVMF/AAVMF_CODE.fd
# endif

ifeq ($(OSARCH), amd64)
QEMU_SMP_DBG = -smp $(shell echo $(shell nproc)/4 | bc)
QEMU_SMP = -smp $(shell nproc)
endif

ifeq ($(OSARCH), i386)
QEMU_SMP_DBG = -smp $(shell echo $(shell nproc)/4 | bc)
QEMU_SMP = -smp $(shell nproc)
endif

ifeq ($(OSARCH), aarch64)
QEMU_SMP_DBG = -smp 4
QEMU_SMP = -smp 4
endif

ifeq ($(OSARCH), amd64)
QEMUHWACCELERATION = -machine q35 -enable-kvm
QEMUMEMORY = -m 4G
else ifeq ($(OSARCH), i386)
QEMUHWACCELERATION = -machine q35 -enable-kvm
QEMUMEMORY = -m 4G
else ifeq ($(OSARCH), aarch64)
QEMUHWACCELERATION =
QEMUMEMORY = -m 1G
endif

clean_logs:
	rm -f serial.log profiler.log serial3.dmp network.dmp parallel.log mouse.pcap kbd.pcap mousex.pcap kbdx.pcap

vscode_debug_only: clean_logs
	$(QEMU) -S -gdb tcp::1234 -d cpu_reset,int -no-reboot -no-shutdown $(QEMU_UEFI_BIOS) -m 512M $(QEMUFLAGS) $(QEMU_SMP_DBG)

vscode_debug: build_lynx build_kernel build_userspace build_drivers build_image vscode_debug_only

qemu: qemu_vdisk clean_logs
	touch serial.log parallel.log
	# x-terminal-emulator -e tail -f serial.log &
#	x-terminal-emulator -e tail -f parallel.log &
	$(QEMU) $(QEMU_UEFI_BIOS) -cpu host $(QEMUFLAGS) $(QEMUHWACCELERATION) $(QEMUMEMORY) $(QEMU_SMP)

qemubios: qemu_vdisk clean_logs
	$(QEMU) -cpu host $(QEMUFLAGS) $(QEMUHWACCELERATION) $(QEMUMEMORY) $(QEMU_SMP)

run: build qemu

clean: clean_logs
	rm -rf doxygen-doc iso_tmp_data initrd_tmp_data
	rm -f initrd.tar $(OSNAME).iso $(OSNAME).img
	make -C Kernel clean
	make -C Lynx clean
	make -C Userspace clean
	make -C Drivers clean
