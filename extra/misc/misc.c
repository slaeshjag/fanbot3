#define	_GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <config/api.h>

#include <sys/types.h>
#include <sys/stat.h>

void sendHelp(const char *from) {
	ircMessage(from, "<strlen <string> - Return the string length of <string>");
	ircMessage(from, "<vecka - Get current ISO8601 week number");
	ircMessage(from, "<unix - Current UNIX-time according to local timezone");
	ircMessage(from, "<roll <sides> - Roll a dice");
	return;
}


unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "misc";
}


void *pluginDestroy(void *handle) {
	return NULL;
}


void *pluginDoInit(const char *network) {
	return NULL;
}


void pluginTimerPoke(void *handle, int id) {

	return;
}


void pluginFilter(void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	char buff[520], *tmp;
	int i;
	time_t t;
	struct tm *time_s;

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	channel = ircGetIntendedChannel(channel, from);
	
	if (strcmp(message, API_HELP_CMD) == 0)
		sendHelp(from);
	if (strstr(message, "<strlen ") == message) {
		i = strlen(message + 8);
		sprintf(buff, "%s: %i", from, i);
		ircMessage(channel, buff);
	} else if (!strcmp(message, "<vecka")) {
		t = time(NULL);
		time_s = localtime(&t);
		strftime(buff, 520, "Vecka: %V", time_s);
		ircMessage(channel, buff);
	} else if (!strcmp(message, "<unix")) {
		t = time(NULL);
		sprintf(buff, "%s: Current Unix time is: %lli", from, (long long int) t);
		ircMessage(channel, buff);
	} else if (strstr(message, "<roll ") == message) {
		unsigned int match, a, b, die, sides;
		match = sscanf(message, "<roll %ud%u", &a, &b);

		if (match == 1) {
			die = 1;
			sides = a;
		}
		else if (match == 2) {
			die = a;
			sides = b;
		}

		if (sides < 1 || sides > 40) {
			sprintf(buff, "%s: Number of sides must be 1-40", from);
		}
		else if (die < 1 || die > 20) {
			sprintf(buff, "%s: Number of die must be 1-20", from);
		}
		else {
			srand((unsigned) time(&t));
			sprintf(buff, "");

			for (int i = 0; i < die; i++) {
				int result = (rand() % sides) + 1;
				int length = snprintf(NULL, 0, "%i", result);
				char *result_string = malloc(length + 1);
				snprintf(result_string, length + 1, "%d", result);
				strcat(buff, result_string);
				free(result_string);

				if (i < die - 1) {
					strcat(buff, ", ");
				}
			}
		}

		ircMessage(channel, buff);
	} else if (strstr(message, "arne")) {
		/*if (rand() % 5)
			ircMessage(channel, "arne");*/
	} else if (!strcasecmp(from, "wally") && (tmp = strcasestr(message, "pls"))) {
		if (((tmp != message && tmp[-1] == ' ') || tmp == message) && (tmp[4] == ' ' || !tmp[4]))
			ircMessage(channel, "PLS U");
	}
	

	return;
}
