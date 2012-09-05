#include "fanbot.h"


void loadSymbols() {
	fanbot.init = dlsym(fanbot.library, "init");
	fanbot.destroy = dlsym(fanbot.library, "destroy");
	fanbot.reload = dlsym(fanbot.library, "reload");
	
	return;
}


void reload(int signal_num) {
	(fanbot.reload)();

	return;
}


int main(int argc, char **argv) {
	signal(SIGUSR1, reload);

	for (;;) {
		if ((fanbot.library = dlopen("base/config.so", RTLD_NOW | RTLD_GLOBAL)) == NULL) {
			fprintf(stderr, "Unable to load base/config.so. Fanbot is nothing without config.so\n%s\n", dlerror());
			return -1;
		}
		loadSymbols();
		fanbot.handle = (fanbot.init)();
		(fanbot.destroy)(fanbot.handle, "Shutting down - init() returned");
		dlclose(fanbot.library);
	}
	fprintf(stderr, "init() returned. This shouldn't be possible.\n");

	return -1;
}
