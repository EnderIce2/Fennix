build:
	mkdir -p $(WORKSPACE_DIR)/cache/xml2
	if [ ! -f "$(WORKSPACE_DIR)/cache/xml2/.configured" ]; then \
		cd $(WORKSPACE_DIR)/cache/xml2 && \
		$(CURDIR)/libxml2-src/configure \
			--host=x86_64-none-elf \
			--prefix=/usr \
			--libdir=/usr/lib \
			--enable-static \
			--disable-shared \
			--without-python \
			--without-debug \
			--without-docbook \
			--without-ftp \
			--without-http \
			--without-html \
			--without-legacy \
			--without-pattern \
			--without-push \
			--without-regexps \
			--without-sax1 \
			--without-schemas \
			--without-schematron \
			--without-threads \
			--without-valid \
			--without-xinclude \
			--without-xpath \
			--without-xptr \
			--without-modules \
			--without-zlib \
			--without-lzma \
			--without-coverage \
			CC="$(CC)" \
			CXX="$(CXX)" \
			AR="$(AR)" \
			CFLAGS="-nostdinc -I$(WORKSPACE_DIR)/out/include" \
			CXXFLAGS="-nostdinc++ -I$(WORKSPACE_DIR)/out/include" \
			LDFLAGS="-nostdlib -L$(WORKSPACE_DIR)/out/lib -lc" && \
		touch .configured; \
	fi
	$(MAKE) -C $(WORKSPACE_DIR)/cache/xml2 -j$(shell nproc)
	$(MAKE) -C $(WORKSPACE_DIR)/cache/xml2 install DESTDIR=$(WORKSPACE_DIR)/out
	cd $(WORKSPACE_DIR)/cache/xml2 && \
	./libtool --finish $(WORKSPACE_DIR)/out/usr/lib

clean:
	$(info Nothing to clean)
