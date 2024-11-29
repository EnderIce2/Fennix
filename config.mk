# If the OS we are building should be compiled
# for debugging purposes.
DEBUG = 1

# Operating system name.
OSNAME = Fennix

# OS architecture: amd64, i386, aarch64
OSARCH = amd64

# Kernel version.
KERNEL_VERSION = dev

# Which bootloader to use.
# Available bootloaders:
#   - grub     - GRUB
#   - limine   - Limine
BOOTLOADER = grub

BUILD_KERNEL = 1
# BUILD_USERSPACE = 1
# BUILD_DRIVERS = 1

QUIET_BUILD = 1

# The path of the cross-compiler.
#   Default: $(CURDIR)/tools/cross/
COMPILER_PATH := $(CURDIR)/tools/cross/

# Qemu path. If you want to use the one
# you have installed in your system, change
# it to /usr/bin/qemu-system-
# (do not include x86_64 or i386, it will be
# added automatically depending on the OSARCH)
#   Default: $(CURDIR)/tools/cross/
__CONF_QEMU_PATH := $(CURDIR)/tools/cross/

# Set libc to use. Available options:
#   - internal  - Use the internal libc
USE_LIBC = internal

# Build all libraries as static libraries.
USERSPACE_STATIC_LIBS = 0









# Make related variables
# -----------------------
# Do not change anything below this line unless
# you know what you are doing.

ifeq ($(OSARCH), amd64)
COMPILER_ARCH = x86_64
__CONF_QEMU_PATH := $(__CONF_QEMU_PATH)bin/qemu-system-x86_64
else ifeq ($(OSARCH), i386)
COMPILER_ARCH = i386
__CONF_QEMU_PATH := $(__CONF_QEMU_PATH)bin/qemu-system-i386
else ifeq ($(OSARCH), aarch64)
COMPILER_ARCH = aarch64
__CONF_QEMU_PATH := $(__CONF_QEMU_PATH)bin/qemu-system-aarch64
endif

export __CONF_QEMU_PATH

__CONF_CC := $(COMPILER_PATH)/bin/$(COMPILER_ARCH)-fennix-gcc
__CONF_CPP := $(COMPILER_PATH)/bin/$(COMPILER_ARCH)-fennix-g++
__CONF_LD := $(COMPILER_PATH)/bin/$(COMPILER_ARCH)-fennix-ld
__CONF_AS := $(COMPILER_PATH)/bin/$(COMPILER_ARCH)-fennix-as
__CONF_NM := $(COMPILER_PATH)/bin/$(COMPILER_ARCH)-fennix-nm
__CONF_OBJCOPY := $(COMPILER_PATH)/bin/$(COMPILER_ARCH)-fennix-objcopy
__CONF_OBJDUMP := $(COMPILER_PATH)/bin/$(COMPILER_ARCH)-fennix-objdump
__CONF_GDB := $(COMPILER_PATH)/bin/$(COMPILER_ARCH)-fennix-gdb

export __CONF_CC
export __CONF_CPP
export __CONF_LD
export __CONF_AS
export __CONF_NM
export __CONF_OBJCOPY
export __CONF_OBJDUMP
export __CONF_GDB

export DEBUG
export OSNAME
export OSARCH
export KERNEL_VERSION

undefine COMPILER_PATH
undefine COMPILER_ARCH
