build:
	cp -a $(CURDIR)/include/. $(WORKSPACE_DIR)/out/include
	make -C libgcc build
	make -C libdemo build

clean:
	make -C libgcc clean
	make -C libdemo clean
