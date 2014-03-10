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
	int i, args;
	char *t;
	FILE *fp = fopen("conf/humour.list", "r");

	m = malloc(sizeof(*m));
	
	for (i = 0; !feof(fp); fgets(buff, 512, fp), i++);
	i--;

	m->list = malloc(sizeof(*m->list) * i);
	m->entries = i;
	fseek(fp, 0, SEEK_SET);

	for (i = 0; !feof(fp); i++) {
		fscanf(fp, "%s %i", m->list[i].command, &m->list[i].args);
		fgets(fp, 320, m->list[i].reply);
		m->list[i].reply[strlen(m->list[i].reply) - 1] = 0;
		/* Allow for spaces in the command */
		for (t = m->list[i].command; strchr(t, '%'); t = strchr(t, '%'))
			*strchr(t, '%') = ' ';
		/* TODO: Expand something to the bots ircnick */
	}

	return m;
}


void pluginTimerPoke(void *handle, int id) {
	return;
}


void pluginFilter(void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	char buff[520];
	int i;
	struct list_main *m = handle;

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	channel = ircGetIntendedChannel(channel, from);
	for (i = 0; i < m->entries; i++) {
		/* TODO: Handle arguments */
		if (strstr(message, m->list[i].command) == message)
			ircMessage(channel, m->list[i].reply);
	}

	return;
}
