#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <config/api.h>


unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "connection-keepalive";
}


void *pluginDestroy(void *handle) {
	return NULL;
}


void *pluginDoInit(const char *network) {
	timerAdd(1500, "connection-keepalive"); /* Half an hour */

	return NULL;
}


void pluginTimerPoke(void *handle, int id) {
	char str[512];

	timerAdd(1500, "connection-keepalive"); /* Half an hour */

	sprintf(str, "ISON %s", networkNick());
	ircRaw(str);

	return;
}


void pluginFilter(void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	char buff[520];

	if (strcmp(command, "PING") != 0)
		return;

	fprintf(stderr, "Got ping\n");

	return;
}
