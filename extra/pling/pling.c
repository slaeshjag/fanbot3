#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <config/api.h>


struct MESSAGE_BUFFER {
	char			message[128];
	char			who[128];
	char			channel[512];
	int			id;
	struct MESSAGE_BUFFER	*next;
};

typedef struct {
	struct MESSAGE_BUFFER	*buffer;
} MAIN;



void messageBufferDestroy(MAIN *m) {
	struct MESSAGE_BUFFER *buffer, *tmp;

	buffer = m->buffer;
	m->buffer = NULL;
	while (buffer != NULL) {
		timerDelete(buffer->id);
		tmp = buffer->next;
		free(buffer);
		buffer = tmp;
	}

	return;
}


void messageBufferAdd(MAIN *m, const char *message, const char *who, const char *channel, time_t when) {
	struct MESSAGE_BUFFER *buffer;

	if ((buffer = malloc(sizeof(struct MESSAGE_BUFFER))) == NULL)
		return;
	
	if ((buffer->id = timerAdd(when, "pling")) == -1) {
		free(buffer);
		return;
	}

	strncpy(buffer->message, message, 128);
	buffer->message[127] = 0;
	strncpy(buffer->who, who, 128);
	strncpy(buffer->channel, channel, 512);

	buffer->next = m->buffer;
	m->buffer = buffer;

	return;
}


void messageBufferDelete(MAIN *m, int id) {
	struct MESSAGE_BUFFER *buffer, *old;


	old = m->buffer;
	buffer = old->next;
	if (old->id == id) {
		m->buffer = old->next;
		timerDelete(old->id);
		free(old);
		return;
	}
	while (buffer != NULL) {
		if (buffer->id == id) {
			old->next = buffer->next;
			timerDelete(buffer->id);
			free(buffer);
			return;
		}
		old = buffer;
		buffer = buffer->next;
	}

	return;
}


struct MESSAGE_BUFFER *messageGetBuffer(MAIN *m, int id) {
	struct MESSAGE_BUFFER *buffer;

	buffer = m->buffer;
	while (buffer != NULL) {
		if (buffer->id == id)
			return buffer;
	}

	return NULL;
}


unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "pling";
}


void *pluginDestroy(void *handle) {
	messageBufferDestroy(handle);
	free(handle);

	return NULL;
}


void *pluginDoInit(const char *network) {
	MAIN *m;

	if ((m = malloc(sizeof(MAIN))) == NULL)
		return NULL;
	m->buffer = NULL;

	return m;
}


void pluginTimerPoke(void *handle, int id) {
	MAIN *m = handle;
	char buff[512];
	struct MESSAGE_BUFFER *buffer;
	
	if ((buffer = messageGetBuffer(m, id)) == NULL)
		return;
	sprintf(buff, "%s: %s\n", buffer->who, buffer->message);
	ircMessage(buffer->channel, buff);
	messageBufferDelete(m, id);

	return;
}


void pluginFilter(void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	char buff[520];
	int minutes, hours;
	time_t then;
	if (handle == NULL)
		return;

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	if (strstr(message, "<pling ") != message)
		return;
	channel = ircGetIntendedChannel(channel, from);
	
	message += strlen("<pling ");
	hours = minutes = 0;
	sscanf(message, "+%i:%i", &hours, &minutes);

	if (minutes == 0 && hours == 0) {
		sprintf(buff, "%s: Dateformat: +hh:mm where time is relative to now", from);
		ircMessage(channel, buff);
		return;
	}

	then = time(NULL);
	then += minutes * 60 + hours * 3600;
	if ((message = strstr(message, " ")) == NULL)
		message = "Pling!";
	else
		message++;

	messageBufferAdd(handle, message, from, channel, then);
	sprintf(buff, "%s: Mkay, I'll remind you in %i hours and %i minutes", from, hours, minutes);
	ircMessage(channel, buff);

	return;
}
