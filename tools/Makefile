WORKING_DIR = $(CURDIR)
CROSS_DIR=$(WORKING_DIR)/cross
export PATH := $(CROSS_DIR):$(PATH)

QEMU_VERSION = 9.1.2

default:
	$(error Please specify a target)

all: do_tools do_limine __clone_all do_binutils do_gcc do_qemu

clean:
	rm -f rep ep

do_tools:
	gcc -w ReadEthernetPackets.c -o rep
	g++ -w ErrorParser.cpp -o ep
	chmod +x rep
	chmod +x ep

do_limine:
ifeq ("$(wildcard ./limine)", "")
	git clone https://github.com/limine-bootloader/limine.git --branch=v6.x-branch-binary --depth=1 limine
else
	$(info > TOOLS: Skipping cloning limine because directory already exists.)
endif

__clone_qemu:
ifeq ("$(wildcard ./qemu)", "")
	wget https://download.qemu.org/qemu-${QEMU_VERSION}.tar.xz
	tar xvJf qemu-${QEMU_VERSION}.tar.xz
	rm -f qemu-${QEMU_VERSION}.tar.xz
	mv qemu-${QEMU_VERSION} qemu
else
	$(info > TOOLS: Skipping downloading qemu because directory already exists.)
endif

.PHONY: all clean __clone_binutils __clone_gcc __prep_cross __patch_cross __patch_cross_binutils __patch_cross_gcc

__clone_binutils:
ifeq ("$(wildcard ./binutils-gdb)", "")
	git clone --depth 1 -b binutils-2_43_1 --single-branch git://sourceware.org/git/binutils-gdb.git binutils-gdb
else
	$(info > TOOLS: Reseting binutils-gdb...)
	cd binutils-gdb && \
	git fetch origin && \
	git reset --hard binutils-2_43_1 && \
	git clean -dfx
	$(info > TOOLS: Operation completed for binutils-gdb)
endif

__clone_gcc:
ifeq ("$(wildcard ./gcc)", "")
	git clone --depth 1 -b releases/gcc-14.2.0 --single-branch git://gcc.gnu.org/git/gcc.git gcc
else
	$(info > TOOLS: Reseting gcc...)
	cd gcc && \
	git fetch origin && \
	git reset --hard releases/gcc-14.2.0 && \
	git clean -dfx
	$(info > TOOLS: Operation completed for gcc)
endif

__patch_cross_binutils: __clone_binutils
	$(info > TOOLS: Patching binutils-gcc)
	cd binutils-gdb && git apply ../binutils-gdb.patch
	$(info > TOOLS: Running automake for binutils-gdb/ld)
	cd binutils-gdb/ld && automake

__patch_cross_gcc: __clone_gcc
	$(info > TOOLS: Patching gcc)
	cd gcc && git apply ../gcc.patch
	cd gcc && ./contrib/download_prerequisites
	$(info > TOOLS: Running autoconf for gcc/libstdc++-v3)
	cd gcc/libstdc++-v3 && autoconf

__patch_cross: __patch_cross_binutils __patch_cross_gcc

__prep_cross:
	mkdir -p cross
	mkdir -p binutils-gdb/__build
	mkdir -p gcc/__build

__clone_all_no_qemu: __clone_binutils __clone_gcc

__clone_all: __clone_qemu __clone_all_no_qemu

do_qemu: __prep_cross
	cd qemu && \
	bash ./configure --target-list=x86_64-softmmu,i386-softmmu,aarch64-softmmu \
	--prefix="$(CROSS_DIR)" \
	--enable-gtk --disable-tools \
	--disable-gio --disable-virtfs --disable-vnc \
	--disable-opengl && \
	make --quiet -j$(shell nproc) && make --quiet install

do_binutils:
# x86_64
	$(MAKE) __patch_cross_binutils
	$(MAKE) __prep_cross
	cd binutils-gdb/__build && \
	../configure --target=x86_64-fennix \
	--prefix="$(CROSS_DIR)" --disable-nls --quiet \
	--with-sysroot --enable-shared --disable-werror && \
	make --quiet all -j$(shell nproc) && \
	make --quiet install
# i386
	$(MAKE) __patch_cross_binutils
	$(MAKE) __prep_cross
	cd binutils-gdb/__build && \
	../configure --target=i386-fennix \
	--prefix="$(CROSS_DIR)" --disable-nls --quiet \
	--with-sysroot --enable-shared --disable-werror && \
	make --quiet all -j$(shell nproc) && \
	make --quiet install

do_gcc:
# x86_64
	$(MAKE) __patch_cross_gcc
	$(MAKE) __prep_cross
	cd gcc/__build && \
	../configure --target=x86_64-fennix --quiet \
	--prefix="$(CROSS_DIR)" --disable-nls --enable-default-pie \
	--enable-languages=c,c++ --enable-shared --without-headers && \
	make --quiet all-gcc -j$(shell nproc) && \
	make --quiet all-target-libgcc -j$(shell nproc) && \
	make --quiet install-gcc -j$(shell nproc) && \
	make --quiet install-target-libgcc -j$(shell nproc)
# i386
	$(MAKE) __patch_cross_gcc
	$(MAKE) __prep_cross
	cd gcc/__build && \
	../configure --target=i386-fennix --quiet \
	--prefix="$(CROSS_DIR)" --disable-nls --enable-default-pie \
	--enable-languages=c,c++ --enable-shared --without-headers && \
	make --quiet all-gcc -j$(shell nproc) && \
	make --quiet all-target-libgcc -j$(shell nproc) && \
	make --quiet install-gcc -j$(shell nproc) && \
	make --quiet install-target-libgcc -j$(shell nproc)
