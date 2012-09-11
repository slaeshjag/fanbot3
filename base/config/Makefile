SRC = config.c plugin.c network.c layer.c irc.c filter.c timer.c
CFLAGS += -fPIC -shared -Wl,-soname,config.so -O2 -Wall
LDFLAGS += -ldl

default:
	$(CC) $(CFLAGS) $(SRC) -o ../../bin/base/config.so $(LDFLAGS)

clean:
	

