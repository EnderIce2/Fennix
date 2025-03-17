export DOCKER_ROOT = $(CURDIR)/docker-root
DOCKER_IMAGE_NAME = fennix-userspace

export CMAKE_INSTALL_PREFIX := $(DOCKER_ROOT)
export CMAKE_SYSROOT := $(DOCKER_ROOT)

export LDFLAGS := --sysroot=$(DOCKER_ROOT) \
	-ggdb3 -O0
export CFLAGS := \
	--sysroot=$(DOCKER_ROOT) \
	-I$(DOCKER_ROOT)/include \
	-DDEBUG -ggdb3 -O0 -fdiagnostics-color=always

create_docker_out:
	rm -rf cache
	rm -rf $(DOCKER_ROOT)
	mkdir -p $(DOCKER_ROOT)
	mkdir -p $(DOCKER_ROOT)/bin
	mkdir -p $(DOCKER_ROOT)/etc
	mkdir -p $(DOCKER_ROOT)/lib
	mkdir -p $(DOCKER_ROOT)/lib64
	mkdir -p $(DOCKER_ROOT)/include/linux
	mkdir -p $(DOCKER_ROOT)/usr/bin
	mkdir -p $(DOCKER_ROOT)/usr/share/doc
	mkdir -p $(DOCKER_ROOT)/usr/share/info
	mkdir -p $(DOCKER_ROOT)/usr/include
	mkdir -p $(DOCKER_ROOT)/usr/lib

build_docker_coreutils:
	mkdir -p cache/coreutils
	cd cache/coreutils && \
	cmake $(CURDIR)/../coreutils \
	-DCMAKE_INSTALL_PREFIX:PATH=$(DOCKER_ROOT) \
	-DCMAKE_SYSROOT=$(DOCKER_ROOT) \
	-DCMAKE_C_STANDARD_INCLUDE_DIRECTORIES=$(DOCKER_ROOT)/include \
	-DCMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES=$(DOCKER_ROOT)/include \
	&& \
	make -j$(shell nproc) && \
	make install

build_docker_libc:
	mkdir -p cache/libc
	cd cache/libc && \
	cmake $(CURDIR)/../libc \
	-DCMAKE_INSTALL_PREFIX=$(DOCKER_ROOT) \
	-DCMAKE_SYSROOT=$(DOCKER_ROOT) \
	-DCMAKE_INSTALL_INCLUDEDIR=include \
	-DCMAKE_INSTALL_LIBDIR=lib \
	-DCMAKE_INSTALL_BINDIR=bin && \
	make -j$(shell nproc) && \
	make install

docker_image:
	docker build -t $(DOCKER_IMAGE_NAME) -f Dockerfile .

docker_run:
	@echo "Running container..."
	@docker run -it --name fennix-userspace-instance fennix-userspace:latest /bin/sh || true
	@echo "\nExit code: $$(docker inspect fennix-userspace-instance --format='{{.State.ExitCode}}')"
	@docker rm fennix-userspace-instance >/dev/null 2>&1

docker: create_docker_out
	$(MAKE) build_docker_libc
	$(MAKE) build_docker_coreutils

clean:
	rm -rf cache
	rm -rf $(DOCKER_ROOT)
