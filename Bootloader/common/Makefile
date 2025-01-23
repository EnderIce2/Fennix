define find-sources
	$(shell find ./ -type f -name '$1' -print0 | xargs -0)
endef

BMP_SOURCES := $(call find-sources,*.bmp)
PSF_SOURCES := $(call find-sources,*.psf)
S_SOURCES := $(call find-sources,*.S)
s_SOURCES := $(call find-sources,*.s)
C_SOURCES := $(call find-sources,*.c)
CXX_SOURCES := $(call find-sources,*.cpp)

OBJ = $(BMP_SOURCES:.bmp=.o) $(PSF_SOURCES:.psf=.o) $(s_SOURCES:.s=.o) $(S_SOURCES:.S=.o) $(C_SOURCES:.c=.o) $(CXX_SOURCES:.cpp=.o)

default:
	$(error Do not run this Makefile directly!)

build:

clean:
