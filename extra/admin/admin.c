#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <config/api.h>

#include <sys/types.h>
#include <sys/stat.h>

#define	min(x, y)		((x) < (y) ? (x) : (y))


void sendHelp(const char *from) {
	return;
}


unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "admin";
}


void *pluginDestroy(void *handle) {
	return NULL;
}


void *pluginDoInit(const char *network) {
	return NULL;
}


void pluginTimerPoke(void *handle, int id) {

	return;
}


void pluginFilter(void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	char buff[520];
	int i, j;
	double k;
	time_t t;

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	channel = ircGetIntendedChannel(channel, from);
	
	if (strcmp(message, API_HELP_CMD) == 0)
		sendHelp(from);
	

	return;
}
