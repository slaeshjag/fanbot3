#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <config/api.h>

#include "twitter.h"

unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "twitter";
}


void *pluginDestroy(void *handle) {
	return NULL;
}


void *pluginDoInit(const char *network) {
	return NULL;
}


char *findTweetStart(char *foo) {
	int i, len;

	len = strlen(foo);
	for (i = 0; i < len; i++)
		if (foo[i] != ' ')
			return &foo[i];
	return "Error in data from Twitter. They probably changed their site again";
}


char stripGetRealChar(char *str, int *cnt) {
	char buff[8], ret;
	unsigned int ch;
	int i;

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


void pluginFilter(void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	char *buffer, *next, buff[520], *print;

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	if (strstr(message, "<twit ") != message)
		return;
	
	message += 5;
	while (*message == ' ')
		message++;
	channel = ircGetIntendedChannel(channel, from);
	sprintf(buff, "http://twitter.com/%s", message);
	if (getPageFromURL(buff, NULL, NULL, &buffer) != NET_NO_ERROR) {
		sprintf(buff, "%s: Internal error.", from);
		ircMessage(channel, buff);
		return;
	}
	
	if (strstr(buffer, "js-tweet-text") == NULL) {
		free(buffer);
		sprintf(buff, "%s: User %s doesn't seem to exist", from, message);
		ircMessage(channel, buff);
		return;
	}

	next = strstr(buffer, "js-tweet-text");
	if (strlen(next) > strlen("js-tweet-text\">\n"))
		next += strlen("js-tweet-text\">\n");
	else {
		sprintf(buff, "%s: Error in data from Twitter. They probably changed their site again...", from);
		ircMessage(channel, buff);
		return;
	}

	next = findTweetStart(next);
	sprintf(buff, "%s: <@%s> ", from, message);
	print = buff + strlen(buff);

	while (next != NULL) {
		stripTags(next, print, 510 - strlen(buff));
		if (print[0] == '@');	/* We don't want that */
		else {
			if (*buff == 0) break;
			free(buffer);
			ircMessage(channel, buff);
			return;
		}

		next = strstr(next, "js-tweet-text");
		if (strlen(next) > strlen("js-tweet-text\">\n"))
			next += strlen("js-tweet-text\">\n");
		else break;
		next = findTweetStart(next);
	}
	
	free(buffer);
	sprintf(buff, "%s: No suitable tweet found for user %s", from, message);
	ircMessage(channel, buff);

	return;
}
