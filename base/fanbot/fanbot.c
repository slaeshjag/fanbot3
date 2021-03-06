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


void die(int signal_num) {
	(fanbot.destroy)(fanbot.handle, "SIGINT received (admin pressed CTRL-C) - Bot is shutting down");
	dlclose(fanbot.library);
	exit(0);
}


void segfault(int signal_num) {
	(fanbot.destroy)(fanbot.handle, "SIGSEGV received (Segmentation fault) - Bot is shutting down");
	dlclose(fanbot.library);
	exit(-1);
}


void busserr(int signal_num) {
	(fanbot.destroy)(fanbot.handle, "SIGBUS received (Buss error) - Bot is shutting down");
	dlclose(fanbot.library);
	exit(-1);
}


int main(int argc, char **argv) {
	signal(SIGUSR1, reload);
	signal(SIGINT, die);
	signal(SIGSEGV, segfault);
	#ifdef SIGBUS
	signal(SIGBUS, busserr);
	#endif
	signal(SIGPIPE, SIG_IGN);

	for (;;) {
		if ((fanbot.library = dlopen("base/config.so", RTLD_NOW | RTLD_GLOBAL)) == NULL) {
			fprintf(stderr, "Unable to load base/config.so. Fanbot is nothing without config.so\n%s\n", dlerror());
			return -1;
		}
		loadSymbols();
		fanbot.handle = (fanbot.init)();
		(fanbot.destroy)(fanbot.handle, "SIGUSR1 received - Bot is going down for complete reload");
		dlclose(fanbot.library);
	}
	fprintf(stderr, "THE WORLD IS GOING UNDER! INFINITE LOOPS AREN'T INFINITE ANYMORE! SAVE YOURSELF! ETC!!\n");

	return -1;
}
