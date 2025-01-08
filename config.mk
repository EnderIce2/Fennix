# If the OS we are building should be compiled
# for debugging purposes.
DEBUG = 1

# Operating system name.
OSNAME = Fennix

# OS architecture, check AVAILABLE_ARCHS for available options.
OSARCH = amd64

# Kernel version.
KERNEL_VERSION = dev

# Which bootloader to use.
# Available bootloaders:
#   - grub     - GRUB
#   - limine   - Limine
BOOTLOADER = grub

BUILD_KERNEL = 1
BUILD_USERSPACE = 1
BUILD_DRIVERS = 1

QUIET_BUILD = 1

# The path of the cross-compiler.
#   Default: $(CURDIR)/tools/cross
COMPILER_PATH := $(CURDIR)/tools/cross

# Qemu path. If you want to use the one
# you have installed in your system, change
# it to /usr
# (do not include x86_64 or i386, it will be
# added automatically depending on the OSARCH)
#   Default: $(CURDIR)/tools/cross
__CONF_QEMU_PATH := $(CURDIR)/tools/cross

# Set libc to use. Available options:
#   - internal  - Use the internal libc
USE_LIBC = internal










# Make related variables
# -----------------------
# Do not change anything below this line unless
# you know what you are doing.

# Available architectures. Do not change
export AVAILABLE_ARCHS := amd64 i386 arm aarch64

ifneq ($(filter $(OSARCH),$(AVAILABLE_ARCHS)),$(OSARCH))
$(error OSARCH=$(OSARCH) is not a supported architecture. Choose one of: $(AVAILABLE_ARCHS))
endif

ARCH_MAP := amd64=x86_64 i386=i386 arm=arm aarch64=aarch64
COMPILER_ARCH := $(patsubst $(OSARCH)=%,%,$(filter $(OSARCH)=%,$(ARCH_MAP)))
__CONF_QEMU_PATH := $(__CONF_QEMU_PATH)/bin/qemu-system-$(COMPILER_ARCH)
TOOLCHAIN_PREFIX := $(COMPILER_PATH)/bin/$(COMPILER_ARCH)-fennix-

export __CONF_CC := $(TOOLCHAIN_PREFIX)gcc
export __CONF_CXX := $(TOOLCHAIN_PREFIX)g++
export __CONF_LD := $(TOOLCHAIN_PREFIX)ld
export __CONF_AS := $(TOOLCHAIN_PREFIX)as
export __CONF_AR := $(TOOLCHAIN_PREFIX)ar
export __CONF_NM := $(TOOLCHAIN_PREFIX)nm
export __CONF_OBJCOPY := $(TOOLCHAIN_PREFIX)objcopy
export __CONF_OBJDUMP := $(TOOLCHAIN_PREFIX)objdump
export __CONF_GDB := $(TOOLCHAIN_PREFIX)gdb

export DEBUG
export OSNAME
export OSARCH
export KERNEL_VERSION
