#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <config/api.h>

#include <sys/types.h>
#include <sys/stat.h>

struct MESSAGE_BUFFER {
	char			message[512];
	char			who[128];
	char			channel[512];
	time_t			when;
	int			id;
	struct MESSAGE_BUFFER	*next;
};

typedef struct {
	struct MESSAGE_BUFFER	*buffer;
	struct MESSAGE_BUFFER	*re_remind;
	const char		*network;
} MAIN;


void sendHelp(const char *from) {
	ircMessage(from, "<pling [!nick] <+hh:mm[:ss]> [message] - Remind someone about [message] in <+hh:mm[:ss]> (time relative from now)");
	ircMessage(from, "<getpling - List all reminders that you'll get in the future");
	ircMessage(from, "<later <+hh:mm:ss> - Remind about last reminder it sent in <+hh:mm> (time relative from now)");
	ircMessage(from, "<rmpling <id> - Delete a reminder");
	ircMessage(from, "<movepling <id> <+hh:mm[:ss]> - Move the pling to a time relative to now");

	return;
}


static int calculateTimeOffset(const char *str, time_t *when, int *h, int *m, int *s) {
	int offset, read, i, j;
	int timedelta;
	int number[3];
	char delim[3];
	*h = *m = *s = 0;

	if (*str == '+') {
		sscanf(str, "+%i:%i:%i", h, m, s);
	} else if (*str == '@') {
		delim[0] = delim[1] = delim[2] = '\0';

		read = sscanf(str, "@%d%c%d%c%d%c", number, delim, number+1, delim+1, number+2, delim+2);

		for (i = 0; i < read/2; i++) {
			/* h, m, s are the only valid delimiters */
			if (delim[i] != 'h' && delim[i] != 'm' && delim[i] != 's')
				return -1;

			/* duplicates are not allowed (i.e. you can't say 30s30s30s) */
			for (j = i+1; j < read/2; j++)
				if (delim[j] == delim[i])
					return -1;

			switch (delim[i]) {
				case 'h': *h = number[i]; break;
				case 'm': *m = number[i]; break;
				case 's': *s = number[i]; break;
			}
		}
	} else {
		return -1;
	}

	timedelta = *h * 3600 + *m * 60 + *s;
	if (timedelta < 60)
		return -1;

	*when = time(NULL) + timedelta;

	for (offset = 0; str[offset] != ' ' && str[offset] != 0; offset++);
	return offset;
}


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

	buffer = m->re_remind;
	m->re_remind = NULL;
	while (buffer != NULL) {
		timerDelete(buffer->id);
		tmp = buffer->next;
		free(buffer);
		buffer = tmp;
	}
		

	return;
}


struct MESSAGE_BUFFER *messageBufferMake(MAIN *m, const char *message, const char *who, const char *channel, time_t when) {
	struct MESSAGE_BUFFER *buffer;

	if ((buffer = malloc(sizeof(struct MESSAGE_BUFFER))) == NULL)
		return NULL;
	
	if ((buffer->id = timerAdd(when, "pling")) == -1) {
		free(buffer);
		return NULL;
	}

	strncpy(buffer->message, message, 512);
	buffer->message[511] = 0;
	strncpy(buffer->who, who, 128);
	strncpy(buffer->channel, channel, 512);
	buffer->when = when;

	return buffer;
}

void messageBufferAdd(MAIN *m, const char *message, const char *who, const char *channel, time_t when) {
	struct MESSAGE_BUFFER *buffer;

	if ((buffer = messageBufferMake(m, message, who, channel, when)) == NULL)
		return;
	buffer->next = m->buffer;
	m->buffer = buffer;

	return;
}


void messageBufferAddNick(MAIN *m, struct MESSAGE_BUFFER *buffer) {
	buffer->next = m->re_remind;
	m->re_remind = buffer;

	return;
}


void messageBufferDump(MAIN *m) {
	struct MESSAGE_BUFFER *buffer;
	char buff[256];
	FILE *fp;
	
	mkdir("data/pling", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	sprintf(buff, "data/pling/%s", m->network);
	unlink(buff);

	if ((fp = fopen(buff, "w")) == NULL) {
		configErrorPush("pling: Unable to dump the messsage buffer to file: Unable to create file");
		return;
	}

	buffer = m->buffer;
	while (buffer != NULL) {
		fprintf(fp, "%lli %s %s %s\n", (long long int) buffer->when, buffer->channel, buffer->who, buffer->message);
		buffer = buffer->next;
	}

	fclose(fp);
	return;
}


void messageBufferRead(MAIN *m) {
	FILE *fp;
	long long int when;
	time_t now;
	char buff[256], channel[512], message[514], who[128];

	sprintf(buff, "data/pling/%s", m->network);
	now = time(NULL);

	if ((fp = fopen(buff, "r")) == NULL) {
		configErrorPush("pling: Warning: Unable to open message buffer dump");
		return;
	}

	while (!feof(fp)) {
		when = 0;
		fscanf(fp, "%lli %s %s", (long long int *) &when, channel, who);
		if (when == 0)
			break;
		fgets(message, 514, fp);
		if (strchr(message, '\n'))
			*(strchr(message, '\n')) = 0;
		if (now >= when)
			when = now + 2;
		messageBufferAdd(m, &message[1], who, channel, when);
	}

	fclose(fp);

	return;
}


void messageBufferDeleteNick(MAIN *m, const char *who) {
	struct MESSAGE_BUFFER *buffer, *old;

	if (m->re_remind == NULL)
		return;

	old = m->re_remind;
	buffer = old->next;
	if (strcmp(old->who, who) == 0) {
		m->re_remind = old->next;
		free(old);
		return;
	}
	while (buffer != NULL) {
		if (strcmp(buffer->who, who) == 0) {
			old->next = buffer->next;
			free(buffer);
			return;
		}
		old = buffer;
		buffer = buffer->next;
	}

	return;
}


void messageBufferDelete(MAIN *m, int id, const char *who) {
	struct MESSAGE_BUFFER *buffer, *old;
	char who1[128], who2[128];

	if (m->buffer == NULL)
		return;

	if (who)
		sprintf(who1, "%s", who);
	else
		*who1 = 0;
	stringToUpper(who1);

	old = m->buffer;
	buffer = old->next;
	sprintf(who2, "%s", old->who);
	stringToUpper(who2);

	if (old->id == id && (!strcmp(who1, who2) || !who)) {
		m->buffer = old->next;
		timerDelete(old->id);
		messageBufferDeleteNick(m, old->who);
		messageBufferAddNick(m, old);
		return;
	}
	while (buffer != NULL) {
		sprintf(who2, "%s", old->who);
		stringToUpper(who2);
	
		if (buffer->id == id && (!strcmp(who1, who2) || !who)) {
			old->next = buffer->next;
			timerDelete(buffer->id);
			messageBufferDeleteNick(m, buffer->who);
			messageBufferAddNick(m, buffer);
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
		buffer = buffer->next;
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
	m->re_remind = NULL;
	m->network = network;

	messageBufferRead(m);

	return m;
}


void pluginTimerPoke(void *handle, int id) {
	MAIN *m = handle;
	char buff[1024];
	struct MESSAGE_BUFFER *buffer;
	
	if ((buffer = messageGetBuffer(m, id)) == NULL)
		return;
	sprintf(buff, "%s: %s\n", buffer->who, buffer->message);
	ircMessage(buffer->channel, buff);
	messageBufferDelete(m, id, NULL);

	return;
}


struct MESSAGE_BUFFER *pluginFindOld(MAIN *m, const char *who) {
	struct MESSAGE_BUFFER *buffer;

	buffer = m->re_remind;
	while (buffer != NULL) {
		if (strcmp(buffer->who, who) == 0)
			return buffer;
		buffer = buffer->next;
	}

	return NULL;
}


int pluginRepling(MAIN *m, const char *message, const char *from, const char *channel) {
	struct MESSAGE_BUFFER *buffer_s;
	char buffer[520];
	int hours, minutes, seconds;
	time_t then;

	message += 7;
	if ((calculateTimeOffset(message, &then, &hours, &minutes, &seconds)) < 0) {
		sprintf(buffer, "%s: Usage: <later +hh:mm", from);
		ircMessage(channel, buffer);
		return -1;
	}
		
	if ((buffer_s = pluginFindOld(m, from)) == NULL) {
		sprintf(buffer, "%s: You have not been reminded about anything yet.", from);
		ircMessage(channel, buffer);
		return -1;
	}

	messageBufferAdd(m, buffer_s->message, from, channel, then);
	messageBufferDeleteNick(m, from);

	sprintf(buffer, "%s: Mkay, I'll remind you again in %i hours, %i minutes and %i seconds", from, hours, minutes, seconds);
	ircMessage(channel, buffer);

	return 0;
}


int pluginListPling(MAIN *m, const char *from) {
	struct MESSAGE_BUFFER *buffer;
	char message[512], who1[128], who2[128];
	int hours, minutes, days;
	time_t now, diff;
	now = time(NULL);
	
	strncpy(who1, from, 128);
	stringToUpper(who1);
	sprintf(message, "List of reminders:");
	ircMessage(from, message);

	buffer = m->buffer;
	while (buffer != NULL) {
		strncpy(who2, buffer->who, 128);
		stringToUpper(who2);
		if (strcmp(who2, who1) == 0) {
			diff = buffer->when - now;
			minutes = (diff % 3600) / 60;
			hours = (diff % 86400) / 3600;
			days = diff / 86400;
			sprintf(message, "+%id, %i:%i %s | [%i]", days, hours, minutes, buffer->message, buffer->id);
			ircMessage(from, message);
		}
		buffer = buffer->next;
	}

	return 0;
}


void pluginDeletePling(void *handle, const char *from, const char *message) {
	int id;
	char buff[510];

	message += strlen("<rmpling");
	id = atoi(message);
	messageBufferDelete(handle, id, from);

	return;
}


int pluginMovePling(void *handle, const char *from, const char *message) {
	MAIN *m = handle;
	struct MESSAGE_BUFFER *buffer;
	int hours, minutes, seconds, id, n;
	char who1[128], who2[128], buff[512];
	time_t when;

	hours = minutes = seconds = 0;
	id = -1;
	message += 11;
	id = -1;
	for (; *message == ' '; message++);
	sscanf(message, "%i", &id);
	for (; *message != ' ' && *message != 0; message++);
	for (; *message == ' '; message++);

	if ((n = calculateTimeOffset(message, &when, &hours, &minutes, &seconds)) < 0)
		goto bad_dateformat;
	
	for (buffer = m->buffer; buffer; buffer = buffer->next)
		if (buffer->id == id)
			break;
	if (!buffer)
		goto bad_event;
	strcpy(who1, from);
	strcpy(who2, buffer->who);
	stringToUpper(who1);
	stringToUpper(who2);
	if (strcmp(who1, who2))
		goto bad_event;
	timerDelete(buffer->id);
	buffer->when = when;

	sprintf(buff, "Moved pling %i to %i h, %i m, %i s from now", buffer->id, hours, minutes, seconds);
	ircMessage(from, buff);
	buffer->id = timerAdd(buffer->when, "pling");
	return 0;

	bad_dateformat:
	
	sprintf(buff, "%s: Dateformat: [! nickname] +hh:mm [message] where time is relative to now", from);
	ircMessage(from, buff);
	return -1;
	
	bad_event:
	sprintf(buff, "Event %i doesn't exist", id);
	ircMessage(from, buff);
	return -1;
}

	

void pluginFilter(void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	char buff[520], to[520];
	int minutes, hours, start_from, seconds, n;
	time_t then;
	if (handle == NULL)
		return;

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	channel = ircGetIntendedChannel(channel, from);
	
	if (strcmp(message, API_HELP_CMD) == 0)
		sendHelp(from);
	if (strstr(message, "<later ") == message)
		pluginRepling(handle, message, from, channel);
	if (strstr(message, "<getpling") == message)
		pluginListPling(handle, from);
	if (strstr(message, "<rmpling") == message)
		pluginDeletePling(handle, from, message);
	if (strstr(message, "<movepling") == message)
		pluginMovePling(handle, from, message);
	if (strstr(message, "<pling ") != message)
		return;
	
	message += strlen("<pling ");
	hours = minutes = start_from = seconds = 0;
	*to = 0;
	sscanf(message, "! %s", to);
	if (*to != 0) {
		start_from = 1;
		message += 2;
		if ((message = strstr(message, " ")) == NULL) {
			sprintf(buff, "%s: Dateformat: [! nickname] +hh:mm [message] where time is relative to now", from);
			ircMessage(channel, buff);
			return;
		}
		message++;
	} else
		sprintf(to, "%s", from);
	
	if ((n = calculateTimeOffset(message, &then, &hours, &minutes, &seconds)) < 0) {
		sprintf(buff, "%s: Dateformat: [! nickname] +hh:mm [message] where time is relative to now", from);
		ircMessage(channel, buff);
		return;
	}

	message += n;

	if ((message = strstr(message, " ")) == NULL)
		message = "Pling!";
	else
		message++;

	if (start_from)
		sprintf(buff, "<%s> reminds you: %s", from, message);
	else
		sprintf(buff, "%s", message);

	if (*channel != '#')
		messageBufferAdd(handle, buff, to, to, then);
	else
		messageBufferAdd(handle, buff, to, channel, then);

	if (start_from)
		sprintf(buff, "%s: Mkay, I'll remind %s in %i hours, %i minutes and %i seconds", from, to, hours, minutes, seconds);
	else
		sprintf(buff, "%s: Mkay, I'll remind you in %i hours and %i minutes and %i seconds", from, hours, minutes, seconds);
	ircMessage(channel, buff);

	return;
}
