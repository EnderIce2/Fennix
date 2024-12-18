FROM ubuntu:22.04 AS base

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /fennix
ADD . /fennix

RUN apt -y update
RUN apt -y install \
	build-essential \
	bison \
	flex \
	libgmp3-dev \
	libmpc-dev \
	libmpfr-dev \
	texinfo \
	libzstd-dev \
	libisl-dev \
	autoconf \
	m4 \
	automake \
	gettext \
	gperf \
	dejagnu \
	guile-3.0 \
	guile-3.0-dev \
	expect \
	tcl \
	autogen \
	tex-common \
	sphinx-common \
	git \
	ssh \
	diffutils \
	patch

RUN apt clean && rm -rf /var/lib/apt/lists

RUN make -C tools __clone_all_no_qemu
RUN make --quiet -C tools do_binutils_64
RUN make --quiet -C tools do_binutils_32
RUN make --quiet -C tools do_gcc_64
RUN make --quiet -C tools do_gcc_32
RUN cd tools && rm -rf binutils-gdb gcc build-binutils64 build-gcc64 build-binutils32 build-gcc32

RUN make build
