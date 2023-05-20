build:
	mkdir -p out
	mkdir -p out/modules
	make -C Input build
	make -C Storage build
	make -C Audio build
	make -C FileSystem build
	make -C Generic build
	make -C Network build
	make -C Video build

prepare:
	$(info Nothing to prepare)

clean:
	rm -rf out
	make -C Input clean
	make -C Storage clean
	make -C Audio clean
	make -C FileSystem clean
	make -C Generic clean
	make -C Network clean
	make -C Video clean
