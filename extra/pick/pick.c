#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <config/api.h>

#include <sys/types.h>
#include <sys/stat.h>

void sendHelp(const char *from) {
	ircMessage(from, "<pick arg1 or arg2 or ... - Pick one of the options");
	return;
}


unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "pick";
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


void pickNose(char *message, const char *channel, const char *from) {
	char *blah;
	int i, selection;
	char reply[520];

	for (blah = message, i = 0; blah; blah++, blah = strstr(blah, " or "), i++);
	if (i < 2 || !strlen(message)) {
		sprintf(reply, "%s: Pick what? Your nose? Usage: <pick arg1 or arg2 or ...", from);
		ircMessage(channel, reply);
		return;
	}

	selection = rand() % i;
	for (i = 0, blah = message; blah && i < selection; blah++, blah = strstr(blah, " or "), i++);
	if (i)
		blah += 4;
	if (strstr(blah, " or "))
		*strstr(blah, " or ") = 0;
	while (*blah == ' ')
		blah++;
	sprintf(reply, "%s: %s\n", from, blah);
	ircMessage(channel, reply);

	return;
}


void pluginFilter(void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	char buff[520];

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	channel = ircGetIntendedChannel(channel, from);
	
	if (strcmp(message, API_HELP_CMD) == 0)
		sendHelp(from);
	else if (strstr(message, "<pick ") == message) {
		strcpy(buff, message);
		pickNose(buff + 6, channel, from);
	}
	
	return;
}
