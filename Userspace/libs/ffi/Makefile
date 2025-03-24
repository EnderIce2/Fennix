build:
	mkdir -p $(WORKSPACE_DIR)/cache/ffi
	if [ ! -f "$(WORKSPACE_DIR)/cache/ffi/.configured" ]; then \
		cd $(WORKSPACE_DIR)/cache/ffi && \
		$(CURDIR)/libffi-src/configure \
			--host=x86_64-none-elf \
			--prefix=/usr \
			--libdir=/usr/lib \
			--enable-static \
			--disable-shared \
			--disable-docs \
			--disable-multi-os-directory \
			CC="$(CC)" \
			CXX="$(CXX)" \
			AR="$(AR)" \
			CFLAGS="-nostdinc -I$(WORKSPACE_DIR)/out/include" \
			CXXFLAGS="-nostdinc++ -I$(WORKSPACE_DIR)/out/include" \
			LDFLAGS="-nostdlib -L$(WORKSPACE_DIR)/out/lib -lc" && \
		touch .configured; \
	fi
	$(MAKE) -C $(WORKSPACE_DIR)/cache/ffi -j$(shell nproc)
	$(MAKE) -C $(WORKSPACE_DIR)/cache/ffi install DESTDIR=$(WORKSPACE_DIR)/out
	cd $(WORKSPACE_DIR)/cache/ffi && \
	./libtool --finish $(WORKSPACE_DIR)/out/usr/lib

clean:
	$(info Nothing to clean)
