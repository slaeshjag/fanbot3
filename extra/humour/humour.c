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
	struct list_main *m = handle;

	free(m->list);
	free(m);

	return NULL;
}


void *pluginDoInit(const char *network) {
	struct list_main *m;
	int i, j, args;
	char *t, buff[600];
	const char *nick = ircNickname();
	FILE *fp = fopen("conf/humour.list", "r");

	m = malloc(sizeof(*m));
	if (!fp) {
		m->list = NULL;
		m->entries = 0;
		return m;
	}
	
	for (i = 0; !feof(fp); fgets(buff, 512, fp), i++);
	i--;
	m->list = malloc(sizeof(*m->list) * i);
	m->entries = i;
	fseek(fp, 0, SEEK_SET);

	for (i = 0; i < m->entries; i++) {
		fscanf(fp, "%s ", buff);
		if ((t = strchr(buff, '@'))) {
			*t = 0;
			t++;
			sprintf(m->list[i].command, "%s%s%s", buff, nick, t);
		} else
			strcpy(m->list[i].command, buff);
		fgets(m->list[i].reply, 320, fp);

		for (j = args = 0; m->list[i].reply[j]; j++) {
			if (m->list[i].reply[j] == '%') {
				if (m->list[i].reply[j + 1] == '%')
					j++;
				else
					args++;
			}
		}

		if (args > 3) {
			fprintf(stderr, "BAD MESSAGE IN LIST AT LINE %i: Too many arguments\n", i);
			return NULL;
		}

		m->list[i].args = args;
		m->list[i].reply[strlen(m->list[i].reply) - 1] = 0;
		/* Allow for spaces in the command */
		for (t = m->list[i].command; t && strchr(t, '~'); t = strchr(t, '~'))
			*strchr(t, '~') = ' ';
		fprintf(stderr, "Added command '%s'\n", m->list[i].command);
	}

	return m;
}


void pluginTimerPoke(void *handle, int id) {
	return;
}


void pluginFilter(void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	char buff[520], arg1[512], arg2[512], arg3[512];
	int i, j;
	struct list_main *m = handle;

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	channel = ircGetIntendedChannel(channel, from);
	for (i = 0; i < m->entries; i++) {
		j = strlen(m->list[i].command);
		/* TODO: Handle arguments */
		if (strstr(message, m->list[i].command) == message) {
			if (message[j] != 0 && message[j] != ' ')
				continue;
			message += strlen(m->list[i].command);
			if (m->list[i].args > 0) {
				*arg1 = *arg2 = *arg3 = 0;
				sscanf(message, " %s %s %s", arg1, arg2, arg3);
			}
		
			if (m->list[i].args == 0) {
				ircMessage(channel, m->list[i].reply);
				continue;
			} else if (m->list[i].args == 1)
				sprintf(buff, m->list[i].reply, arg1);
			else if (m->list[i].args == 2)
				sprintf(buff, m->list[i].reply, arg1, arg2);
			else if (m->list[i].args == 3)
				sprintf(buff, m->list[i].reply, arg1, arg2, arg3);
			else continue;
			ircMessage(channel, buff);
		}
	}

	return;
}
