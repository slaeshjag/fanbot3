#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <config/api.h>

#include <sys/types.h>
#include <sys/stat.h>


int stirling(int n, int k, time_t t) {
	if (time(NULL) - t > 5)
		return -1;
	if (k == 1 || n == k)
		return 1;
	if (!(1 < k && k < n))
		return 0;
	return stirling(n - 1, k - 1, t) + k * stirling(n - 1, k, t);
}


void sendHelp(const char *from) {
	ircMessage(from, "<s <n>, <k> - Calculate S(n, k)");
	return;
}


unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "math";
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
	char buff[520];
	int i, j, k;

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	channel = ircGetIntendedChannel(channel, from);
	
	if (strcmp(message, API_HELP_CMD) == 0)
		sendHelp(from);
	if (strstr(message, "<s ") == message) {
		i = j = 0;
		sscanf(message, "<s %i, %i", &i, &j);
		k = stirling(i, j, time(NULL));
		if (k >= 0)
			sprintf(buff, "S(%i, %i) = %i", i, j, k);
		else
			sprintf(buff, "S(%i, %i) took too long .-.", i, j);
		ircMessage(channel, buff);
	}
	

	return;
}
