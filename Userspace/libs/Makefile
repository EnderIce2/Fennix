build:
	cp -a $(CURDIR)/include/. $(WORKSPACE_DIR)/out/include
	make -C libgcc build
	make -C libm build
	make -C libexample build

clean:
	make -C libgcc clean
	make -C libm clean
	make -C libexample clean
