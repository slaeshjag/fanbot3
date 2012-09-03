SRC = config.c plugin.c network.c layer.c
CFLAGS += -fPIC -shared -Wl,-soname,config.so -O3
LDFLAGS += -ldl

default:
	$(CC) $(CFLAGS) $(SRC) -o ../../bin/base/config.so $(LDFLAGS)

clean:
	

