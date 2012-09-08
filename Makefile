default:
	mkdir -p bin
	mkdir -p bin/base
	mkdir -p bin/extra
	cp skel/* bin/base
	cd base && make
	cd extra && make

clean:
	rm -Rf bin
	cd base && make clean

