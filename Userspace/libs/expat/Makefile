build:
	mkdir -p $(WORKSPACE_DIR)/cache/expat
	if [ ! -f "$(WORKSPACE_DIR)/cache/expat/.configured" ]; then \
		cd $(WORKSPACE_DIR)/cache/expat && \
		$(CURDIR)/libexpat-src/configure \
			--host=x86_64-none-elf \
			--prefix=/usr \
			--libdir=/usr/lib \
			--enable-static \
			--disable-shared \
			--without-docbook \
			--without-xmlwf \
			--without-examples \
			--without-tests \
			CC="$(CC)" \
			CXX="$(CXX)" \
			AR="$(AR)" \
			CFLAGS="-nostdinc -I$(WORKSPACE_DIR)/out/include" \
			CXXFLAGS="-nostdinc++ -I$(WORKSPACE_DIR)/out/include" \
			LDFLAGS="-nostdlib -L$(WORKSPACE_DIR)/out/lib -lc" && \
		touch .configured; \
	fi
	$(MAKE) -C $(WORKSPACE_DIR)/cache/expat -j$(shell nproc)
	$(MAKE) -C $(WORKSPACE_DIR)/cache/expat install DESTDIR=$(WORKSPACE_DIR)/out
	cd $(WORKSPACE_DIR)/cache/expat && \
	./libtool --finish $(WORKSPACE_DIR)/out/usr/lib

clean:
	$(info Nothing to clean)
