WORKING_DIR = $(shell pwd)
CROSS_DIR=$(WORKING_DIR)/cross
export PATH := $(CROSS_DIR):$(PATH)

QEMU_VERSION = 8.1.2

all: do_rep do_ep do_limine clone_all do_binutils_64 do_gcc_64 do_binutils_32 do_gcc_32 do_qemu

clean:
	rm -f rep ep fex

do_rep:
	gcc -w ReadEthernetPackets.c -o rep
	chmod +x rep

do_ep:
	g++ -w ErrorParser.cpp -o ep
	chmod +x ep

do_limine:
	git clone https://github.com/limine-bootloader/limine.git --branch=v6.x-branch-binary --depth=1

__clone_qemu:
	wget https://download.qemu.org/qemu-${QEMU_VERSION}.tar.xz
	tar xvJf qemu-${QEMU_VERSION}.tar.xz
	rm -f qemu-${QEMU_VERSION}.tar.xz
	mv qemu-${QEMU_VERSION} qemu

__clone_all_no_qemu:
	git clone -b binutils-2_42-branch --single-branch https://github.com/Fennix-Project/binutils-gdb.git binutils-gdb
	git clone -b releases/gcc-13 --single-branch https://github.com/Fennix-Project/gcc.git gcc
	cd gcc && $(shell ./contrib/download_prerequisites)
	mkdir -p cross
	mkdir -p build-binutils64
	mkdir -p build-gcc64
	mkdir -p build-binutils32
	mkdir -p build-gcc32

clone_all: __clone_qemu __clone_all_no_qemu

do_qemu:
	cd qemu && \
	bash ./configure --target-list=x86_64-softmmu,i386-softmmu,aarch64-softmmu \
	--prefix="$(CROSS_DIR)" \
	--enable-gtk --disable-tools \
	--disable-gio --disable-virtfs --disable-vnc \
	--disable-opengl && \
	make -j$(shell nproc) && make install

do_binutils_64:
	cd build-binutils64 && \
	../binutils-gdb/configure --target=x86_64-fennix \
	--prefix="$(CROSS_DIR)" --disable-nls \
	--with-sysroot --enable-shared --disable-werror && \
	make all -j$(shell nproc) && make install

do_gcc_64:
	cd build-gcc64 && \
	../gcc/configure --target=x86_64-fennix \
	--prefix="$(CROSS_DIR)" --disable-nls --enable-default-pie \
	--enable-languages=c,c++ --enable-shared --without-headers && \
	make all-gcc -j$(shell nproc) && \
	make all-target-libgcc -j$(shell nproc) && \
	make install-gcc -j$(shell nproc) && \
	make install-target-libgcc -j$(shell nproc)

do_binutils_32:
	cd build-binutils32 && \
	../binutils-gdb/configure --target=i386-fennix \
	--prefix="$(CROSS_DIR)" --disable-nls \
	--with-sysroot --enable-shared --disable-werror && \
	make all -j$(shell nproc) && make install

do_gcc_32:
	cd build-gcc32 && \
	../gcc/configure --target=i386-fennix \
	--prefix="$(CROSS_DIR)" --disable-nls --enable-default-pie \
	--enable-languages=c,c++ --enable-shared --without-headers && \
	make all-gcc -j$(shell nproc) && \
	make all-target-libgcc -j$(shell nproc) && \
	make install-gcc -j$(shell nproc) && \
	make install-target-libgcc -j$(shell nproc)
