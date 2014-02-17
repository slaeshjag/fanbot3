#include <stdlib.h>
#include <stdio.h>
#include <pcre.h>

#include <config/api.h>
#include "net.h"

struct regexes {
	pcre		*match1;
	pcre		*match2;
};


unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "Google-calculator";
}


void *pluginDestroy(void *handle) {
	struct regexes *r = handle;


	free(r);
	return NULL;
}


void *pluginDoInit(const char *network) {
	struct regexes *r;

	r = malloc(sizeof(*r));
	r->match1 = pcre_compile("/\\<\\s*span[^\\>]*id\\s*\\=\\s*\"\\s*cwos\\s*\"[^\\>]*\\>\\s*([^\\<]*)\\s*\\<\\s*\\/\\s*span\\s*\\>/", 0, NULL, NULL, NULL);
	r->match2 = pcre_compile("/\\<\\s*div\\b[^\\>]+\\bclass\\s*\\=\\s*\"[^\"]*\\b(vk_ans|vk_bk)\\b[^\"]*\\b(vk_ans|vk_bk)\\b[^\"]*\"[^\\>]*\\>\\s*([^\\<]*)\\b\\s*\\<\\s*\\/\\s*div\\s*\\>/", 0, NULL, NULL, NULL);

	return r;
}


void sendHelp(const char *from) {
	ircMessage(from, ".c <query> - Send <query> to google calculator");

	return;
}


void pluginTimerPoke(void *handle, int id) {
	return;
}


void plugin_escape_string(char *out, const char *in) {
	int i;

	*out = 0;
	for (i = 0; in[i]; i++)
		sprintf(&out[i * 3], "%%%.2X", (unsigned char) in[i]);
	return;
}


void pluginFilter(void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	char *buffer, *next, buff[2048], *print, buff_request[2048];

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	if (strcmp(message, API_HELP_CMD) == 0)
		sendHelp(from);
	if (strstr(message, ".c ") != message) {
		if (strstr(message, "calc ") != message)
			return;
		else
			message += 5;
	} else
		message += 3;
	
	while (*message == ' ')
		message++;
	channel = ircGetIntendedChannel(channel, from);

	plugin_escape_string(buff_request, message);
	sprintf(buff, "https://www.google.se/search?q=%s", buff_request);
	fprintf(stderr, "URL: %s\n", buff);
	if (getPageFromURL(buff, NULL, NULL, &buffer) != NET_NO_ERROR) {
		sprintf(buff, "%s: Internal error.", from);
		ircMessage(channel, buff);
		return;
	}

	fprintf(stderr, "%s\n", buffer);

	return;
}
