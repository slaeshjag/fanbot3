#include "legacy.h"


unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "legacy";
}


char *pluginURLEscapeUgly(const char *string) {
	char *new;
	int i;
	const unsigned char *string_u;

	string_u = (const unsigned char *) string;

	if ((new = malloc(strlen(string) * 3 + 1)) == NULL)
		return NULL;
	
	for (i = 0; string_u[i] != 0; i++) {
		if (string_u[i] == ' ')
			sprintf(&new[i*3], "%%%X", ';');
		else
			sprintf(&new[i*3], "%%%X", string_u[i]);
	}
	return new;
}


void *pluginDestroy(void *handle) {
	LEGACY *legacy = handle;

	free(legacy->command);
	free(legacy->file);
	free(legacy->buffer);
	free(legacy);

	return NULL;
}

void *pluginDoInit(const char *network) {
	LEGACY *legacy;
	int i;
	char *tmp;

	if ((legacy = malloc(sizeof(LEGACY))) == NULL)
		return NULL;
	
	if (getPageFromURL(COMMANDLIST, NULL, NULL, &legacy->buffer) != NET_NO_ERROR) {
		free(legacy);
		return NULL;
	}

	legacy->commands = 0;
	tmp = legacy->buffer;
	for (legacy->commands = 0; tmp != NULL; legacy->commands++) {
		if ((tmp = strchr(tmp, '\n')) != NULL)
			tmp++;
	}

	legacy->command = malloc(sizeof(char *) * legacy->commands);
	legacy->file = malloc(sizeof(char *) * legacy->commands);

	if (!legacy->command || !legacy->file)
		return pluginDestroy(legacy);

	/* There. Now all I need to do is mangle the string and make it ugly */
	
	tmp = legacy->buffer;
	for (i = 0; i < legacy->commands; i++) {
		legacy->command[i] = tmp;
		if ((tmp = strchr(tmp, '\n')) != NULL) {
			*tmp = 0;
			tmp++;
		}
	}

	for (i = 0; i < legacy->commands; i++) {
		tmp = strchr(legacy->command[i], ';');
		if (tmp != NULL) {
			*tmp = 0;
			tmp++;
		}
		legacy->file[i] = tmp;
	}

	return legacy;
}


void pluginProcessHit(const char *channel, const char *message, char *urlfile) {
	char *buffer, *tmp, *end;

	if (getPageFromURL(urlfile, NULL, NULL, &buffer) != NET_NO_ERROR)
		return;
	
	tmp = buffer;
	while (tmp != NULL) {
		if ((end = strchr(tmp, '\n')) != NULL) {
			*end = 0;
			end++;
		}
	
		if (*tmp != 0)
			ircMessage(channel, tmp);
		tmp = end;
	}

	free(buffer);
	return;
}


void pluginTimerPoke(void *handle, int id) {
	return;
}


void pluginFilter(void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	LEGACY *legacy = handle;
	char buff[2048], *data;
	int i;

	if (legacy == NULL)
		return;
	if (strcmp(command, "PRIVMSG") != 0)
		return;
	channel = ircGetIntendedChannel(channel, from);
	for (i = 0; i < legacy->commands; i++) {
		sscanf(message, "%s", buff);
		if (strcmp(buff, legacy->command[i]) == 0) {
			data = pluginURLEscapeUgly(message);
			sprintf(buff, "%s%s?args=%s", HOST_PREFIX, legacy->file[i], data);
			free(data);
			pluginProcessHit(channel, message, buff);
			return;
		}
	}


	if (strcmp(message, "!update!") == 0) {
		sprintf(buff, "%s: Requesting complete reload of filter plugins", from);
		ircMessage(channel, buff);
		configFilterReload();
	}

	return;
}
