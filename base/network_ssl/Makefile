SRC = network.c
OUTPUT = ../../bin/base/network_ssl.so
CFLAGS += -fPIC -shared -Wl,-soname,network_ssl.so -O3 -Wall -I../
LDFLAGS += -lssl


default:
	$(CC) $(SRC) -o $(OUTPUT) $(CFLAGS) $(LDFLAGS)

clean:
	
