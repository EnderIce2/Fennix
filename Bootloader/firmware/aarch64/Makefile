define find-sources
	$(shell find ./ -type f -name '$1' $(shell echo $(foreach board,$(filter-out $(BOARD_TYPE),$(AVAILABLE_BOARDS)), -not -path \"./$(board)/*\")) -print0 | xargs -0)
endef

S_SOURCES := $(call find-sources,*.S)
s_SOURCES := $(call find-sources,*.s)
C_SOURCES := $(call find-sources,*.c)

define find-common-sources
	$(shell find ../../common -type f -name '$1' -print0 | xargs -0)
endef

C_COMMON_SOURCES := $(call find-common-sources,*.c)

OBJ = $(s_SOURCES:.s=.o) $(S_SOURCES:.S=.o) $(C_SOURCES:.c=.o) $(C_COMMON_SOURCES:.c=.o)

LDFLAGS = -static -nostdlib -nodefaultlibs -nolibc \
	-Wl,-static,--no-dynamic-linker,-ztext 	\
	-zmax-page-size=0x1000					\
	-Wl,-Map boot.map -fno-pic -fno-pie

CFLAGS :=										\
	$(INCLUDE_DIR)								\
	-D__kernel__='1'							\
	-DGIT_COMMIT='"$(GIT_COMMIT)"'				\
	-DGIT_COMMIT_SHORT='"$(GIT_COMMIT_SHORT)"'	\
	-fno-pic -fno-pie -fno-builtin -I../../include

CFLAGS += -mcmodel=large
LDFLAGS += -T$(BOARD_TYPE)/linker.ld

ifeq ($(DEBUG), 1)
	CFLAGS += -DDEBUG -ggdb3 -O0 -fdiagnostics-color=always
#	CFLAGS += -fsanitize=undefined
	CFLAGS += -fstack-check -fverbose-asm
	LDFLAGS += -ggdb3 -O0
	ASFLAGS += -g --gstabs+ --gdwarf-5 -D
endif # DEBUG

default:
	$(error Do not run this Makefile directly!)

build: boot.bin

boot.bin: $(OBJ)
	$(info Linking $@)
	$(CC) $(LDFLAGS) $(OBJ) -o tmp.elf
	$(OBJCOPY) tmp.elf -O binary ../../boot.bin
#	rm tmp.elf

%.o: %.c $(HEADERS)
	$(info Compiling $<)
	$(CC) $(CFLAGS) -fstack-protector-all -std=c17 -c $< -o $@

%.o: %.S
	$(info Compiling $<)
	$(AS) $(ASFLAGS) -c $< -o $@

%.o: %.s
	$(info Compiling $<)
	$(AS) $(ASFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) boot.map
