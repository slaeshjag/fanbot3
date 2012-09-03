#include "fanbot.h"


void loadSymbols() {
	fanbot.init = dlsym(fanbot.library, "init");
	fanbot.destroy = dlsym(fanbot.library, "destroy");
	
	return;
}


void reload(int signal) {
	(fanbot.destroy)(fanbot.handle);
	dlclose(fanbot.library);

	if ((fanbot.library = dlopen("base/config.so", RTLD_NOW | RTLD_GLOBAL)) == NULL) {
		fprintf(stderr, "base/config.so is no longer loadable. This error is fatal.\n");
		exit(-1);
	}

	fanbot.handle = (fanbot.init)();
	
	exit(-1);
	return;
}


int main(int argc, char **argv) {
	signal(SIGUSR1, reload);
	signal(SIGSEGV, reload);

	if ((fanbot.library = dlopen("base/config.so", RTLD_NOW | RTLD_GLOBAL)) == NULL) {
		fprintf(stderr, "Unable to load base/config.so. Fanbot is nothing without config.so\n%s\n", dlerror());
		return -1;
	}

	loadSymbols();
	fanbot.handle = (fanbot.init)();

	fprintf(stderr, "init() returned. This shouldn't be possible.\n");

	return -1;
}
