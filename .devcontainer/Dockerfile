FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="none"

# Optionally install the cmake for vcpkg
COPY ./reinstall-cmake.sh /tmp/

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
	chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
	fi \
	&& rm -f /tmp/reinstall-cmake.sh

# [Optional] Uncomment this section to install additional vcpkg ports.
# RUN su vscode -c "${VCPKG_ROOT}/vcpkg install <your-port-name-here>"

# [Optional] Uncomment this section to install additional packages.
# RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
#     && apt-get -y install --no-install-recommends <your-package-list-here>

# Required packages for building gcc & binutils
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive && \
	apt-get -y install --no-install-recommends \
	build-essential \
	bison \
	flex

# Required packages for building gcc & binutils
RUN wget https://launchpad.net/ubuntu/+archive/primary/+files/autoconf_2.69-11.1_all.deb -O /tmp/autoconf.deb && \
	sudo dpkg --force-all -i /tmp/autoconf.deb

# Required packages for building gcc & binutils
RUN wget https://ftp.gnu.org/gnu/automake/automake-1.15.1.tar.gz -O /tmp/automake.tar.gz && \
	tar -xzf /tmp/automake.tar.gz -C /tmp && \
	cd /tmp/automake-1.15.1 && \
	./configure && make && sudo make install

# Required packages for building qemu
RUN apt-get update && export DEBIAN_FRONTEND=noninteractive && \
	apt-get -y install --no-install-recommends \
	git \
	libglib2.0-dev \
	libfdt-dev \
	libpixman-1-dev \
	zlib1g-dev \
	ninja-build \
	libaio-dev \
	libbluetooth-dev \
	libcapstone-dev \
	libbrlapi-dev \
	libbz2-dev \
	libcap-ng-dev \
	libcurl4-gnutls-dev \
	libgtk-3-dev \
	libibverbs-dev \
	libjpeg8-dev \
	libncurses5-dev \
	libnuma-dev \
	librbd-dev \
	librdmacm-dev \
	libsasl2-dev \
	libsdl2-dev \
	libseccomp-dev \
	libsnappy-dev \
	libssh-dev \
	libvde-dev \
	libvdeplug-dev \
	libvte-2.91-dev \
	libxen-dev \
	liblzo2-dev \
	valgrind \
	xfslibs-dev \
	libnfs-dev \
	libiscsi-dev \
	python3-venv

RUN apt-get update && export DEBIAN_FRONTEND=noninteractive && \
	apt-get -y install --no-install-recommends \
	mingw-w64
