build:
	cp -r include/* ../out/include
	cp ../Kernel/include/interface/syscalls.h ../out/include/fennix/syscall.h
	make -C runtime build
	make -C src build
	make -C ElfInterpreter build

clean:
	make -C runtime clean
	make -C src clean
	make -C ElfInterpreter clean
