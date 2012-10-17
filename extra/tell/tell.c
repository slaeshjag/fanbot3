#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <config/api.h>

#include <sys/types.h>
#include <sys/stat.h>

struct MESSAGE_BUFFER {
	int			id;
	char			message[512];
	char			who[128];
	char			channel[512];
	struct MESSAGE_BUFFER	*next;
};

typedef struct {
	struct MESSAGE_BUFFER	*buffer;
	const char		*network;
	int			cnt;
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


void messageBufferAdd(MAIN *m, const char *message, const char *who, const char *channel) {
	struct MESSAGE_BUFFER *buffer, *last;

	if ((buffer = malloc(sizeof(struct MESSAGE_BUFFER))) == NULL)
		return;
	
	last = m->buffer;
	if (last != NULL)
		while (last->next != NULL)
			last = last->next;
	strncpy(buffer->message, message, 512);
	buffer->message[511] = 0;
	strncpy(buffer->who, who, 128);
	stringToUpper(buffer->who);
	strncpy(buffer->channel, channel, 512);
	buffer->id = m->cnt;
	m->cnt++;
	
	if (last != NULL)
		last->next = buffer;
	else
		m->buffer = buffer;
	buffer->next = NULL;

	return;
}


void messageBufferDump(MAIN *m) {
	struct MESSAGE_BUFFER *buffer;
	char buff[1024];
	FILE *fp;
	
	mkdir("data/tell", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	sprintf(buff, "data/tell/%s", m->network);

	if ((fp = fopen(buff, "w")) == NULL) {
		configErrorPush("tell: Unable to dump the messsage buffer to file: Unable to create file");
		return;
	}

	buffer = m->buffer;
	while (buffer != NULL) {
		fprintf(fp, "%s %s %s\n", buffer->channel, buffer->who, buffer->message);
		buffer = buffer->next;
	}

	fclose(fp);
	return;
}


void messageBufferRead(MAIN *m) {
	FILE *fp;
	char buff[256], channel[512], message[512], who[128];

	sprintf(buff, "data/tell/%s", m->network);

	if ((fp = fopen(buff, "r")) == NULL) {
		configErrorPush("pling: Warning: Unable to open message buffer dump");
		return;
	}

	while (!feof(fp)) {
		*channel = *who = 0;
		fscanf(fp, "%s %s", channel, who);
		*message = 0;
		fgets(message, 512, fp);
		if (*channel == 0 || *who == 0 || *message == 0)
			break;
		if (strchr(message, '\n'))
			*(strchr(message, '\n')) = 0;
		messageBufferAdd(m, &message[1], who, channel);
	}

	fclose(fp);

	return;
}


void messageBufferDelete(MAIN *m, int id) {
	struct MESSAGE_BUFFER *buffer, *old;

	old = m->buffer;
	buffer = old->next;
	if (old->id == id) {
		m->buffer = old->next;
		free(old);
		return;
	}
	while (buffer != NULL) {
		if (buffer->id == id) {
			old->next = buffer->next;
			free(buffer);
			return;
		}
		old = buffer;
		buffer = buffer->next;
	}

	return;
}


void checkForMessages(MAIN *m, const char *from) {
	struct MESSAGE_BUFFER *buffer;
	char channel[256], message[512], nick[256];
	int i, c;

	sprintf(nick, "%s", from);
	stringToUpper(nick);

	*channel = 0;
	c = 0;
	buffer = m->buffer;
	while (buffer != NULL) {
		if (strcmp(buffer->who, nick) == 0) {
			ircMessage(buffer->who, buffer->message);
			if (*buffer->channel == '#')
				sprintf(channel, buffer->channel);
			i = buffer->id;
			buffer = buffer->next;
			messageBufferDelete(m, i);
			c++;
		} else
			buffer = buffer->next;
	}

	if (*channel != 0) {
		sprintf(message, "%s: %i messages sent in query", from, c);
		ircMessage(channel, message);
	}

	return;
}


unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "tell";
}


void *pluginDestroy(void *handle) {
	messageBufferDump(handle);
	messageBufferDestroy(handle);
	free(handle);

	return NULL;
}


void *pluginDoInit(const char *network) {
	MAIN *m;

	if ((m = malloc(sizeof(MAIN))) == NULL)
		return NULL;
	m->buffer = NULL;
	m->network = network;
	m->cnt = 0;

	messageBufferRead(m);

	return m;
}


void pluginTimerPoke(void *handle, int id) {
	return;
}


void pluginFilter(void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	char buff[520], to[520];
	
	if (handle == NULL)
		return;

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	if (strstr(message, "<tell ") != message) {
		checkForMessages(handle, from);
		return;
	}

	channel = ircGetIntendedChannel(channel, from);
	
	message += strlen("<tell ");
	*to = 0;

	sscanf(message, "%s ", to);
	
	if ((message = strstr(message, " ")) == NULL) {
		sprintf(buff, "%s: You need to leave a message!", from);
	}
	else {
		message++;
		sprintf(buff, "<%s> %s", from, message);
		messageBufferAdd(handle, buff, to, channel);
		sprintf(buff, "%s: Mkay, I'll pass that on to %s", from, to);
	}

	ircMessage(channel, buff);

	return;
}
