build:
	cp -f $(WORKSPACE_DIR)/../Kernel/include/interface/errno.h $(CURDIR)/include/errno.h
	cp -f $(WORKSPACE_DIR)/../Kernel/include/interface/syscalls.h $(CURDIR)/include/fennix/syscalls.h
	cp -a $(CURDIR)/include/. $(WORKSPACE_DIR)/out/include
	make -C interpreter build
	make -C runtime build
	make -C src build

clean:
	make -C interpreter clean
	make -C runtime clean
	make -C src clean
