default:
	mkdir -p bin
	mkdir -p bin/base
	cd base && make

clean:
	rm -Rf bin
	cd base && make clean

