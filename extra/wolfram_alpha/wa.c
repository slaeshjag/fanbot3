#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <config/api.h>

#include "wa.h"

unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "wolframalpha";
}


void *pluginDestroy(void *handle) {
	return NULL;
}


void *pluginDoInit(const char *network) {
	WA *wa;
	FILE *fp;

	if (!(fp = fopen("conf/wa.conf", "r")))
		return NULL;
	wa = malloc(sizeof(WA));
	fscanf(fp, "%s", wa->key);

	return wa;
}


void sendHelp(const char *from) {
	ircMessage(from, "<wa <query> - Send <query> through WolframAlpha");

	return;
}


char stripGetRealChar(char *str, int *cnt) {
	char buff[8], ret;
	unsigned int ch;

	strncpy(buff, str, 7);
	buff[7] = 0;

	if (strchr(buff, ';') != NULL)
		*(strchr(buff, ';')) = 0;

	if (strstr(buff, "&amp") == buff) {
		ret = '&';
	}

	else if (strstr(buff, "&lt") == buff) {
		ret = '<';
	}

	else if (strstr(buff, "&gt") == buff) {
		ret = '>';
	}

	else if (strstr(buff, "&nbsp") == buff) {
		ret = ' ';
	}

	else if (strstr(buff, "&apos") == buff) {
		ret = '\'';
	}

	else if (strstr(buff, "&#") == buff) {
		ch = 0;
		sscanf(&buff[2], "%i", &ch);
		ret = ch;
	} else
		ret = '?';
	
	*cnt += (strlen(buff));

	return ret;
}


void stripTags(char *src, char *dst, int max) {
	int t, i, j, len;
	
	len = strlen(src);
	for (i = t = j = 0; i < len; i++) {
		if (src[i] == 0)
			break;
		if (src[i] == '\n')
			break;
		if (src[i] == '\r')
			continue;
		if (src[i] == '<')
			t++;
		else if (src[i] == '>') {
			t--;
			continue;
		}
		if (t > 0)
			continue;
		if (src[i] == '&')
			dst[j] = stripGetRealChar(&src[i], &i);
		else
			dst[j] = src[i];
		j++;
		if (j == max)
			break;
	}

	dst[j] = 0;
	
	return;
}


void pluginTimerPoke(void *handle, int id) {
	return;
}


void escapeSilly(const char *string, char *new) {
	int i;

	for (i = 0; string[i]; i++) {
		sprintf(new, "%%%X", string[i]);
		new += 3;
	}

	*new = 0;

	return;
}
		


void pluginFilter(void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	WA *wa = handle;
	char *buffer, buff[2048], new[1584], new2[1584], *print, *orig_buff;
	int i;

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	if (strcmp(message, API_HELP_CMD) == 0)
		sendHelp(from);
	if (strstr(message, "<wa ") != message)
		return;
	
	channel = ircGetIntendedChannel(channel, from);
	if (!wa) {
		ircMessage(channel, "No WA key loaded! Put your WA API key in conf/wa.conf and then reload all plugins");
		return;
	}
	message += 4;
	while (*message == ' ')
		message++;
	escapeSilly(message, new);
	sprintf(buff, "http://api.wolframalpha.com/v2/query?appid=%s&input=%s&format=plaintext", wa->key, new);
	if (getPageFromURL(buff, NULL, NULL, &buffer) != NET_NO_ERROR) {
		orig_buff = buffer;
		sprintf(buff, "%s: WA Query failed: HTTP transmission error", from);
		free(orig_buff);
		ircMessage(channel, buff);
		return;
	}

	orig_buff = buffer;
	if (!strstr(buffer, "success='true'")) {
		sprintf(buff, "%s: WA Query failed: WA didn't have an answer", from);
		free(orig_buff);
		ircMessage(channel, buff);
		return;
	}

	if (!(print = strstr(buffer, "Result"))) {
		if (!(print = strstr(buffer, "<subpod"))) {
			sprintf(buff, "%s: WA: Query failed: Bad API response", from);
			free(orig_buff);
			ircMessage(channel, buff);
			return;
		}
			
	}

	if (!(print = strstr(print, "<plaintext>"))) {
		sprintf(buff, "%s: WA: Query failed: Bad API response", from);
		free(orig_buff);
		ircMessage(channel, buff);
		return;
	}

	print += 11;
	for (i = 0; print[i]; i++)
		if (print[i] == '\n')
			print[i] = ' ';
	
	if ((buffer = strstr(print, "</plaintext>")))
		*buffer = 0;
	stripTags(print, new2, strlen(print));
	print = new2;
	snprintf(buff, 512, "%s: WA: %s", from, new2);
	buff[511] = 0;
	free(orig_buff);
	ircMessage(channel, buff);
	
	return;
}
