MAKE_TARGETS := build clean
DIRECTORIES := $(sort $(filter-out ./include/,$(dir $(wildcard ./*/))))

FFI_VERSION = 3.4.4
XML2_VERSION = 2.12.10
EXPAT_VERSION = 2.7.0

FFI_ARCHIVE = /tmp/libffi-$(FFI_VERSION).tar.gz
XML2_ARCHIVE = /tmp/libxml2-$(XML2_VERSION).tar.xz
EXPAT_ARCHIVE = /tmp/expat-$(EXPAT_VERSION).tar.gz

.PHONY: $(MAKE_TARGETS) $(DIRECTORIES) prepare

build: copy_includes $(DIRECTORIES)
clean: $(DIRECTORIES)

copy_includes:
	cp -a $(CURDIR)/include/. $(WORKSPACE_DIR)/out/include

prepare:
	rm -rf ffi/libffi-src xml2/libxml2-src expat/libexpat-src

	mkdir -p ffi/libffi-src
	if [ ! -f $(FFI_ARCHIVE) ]; then \
		wget https://github.com/libffi/libffi/releases/download/v$(FFI_VERSION)/libffi-$(FFI_VERSION).tar.gz -O $(FFI_ARCHIVE); \
	fi
	cd ffi && \
	tar xf $(FFI_ARCHIVE) && \
	cp -r libffi-$(FFI_VERSION)/* libffi-src/ && \
	rm -rf libffi-$(FFI_VERSION)

	mkdir -p xml2/libxml2-src
	if [ ! -f $(XML2_ARCHIVE) ]; then \
		wget https://download.gnome.org/sources/libxml2/2.12/libxml2-$(XML2_VERSION).tar.xz -O $(XML2_ARCHIVE); \
	fi
	cd xml2 && \
	tar xf $(XML2_ARCHIVE) && \
	cp -r libxml2-$(XML2_VERSION)/* libxml2-src/ && \
	rm -rf libxml2-$(XML2_VERSION)

	mkdir -p expat/libexpat-src
	if [ ! -f $(EXPAT_ARCHIVE) ]; then \
		wget https://github.com/libexpat/libexpat/releases/download/R_2_7_0/expat-$(EXPAT_VERSION).tar.gz -O $(EXPAT_ARCHIVE); \
	fi
	cd expat && \
	tar xf $(EXPAT_ARCHIVE) && \
	cp -r expat-$(EXPAT_VERSION)/* libexpat-src/ && \
	rm -rf expat-$(EXPAT_VERSION)

$(DIRECTORIES):
	$(MAKE) -C $@ $(MAKECMDGOALS)
