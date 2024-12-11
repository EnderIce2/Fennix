default:
	$(error Do not run this Makefile directly!)

S_SOURCES = $(shell find ./ -type f -name '*.S')
C_SOURCES = $(shell find ./ -type f -name '*.c')
CXX_SOURCES = $(shell find ./ -type f -name '*.cpp')
HEADERS = $(sort $(dir $(wildcard ../../include/*)))
OBJ = $(C_SOURCES:.c=.o) $(CXX_SOURCES:.cpp=.o) $(S_SOURCES:.S=.o)
STACK_USAGE_OBJ = $(C_SOURCES:.c=.su) $(CXX_SOURCES:.cpp=.su)

FILENAME = $(notdir $(shell pwd)).drv

build: $(FILENAME)
	mv $(FILENAME) $(OUTPUT_DIR)$(FILENAME)

$(FILENAME): $(OBJ)
	$(info Linking $@)
	$(CC) $(DRIVER_LDFLAGS) $(OBJ) $(OUTPUT_DIR)dcrt0.o -L$(OUTPUT_DIR) -lkernel -o $@

WARNCFLAG = -Wall -Wextra

%.o: %.c $(HEADERS)
	$(info Compiling $<)
	$(CC) $(DRIVER_CFLAGS) $(WARNCFLAG) -std=c17 -c $< -o $@

%.o: %.cpp $(HEADERS)
	$(info Compiling $<)
	$(CXX) $(DRIVER_CFLAGS) $(WARNCFLAG) -std=c++20 -fno-exceptions -fno-rtti -c $< -o $@

%.o: %.S
	$(info Compiling $<)
	$(AS) -o $@ $<

clean:
	rm -f file.map $(OBJ) $(STACK_USAGE_OBJ)
