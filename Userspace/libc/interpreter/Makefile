default:
	$(error Do not run this Makefile directly!)

OBJECT_NAME := ld.so

OUTPUT_DIR=$(WORKSPACE_DIR)/out/lib/
SYSROOT = --sysroot=$(WORKSPACE_DIR)/out/

S_SOURCES = $(shell find ./ -type f -name '*.S')
C_SOURCES = $(shell find ./ -type f -name '*.c')
CXX_SOURCES = $(shell find ./ -type f -name '*.cpp')
OBJ = ${S_SOURCES:.S=.o} ${C_SOURCES:.c=.o} ${CXX_SOURCES:.cpp=.o}

CFLAGS := -fvisibility=hidden -fPIC -I$(WORKSPACE_DIR)/out/include -DLIBC_GIT_COMMIT='"$(shell git rev-parse HEAD)"'

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -ggdb3 -O0 -fdiagnostics-color=always
endif

build: $(OBJECT_NAME)

$(OBJECT_NAME): $(OBJ)
	$(info Linking $@)
	$(CC) -nostdlib -shared -fPIC -fPIE -fno-plt -Wl,-soname,$(OBJECT_NAME) $(SYSROOT) $(OBJ) -o $(OBJECT_NAME)
	cp $(OBJECT_NAME) $(OUTPUT_DIR)$(OBJECT_NAME)

%.o: %.c
	$(info Compiling $<)
	$(CC) $(CFLAGS) -std=c17 -c $< -o $@

%.o: %.cpp
	$(info Compiling $<)
	$(CC) $(CFLAGS) -std=c++20 -c $< -o $@

%.o: %.S
	$(info Compiling $<)
	$(AS) -c $< -o $@

clean:
	rm -f $(OBJ) $(OBJECT_NAME)
