#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <config/api.h>

#include <sys/types.h>
#include <sys/stat.h>

struct MESSAGE_BUFFER {
	char			message[512];
	char			who[128];
	char			channel[512];
	time_t			when;
	int			id;
	struct MESSAGE_BUFFER	*next;
};

typedef struct {
	struct MESSAGE_BUFFER	*buffer;
	struct MESSAGE_BUFFER	*re_remind;
	const char		*network;
} MAIN;


void sendHelp(const char *from) {
	return;
}


unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "makerspace.se";
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
	channel = ircGetIntendedChannel(channel, from);
	
	if (strcmp(message, API_HELP_CMD) == 0)
		sendHelp(from);
	if (strstr(message, ".lokalregler") == message) {
		sprintf(buff, "%s: <insert link here>", from);
		ircMessage(channel, buff);
	}
	

	return;
}
