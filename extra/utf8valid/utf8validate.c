#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <config/api.h>

#include "utf8.h"


unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "utf8-validate";
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


int pluginUtf8Validate(const char *str) {
	int i;

	for (i = 0; str[i]; i += utf8GetValidatedCharLength(&str[i]))
		if (utf8GetChar(&str[i]) == UTF8_REPLACEMENT_CHAR)
			return 0;
	return 1;
}


void pluginFilter(void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	char buff[520];

	if (strcmp(command, "PRIVMSG") != 0)
		return;

	if (strstr(message, "<utf8") != message)
		return;
	
	channel = ircGetIntendedChannel(channel, from);
	if (!pluginUtf8Validate(message)) {
		sprintf(buff, "%s: Message is NOT valid UTF-8", from);
		ircMessage(channel, buff);
	} else {
		sprintf(buff, "%s: Message is valid UTF-8", from);
		ircMessage(channel, buff);
	}

	return;
}
