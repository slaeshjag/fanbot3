SRC = random_unicode.c utf8.c
CFLAGS += -O3 -fPIC -shared -Wl,-soname,random_unicode.so -o ../../bin/extra/random_unicode.so -I../../base -Wall

default:
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS)

clean:
	

