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

	pcre_free(r->match1);
	pcre_free(r->match2);
	free(r);
	return NULL;
}


void *pluginDoInit(const char *network) {
	struct regexes *r;
	const char *err;
	int errno;

	r = malloc(sizeof(*r));
	r->match1 = pcre_compile("\\<\\s*span[^\\>]*id\\s*\\=\\s*\"\\s*cwos\\s*\"[^\\>]*\\>\\s*([^\\<]*)\\s*\\<\\s*\\/\\s*span\\s*\\>", PCRE_DOTALL, &err, &errno, NULL);
	r->match2 = pcre_compile("\\<\\s*div\\b[^\\>]+\\bclass\\s*\\=\\s*\"[^\"]*\\b(vk_ans|vk_bk)\\b[^\"]*\\b(vk_ans|vk_bk)\\b[^\"]*\"[^\\>]*\\>\\s*([^\\<]*)\\b\\s*\\<\\s*\\/\\s*div\\s*\\>", PCRE_UCP, &err, &errno, NULL);

	if (!r->match1 || !r->match2)
		fprintf(stderr, "Regex compile failed\n");

	SSL_library_init();
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	OpenSSL_add_all_algorithms();

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
	struct regexes *r = handle;
	char *buffer, buff[2048], buff_request[2048];
	int ovect[63], rc, bufflen;

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

	/* Extract reply */
	bufflen = strlen(buffer);
	if ((rc = pcre_exec(r->match1, NULL, buffer, bufflen, 0, 0, ovect, 63)) >= 1) {
		memcpy(buff, &buffer[ovect[2]], ovect[3] - ovect[2]);
		buff[ovect[3] - ovect[2]] = 0;
	} 
	else if ((rc = pcre_exec(r->match2, NULL, buffer, bufflen, 0, 0, ovect, 63)) >= 3) {
		memcpy(buff, &buffer[ovect[6]], ovect[7] - ovect[6]);
		buff[ovect[7] - ovect[6]] = 0;
	} else {
		sprintf(buff, "Invalid expression %i\n", rc);
	}

	ircMessage(channel, buff);

	free(buffer);

	return;
}
