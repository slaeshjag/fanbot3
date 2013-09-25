#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <config/api.h>

#include <sys/types.h>
#include <sys/stat.h>

#define	min(x, y)		((x) < (y) ? (x) : (y))


#if 0
int stirling(int n, int k, time_t t) {
	if (time(NULL) - t > 5)
		return -1;
	if (k == 1 || n == k)
		return 1;
	if (!(1 < k && k < n))
		return 0;
	return stirling(n - 1, k - 1, t) + k * stirling(n - 1, k, t);
}
#endif


double stirling(int n, int k, time_t dummy) {
	double *generation;
	double result;
	int i, j;

	if ((generation = calloc(k+1, sizeof(double))) == NULL) {
		printf("Calloc failed!\n");
		return -1;
	}

	generation[0] = 1;
	for (i = 1; i <= n; i++) {
		for (j = min(i, k); j > 0; j--) {
			generation[j] = j*generation[j] + generation[j-1];
		}
	generation[0] = 0;
	}

	result = generation[k];
	free(generation);
	return result;
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
	time_t t;

	if (strcmp(command, "PRIVMSG") != 0)
		return;
	channel = ircGetIntendedChannel(channel, from);
	
	if (strcmp(message, API_HELP_CMD) == 0)
		sendHelp(from);
	if (strstr(message, "<s ") == message) {
		t = time(NULL);
		i = j = 0;
		sscanf(message, "<s %i, %i", &i, &j);
		k = stirling(i, j, time(NULL));
		if (time(NULL) - t >= 5)
			sprintf(buff, "S(%i, %i) took too long .-.", i, j);
		else
			sprintf(buff, "S(%i, %i) = %i", i, j, k);
		ircMessage(channel, buff);
	}
	

	return;
}
