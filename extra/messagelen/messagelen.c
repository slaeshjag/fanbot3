#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <config/api.h>


unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "Messagelenght-warning";
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
	
	sprintf(buff, ":%s!%s PRIVMSG %s :%s\r\n", from, host, channel, message);
	message = &message[strlen(message) - 64];
	message = (strstr(message, " ") != NULL) ? strstr(message, " ") + 1 : message;
	if (strlen(buff) >= 512) {
		channel = ircGetIntendedChannel(channel, from);
		sprintf(buff, "%s: That message was exactly the length that the IRC-protocol allows. It probably got cut off. Last part of the message: »%s«", from, message);
		ircMessage(channel, buff);
	}

	return;
}
