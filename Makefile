build:
	mkdir -p out
	touch out/dummy.fex
	make --quiet -C Audio build
	make --quiet -C FileSystem build
	make --quiet -C Generic build
	make --quiet -C Network build
	make --quiet -C Video build

prepare:
	$(info Nothing to prepare)

clean:
	rm -rf out
	make -C Audio clean
	make -C FileSystem clean
	make -C Generic clean
	make -C Network clean
	make -C Video clean
