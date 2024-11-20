WORKSPACE := ../../../

# Config file
include ../$(WORKSPACE)Makefile.conf

FILENAME = utest_

CFLAGS = -static -g -ggdb3 -O0

build:
	$(info Compiling $(FILENAME)linux)
	gcc linux_glibc.c $(CFLAGS) -o $(WORKSPACE)out/bin/$(FILENAME)linux
	$(info Compiling $(FILENAME)win)
	x86_64-w64-mingw32-gcc win_mingw.c $(CFLAGS) -o $(WORKSPACE)out/bin/$(FILENAME)win.exe

clean:
