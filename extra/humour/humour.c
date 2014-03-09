#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <config/api.h>


struct list {
	char			command[128];
	char			reply[320];
	int			args;
};


struct list_main {
	struct list		*list;
	int			entries;
};

unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "humour";
}


void *pluginDestroy(void *handle) {
	int i;
	struct list_main *m = handle;

	free(m->list);
	free(m);

	return NULL;
}


void *pluginDoInit(const char *network) {
	struct list_main *m;

	m = malloc(sizeof(*m));
	m->list = NULL;
	m->entries = 0;

	return m;
}


void pluginTimerPoke(void *handle, int id) {
	return;
}


void pluginFilter(void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	char buff[520];

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	channel = ircGetIntendedChannel(channel, from);
	
	return;
}
