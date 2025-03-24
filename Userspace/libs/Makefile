MAKE_TARGETS := build clean
DIRECTORIES := $(sort $(filter-out ./include/,$(dir $(wildcard ./*/))))

.PHONY: $(MAKE_TARGETS) $(DIRECTORIES) prepare

build: copy_includes $(DIRECTORIES)
clean: $(DIRECTORIES)

copy_includes:
	cp -a $(CURDIR)/include/. $(WORKSPACE_DIR)/out/usr/include

prepare:
	$(info Nothing to prepare)

$(DIRECTORIES):
	$(MAKE) -C $@ $(MAKECMDGOALS)
