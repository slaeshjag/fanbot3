default:
	mkdir -p bin
	mkdir -p bin/base
	mkdir -p bin/extra
	mkdir -p bin/conf
	mkdir -p bin/data
	cp skel/* bin/conf
	cd base && make
	cd extra && make

clean:
	rm -Rf bin
	cd base && make clean

