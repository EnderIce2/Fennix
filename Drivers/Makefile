default:
	$(error Do not run this Makefile directly!)

export CC := $(__CONF_CC)
export CXX := $(__CONF_CXX)
export LD := $(__CONF_LD)
export AS := $(__CONF_AS)
export OBJDUMP := $(__CONF_OBJDUMP)

export OUTPUT_DIR := $(CURDIR)/out/
export INCLUDE_DIR := $(CURDIR)/include

DRIVER_LDFLAGS = -nostdlib -nodefaultlibs -nolibc -zmax-page-size=0x1000	\
			-Wl,-Map file.map -fvisibility=hidden -Wl,--dynamic-linker=/boot/fennix.elf

ifeq ($(OSARCH), amd64)
DRIVER_CFLAGS = -fPIC -fPIE -pie -mno-80387 -mno-mmx -mno-3dnow	\
		  -mno-red-zone -mno-sse -mno-sse2							\
		  -march=x86-64 -pipe -ffunction-sections					\
		  -msoft-float -fno-builtin

else ifeq ($(OSARCH), i386)
DRIVER_CFLAGS = -fPIC -fPIE -pie -mno-80387 -mno-mmx -mno-3dnow	\
		  -mno-red-zone -mno-sse -mno-sse2 -ffunction-sections		\
		  -march=i386 -pipe -msoft-float -fno-builtin
else ifeq ($(OSARCH), aarch64)
DRIVER_CFLAGS = -pipe -fno-builtin -fPIC
endif

DRIVER_CFLAGS += -I$(CURDIR)/include

ifeq ($(DEBUG), 1)
	DRIVER_CFLAGS += -DDEBUG -ggdb3 -O0 -fdiagnostics-color=always -fstack-usage
ifeq ($(OSARCH), amd64)
	DRIVER_CFLAGS += -fverbose-asm
endif
ifneq ($(OSARCH), aarch64)
	DRIVER_CFLAGS += -fstack-check
endif
	DRIVER_LDFLAGS += -ggdb3 -O0
endif

export DRIVER_LDFLAGS
export DRIVER_CFLAGS

build:
	cp -rf ../Kernel/include/interface/* include/
	mkdir -p out
	make -C library build
	make -C audio build
	make -C input build
	make -C misc build
	make -C network build
	make -C storage build
	make -C filesystem build

prepare:
	$(info Nothing to prepare)

clean:
	rm -rf out
	make -C library clean
	make -C audio clean
	make -C input clean
	make -C misc clean
	make -C network clean
	make -C storage clean
	make -C filesystem clean
