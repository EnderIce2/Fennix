export CC := $(__CONF_CC)
export CXX := $(__CONF_CXX)
export LD := $(__CONF_LD)
export AS := $(__CONF_AS)
export AR := $(__CONF_AR)
export OBJDUMP := $(__CONF_OBJDUMP)

export WORKSPACE_DIR = $(CURDIR)

ifeq ($(DEBUG), 1)

export LDFLAGS := --sysroot=$(WORKSPACE_DIR)/out/ \
	-ggdb3 -O0
export CFLAGS := \
	--sysroot=$(WORKSPACE_DIR)/out/ \
	-I$(WORKSPACE_DIR)/out/include \
	-DDEBUG -ggdb3 -O0 -fdiagnostics-color=always

else # DEBUG

export LDFLAGS := --sysroot=$(WORKSPACE_DIR)/out/
export CFLAGS := \
	--sysroot=$(WORKSPACE_DIR)/out/ \
	-I$(WORKSPACE_DIR)/out/include

endif # DEBUG

create_out:
	rm -rf out
	mkdir -p out
	mkdir -p out/bin
	mkdir -p out/lib
	mkdir -p out/include/fennix
	mkdir -p out/usr/bin
	mkdir -p out/usr/share/doc
	mkdir -p out/usr/share/info
	mkdir -p out/usr/include
	cp $(WORKSPACE_DIR)/../Kernel/include/interface/* $(WORKSPACE_DIR)/out/include/fennix/

build_coreutils:
	mkdir -p cache/coreutils
	cd cache/coreutils && \
	cmake $(WORKSPACE_DIR)/coreutils \
	-DCMAKE_INSTALL_PREFIX:PATH=$(WORKSPACE_DIR)/out \
	-DCMAKE_SYSROOT=$(WORKSPACE_DIR)/out \
	-DCMAKE_C_STANDARD_INCLUDE_DIRECTORIES=$(WORKSPACE_DIR)/out/include \
	-DCMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES=$(WORKSPACE_DIR)/out/include \
	&& \
	make -j$(shell nproc) && \
	make install

define copy_generic_header
	cp -f $(WORKSPACE_DIR)/../Kernel/include/interface/$(1) $(WORKSPACE_DIR)/libc/abis/fennix/generic/bits/$(1)
endef

build_libc:
	$(call copy_generic_header,errno.h)
	$(call copy_generic_header,syscalls.h)
	$(call copy_generic_header,fcntl.h)
	mkdir -p cache/libc
	cd cache/libc && \
	cmake $(WORKSPACE_DIR)/libc \
	-DCMAKE_INSTALL_PREFIX:PATH=$(WORKSPACE_DIR)/out \
	-DCMAKE_SYSROOT=$(WORKSPACE_DIR)/out \
	-DCMAKE_C_STANDARD_INCLUDE_DIRECTORIES=$(WORKSPACE_DIR)/out/include \
	-DTARGET_OS=fennix \
	-DTARGET_ARCH=$(OSARCH) \
	&& \
	make -j$(shell nproc) && \
	make install

build: create_out
	$(MAKE) build_libc
	make -C libs build
	$(MAKE) build_coreutils
	make -C apps build

prepare:
	$(info Nothing to prepare)

clean:
	rm -rf out cache
	mkdir -p cache
	touch cache/.gitkeep
	make -C libs clean
	make -C apps clean
	make -C docker clean
