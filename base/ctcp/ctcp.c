#include "ctcp.h"
#include <stdio.h>
#include <string.h>

unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "ctcp";
}


void *pluginDoInit(const char *network) {
	return strdup(network);
}


void pluginTimerPoke(void *handle, int id) {
	return;
}


void pluginFilter(void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	char sendbuff[512], os[128];
	struct tm t;
	time_t ti;
	FILE *fp;

	if (strcmp(command, "PRIVMSG") != 0)
		return;

	channel = ircGetIntendedChannel(channel, from);
	if (strstr(message, "\x01VERSION") == message) {	/* CTCP version */
		fp = popen("uname -moo", "r");
		fgets(os, 128, fp);
		pclose(fp);
		if (strlen(os) < 1)
			return;
		if (os[strlen(os) - 1] == '\n')
			os[strlen(os) - 1] = 0;
		sprintf(sendbuff, "NOTICE %s :\x01VERSION Fanbot 3 <http://github.com/slaeshjag/fanbot3> / %s\x01\r\n", from, os);
		networkPushLine(handle, from, sendbuff);
	} else if (strstr(message, "\x01TIME") == message) {	/* CTCP time */
		ti = time(NULL);
		localtime_r(&ti, &t);
		strftime(os, 128, "%a %d %b %Y %T %Z", &t);
		sprintf(sendbuff, "NOTICE %s :\x01TIME %s\x01\r\n", from, os);
		networkPushLine(handle, from, sendbuff);
	}

	return;
}


void *pluginDestroy(void *handle) {
	free(handle);
	return NULL;		/* We don't have any data to get rid of */
}
