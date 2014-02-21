SRC = gcalc.c net.c
CFLAGS += -O2 -fPIC -shared -Wl,-soname,gcalc.so -o ../../bin/extra/gcalc.so -I../../base -Wall
LDFLAGS += -lssl -lpcre

default:
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS)

clean:
	

