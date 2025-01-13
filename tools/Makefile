WORKING_DIR = $(CURDIR)
CROSS_DIR=$(WORKING_DIR)/cross
export PATH := $(CROSS_DIR):$(PATH)

BINUTILS_TAG = binutils-2_43_1
GCC_TAG = releases/gcc-14.2.0
QEMU_TAG = v9.2.0
GRUB_TAG = grub-2.12

default:
	$(error Please specify a target)

all: do_tools do_limine __clone_all do_binutils do_gcc do_qemu

clean:
	rm -f rep ep
	rm -rf binutils-gdb
	rm -rf gcc
	rm -rf qemu

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
	git clone --depth 1 -b ${QEMU_TAG} --single-branch https://gitlab.com/qemu-project/qemu.git qemu
else
	$(info > TOOLS: Reseting qemu...)
	cd qemu && \
	git fetch origin && \
	git reset --hard ${QEMU_TAG} && \
	git clean -dfx
	$(info > TOOLS: Operation completed for qemu)
endif

.PHONY: all clean __clone_binutils __clone_gcc __prep_cross __patch_cross __patch_cross_binutils __patch_cross_gcc

__clone_binutils:
ifeq ("$(wildcard ./binutils-gdb)", "")
	git clone --depth 1 -b $(BINUTILS_TAG) --single-branch git://sourceware.org/git/binutils-gdb.git binutils-gdb
else
	$(info > TOOLS: Reseting binutils-gdb...)
	cd binutils-gdb && \
	git fetch origin && \
	git reset --hard $(BINUTILS_TAG) && \
	git clean -dfx
	$(info > TOOLS: Operation completed for binutils-gdb)
endif

__clone_gcc:
ifeq ("$(wildcard ./gcc)", "")
	git clone --depth 1 -b $(GCC_TAG) --single-branch git://gcc.gnu.org/git/gcc.git gcc
else
	$(info > TOOLS: Reseting gcc...)
	cd gcc && \
	git fetch origin && \
	git reset --hard $(GCC_TAG) && \
	git clean -dfx
	$(info > TOOLS: Operation completed for gcc)
endif

__clone_grub:
ifeq ("$(wildcard ./grub)", "")
	git clone --depth 1 -b $(GRUB_TAG) https://git.savannah.gnu.org/git/grub.git grub
else
	$(info > TOOLS: Reseting grub...)
	cd grub && \
	git fetch origin && \
	git reset --hard $(GRUB_TAG) && \
	git clean -dfx
	$(info > TOOLS: Operation completed for grub)
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

__patch_cross_grub: __clone_grub
	$(info > TOOLS: Patching grub)
	cd grub && git apply ../grub.patch

__patch_cross_qemu: __clone_qemu
	$(info > TOOLS: Patching qemu)
	cd qemu && git apply ../qemu.patch

__patch_cross: __patch_cross_binutils __patch_cross_gcc __patch_cross_grub

__prep_cross:
	mkdir -p cross
	mkdir -p binutils-gdb/__build
	mkdir -p gcc/__build

__clone_all_no_qemu: __clone_grub __clone_binutils __clone_gcc

__clone_all: __clone_qemu __clone_all_no_qemu

cross_dev:
	$(MAKE) __clone_binutils
	$(MAKE) __clone_gcc
	$(MAKE) __clone_qemu
	$(MAKE) __patch_cross_binutils
	$(MAKE) __patch_cross_gcc
	$(MAKE) __patch_cross_qemu

update_patches:
ifeq ("$(wildcard ./binutils-gdb.patch.bk)", "")
	mv binutils-gdb.patch binutils-gdb.patch.bk
endif
ifeq ("$(wildcard ./gcc.patch.bk)", "")
	mv gcc.patch gcc.patch.bk
endif
ifeq ("$(wildcard ./qemu.patch.bk)", "")
	mv qemu.patch qemu.patch.bk
endif
ifeq ("$(wildcard ./grub.patch.bk)", "")
	mv grub.patch grub.patch.bk
endif
# binutils-gdb
	cd binutils-gdb && \
	git add . && \
	git diff --cached > ../binutils-gdb.patch
# gcc
	cd gcc && \
	git add . && \
	git diff --cached > ../gcc.patch
# qemu
	cd qemu && \
	git add . && \
	git diff --cached > ../qemu.patch
# grub
	cd grub && \
	git add . && \
	git diff --cached > ../grub.patch

do_qemu:
	$(MAKE) __patch_cross_qemu
	$(MAKE) __prep_cross
	cd qemu && \
	bash ./configure --target-list=x86_64-softmmu,i386-softmmu,arm-softmmu,aarch64-softmmu \
	--prefix="$(CROSS_DIR)" \
	--enable-gtk --disable-tools \
	--disable-gio --disable-virtfs --disable-vnc \
	--disable-opengl && \
	make --quiet -j$(shell nproc) && make --quiet install

__do_binutils_x86_64:
	mkdir -p binutils-gdb/__build
	cd binutils-gdb/__build && \
	../configure --target=x86_64-fennix \
	--prefix="$(CROSS_DIR)" --disable-nls --quiet \
	--with-sysroot --enable-shared --disable-werror && \
	make --quiet all -j$(shell nproc) && \
	make --quiet install

__do_binutils_i386:
	mkdir -p binutils-gdb/__build
	cd binutils-gdb/__build && \
	../configure --target=i386-fennix \
	--prefix="$(CROSS_DIR)" --disable-nls --quiet \
	--with-sysroot --enable-shared --disable-werror && \
	make --quiet all -j$(shell nproc) && \
	make --quiet install

__do_binutils_arm:
	mkdir -p binutils-gdb/__build
	cd binutils-gdb/__build && \
	../configure --target=arm-fennix \
	--prefix="$(CROSS_DIR)" --disable-nls --quiet \
	--with-sysroot --enable-shared --disable-werror && \
	make --quiet all -j$(shell nproc) && \
	make --quiet install

__do_binutils_aarch64:
	mkdir -p binutils-gdb/__build
	cd binutils-gdb/__build && \
	../configure --target=aarch64-fennix \
	--prefix="$(CROSS_DIR)" --disable-nls --quiet \
	--with-sysroot --enable-shared --disable-werror && \
	make --quiet all -j$(shell nproc) && \
	make --quiet install

__reset_binutils:
	$(MAKE) __patch_cross_binutils
	$(MAKE) __prep_cross

do_binutils:
	$(MAKE) __reset_binutils
	$(MAKE) __do_binutils_x86_64
	$(MAKE) __reset_binutils
	$(MAKE) __do_binutils_i386
	$(MAKE) __reset_binutils
	$(MAKE) __do_binutils_arm
	$(MAKE) __reset_binutils
	$(MAKE) __do_binutils_aarch64

__do_gcc_x86_64:
	mkdir -p gcc/__build
	cd gcc/__build && \
	../configure --target=x86_64-fennix --quiet \
	--prefix="$(CROSS_DIR)" --disable-nls --enable-default-pie \
	--enable-languages=c,c++ --enable-shared --without-headers && \
	make --quiet all-gcc -j$(shell nproc) && \
	make --quiet all-target-libgcc -j$(shell nproc) && \
	make --quiet install-gcc -j$(shell nproc) && \
	make --quiet install-target-libgcc -j$(shell nproc)

__do_gcc_i386:
	mkdir -p gcc/__build
	cd gcc/__build && \
	../configure --target=i386-fennix --quiet \
	--prefix="$(CROSS_DIR)" --disable-nls --enable-default-pie \
	--enable-languages=c,c++ --enable-shared --without-headers && \
	make --quiet all-gcc -j$(shell nproc) && \
	make --quiet all-target-libgcc -j$(shell nproc) && \
	make --quiet install-gcc -j$(shell nproc) && \
	make --quiet install-target-libgcc -j$(shell nproc)

__do_gcc_arm:
	mkdir -p gcc/__build
	cd gcc/__build && \
	../configure --target=arm-fennix --quiet \
	--prefix="$(CROSS_DIR)" --disable-nls --enable-default-pie \
	--enable-languages=c,c++ --enable-shared --without-headers && \
	make --quiet all-gcc -j$(shell nproc) && \
	make --quiet all-target-libgcc -j$(shell nproc) && \
	make --quiet install-gcc -j$(shell nproc) && \
	make --quiet install-target-libgcc -j$(shell nproc)

__do_gcc_aarch64:
	cd gcc/__build && \
	../configure --target=aarch64-fennix --quiet \
	--prefix="$(CROSS_DIR)" --disable-nls --enable-default-pie \
	--enable-languages=c,c++ --enable-shared --without-headers && \
	make --quiet all-gcc -j$(shell nproc) && \
	make --quiet all-target-libgcc -j$(shell nproc) && \
	make --quiet install-gcc -j$(shell nproc) && \
	make --quiet install-target-libgcc -j$(shell nproc)

__reset_gcc:
	$(MAKE) __patch_cross_gcc
	$(MAKE) __prep_cross

do_gcc:
	$(MAKE) __reset_gcc
	$(MAKE) __do_gcc_x86_64
	$(MAKE) __reset_gcc
	$(MAKE) __do_gcc_i386
	$(MAKE) __reset_gcc
	$(MAKE) __do_gcc_arm
	$(MAKE) __reset_gcc
	$(MAKE) __do_gcc_aarch64

__do_grub_x86_64:
	cd grub && ./bootstrap
	mkdir -p grub/__build
	cd grub/__build && \
	../configure --target=x86_64-linux \
	--disable-nls && \
	make all -j$(shell nproc) && \
	make install

__do_grub_i386:
	cd grub && ./bootstrap
	mkdir -p grub/__build
	cd grub/__build && \
	../configure --target=i386-linux \
	--disable-nls && \
	make all -j$(shell nproc) && \
	make install

__do_grub_arm:
	cd grub && ./bootstrap
	mkdir -p grub/__build
	cd grub/__build && \
	../configure --target=arm-linux \
	--disable-nls && \
	make all -j$(shell nproc) && \
	make install

__do_grub_aarch64:
	cd grub && ./bootstrap
	mkdir -p grub/__build
	cd grub/__build && \
	../configure --target=aarch64-linux \
	--disable-nls && \
	make all -j$(shell nproc) && \
	make install

__reset_grub:
	$(MAKE) __patch_cross_grub
	$(MAKE) __prep_cross

do_grub:
	$(MAKE) __reset_grub
	$(MAKE) __do_grub_x86_64
	$(MAKE) __reset_grub
	$(MAKE) __do_grub_i386
	$(MAKE) __reset_grub
	$(MAKE) __do_grub_arm
	$(MAKE) __reset_grub
	$(MAKE) __do_grub_aarch64
