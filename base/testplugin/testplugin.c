#include "testplugin.h"

unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "Testplugin";
}


void *pluginDoInit(const char *network) {
	return NULL;		/* We don't need data of our own */
}


void pluginFilter(void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	char sendbuff[512];
	int *nnn;

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	if (strcmp(channel, networkNick()) == 0)
		channel = from;
	if (strcmp(message, "segfault") == 0) {
		nnn = NULL;
		*nnn = 0xDEADBEEF;
	}

	sprintf(sendbuff, "%s: %s - %s", from, message, host);
	ircMessage(channel, sendbuff);

	return;
}


void *pluginDestroy(void *handle) {
	return NULL;		/* We don't have any data to get rid of */
}
