/* random_unicode.c - Steven Arnow <s@rdw.se>,  2013 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "utf8.h"
#include <config/api.h>


unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "Random-unicode";
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
	char buff[520], buf[5];
	FILE *fp;
	int i;
	unsigned int n;

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	
	if (!strcmp(message, "<password"))
		return;
	
	fp = fopen("/dev/urandom", "rb");
	sprintf(buff, "Dagens lösenord: ");
	for (i = 0; i < 20; i++) {
		fread(&n, 4, 1, fp);
		n = n % UTF8_CHAR_LIMIT;
		buf[utf8Encode(n, buf, 5)] = 0;
		strcat(buff, buf);
	}

	channel = ircGetIntendedChannel(channel, from);
	ircMessage(channel, buff);
	fclose(fp);

	return;
}
