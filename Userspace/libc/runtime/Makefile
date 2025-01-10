default:
	$(error Do not run this Makefile directly!)

S_SOURCES = $(shell find ./ -type f -name '*.S')
C_SOURCES = $(shell find ./ -type f -name '*.c')
CXX_SOURCES = $(shell find ./ -type f -name '*.cpp')
OBJ = ${S_SOURCES:.S=.o} ${C_SOURCES:.c=.o} ${CXX_SOURCES:.cpp=.o}

CRTBEGIN_PATH = $(shell $(CC) -print-file-name=crtbegin.o)
CRTEND_PATH = $(shell $(CC) -print-file-name=crtend.o)
CRTI_PATH = $(shell $(CC) -print-file-name=crti.o)
CRTN_PATH = $(shell $(CC) -print-file-name=crtn.o)

build: $(OBJ)
	cp $^ ../../out/lib/
	cp $(CRTBEGIN_PATH) $(CRTEND_PATH) $(CRTI_PATH) $(CRTN_PATH) $(WORKSPACE_DIR)/out/lib/

%.o: %.c
	$(info Compiling $<)
	$(CC) -nostdlib -std=c17 -DLIBC_GIT_COMMIT='"$(shell git rev-parse HEAD)"' -c $< -o $@

%.o: %.S
	$(info Compiling $<)
	$(AS) -c $< -o $@

clean:
	rm -f $(OBJ)
