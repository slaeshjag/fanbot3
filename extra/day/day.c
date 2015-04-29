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
	ircMessage(from, "<day - Get current day and week");
	return;
}


unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "day";
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
	char buff[520], ti[520];
	time_t now;
	struct tm t;

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	channel = ircGetIntendedChannel(channel, from);
	
	if (strcmp(message, API_HELP_CMD) == 0)
		sendHelp(from);
	if (!strcmp(message, "<day")) {
		now = time(NULL);
		localtime_r(&now, &t);
		strftime(ti, 520, "%A %F, week %V", &t);
		sprintf(buff, "%s: %s", from, ti);
		ircMessage(channel, buff);
	}
	
	return;
}
