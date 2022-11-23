build:
	mkdir -p out
	make --quiet -C Input build
	make --quiet -C Disk build
	make --quiet -C Audio build
	make --quiet -C FileSystem build
	make --quiet -C Generic build
	make --quiet -C Network build
	make --quiet -C Video build

prepare:
	$(info Nothing to prepare)

clean:
	rm -rf out
	make -C Input clean
	make -C Disk clean
	make -C Audio clean
	make -C FileSystem clean
	make -C Generic clean
	make -C Network clean
	make -C Video clean
