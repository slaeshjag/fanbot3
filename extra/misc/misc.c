#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <config/api.h>

#include <sys/types.h>
#include <sys/stat.h>

void sendHelp(const char *from) {
	ircMessage(from, "<strlen <string> - Return the string length of <string>");
	ircMessage(from, "<vecka - Get current ISO8601 week number");
	ircMessage(from, "<unix - Current UNIX-time according to local timezone");
	ircMessage(from, "<roll <sides> - Roll a dice");
	return;
}


unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "misc";
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
	int i;
	time_t t;
	struct tm *time_s;

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	channel = ircGetIntendedChannel(channel, from);
	
	if (strcmp(message, API_HELP_CMD) == 0)
		sendHelp(from);
	if (strstr(message, "<strlen ") == message) {
		i = strlen(message + 8);
		sprintf(buff, "%s: %i", from, i);
		ircMessage(channel, buff);
	} else if (!strcmp(message, "<vecka")) {
		t = time(NULL);
		time_s = localtime(&t);
		strftime(buff, 520, "Vecka: %V", time_s);
		ircMessage(channel, buff);
	} else if (!strcmp(message, "<unix")) {
		t = time(NULL);
		sprintf(buff, "%s: Current Unix time is: %lli", from, (long long int) t);
		ircMessage(channel, buff);
	} else if (strstr(message, "<roll ") == message) {
		unsigned int s;
		s = 0;
		sscanf(message, "<roll %u", &s);
		if (s < 1) {
			sprintf(buff, "%s: Number of sides must be 1 or more", from);
			ircMessage(channel, buff);
		} else {
			sprintf(buff, "%s: %i\n", from, rand() % s + 1);
			ircMessage(channel, buff);
		}
	} else if (strstr(message, "arne")) {
		/*if (rand() % 5)
			ircMessage(channel, "arne");*/
	} else if (!strcasecmp(from, "wally") && !strcasecmp(message, "pls")) {
		ircMessage(channel, "PLS U");
	}
	

	return;
}
