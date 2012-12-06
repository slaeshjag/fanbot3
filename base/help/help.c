#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <config/api.h>


unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "help-sent-announce";
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

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	if (strcmp(message, API_HELP_CMD) != 0)
		return;
	
	channel = ircGetIntendedChannel(channel, from);
	sprintf(buff, "%s: Sending help in query", from);
	ircMessage(channel, buff);

	return;
}
