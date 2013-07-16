SRC = utf8validate.c utf8.c
CFLAGS += -O3 -fPIC -shared -Wl,-soname,utf8valid.so -o ../../bin/extra/utf8valid.so -I../../base -Wall

default:
	$(CC) $(CFLAGS) $(SRC) $(LDFLAGS)

clean:
	

