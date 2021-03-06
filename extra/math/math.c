#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <config/api.h>

#include <sys/types.h>
#include <sys/stat.h>

#define	min(x, y)		((x) < (y) ? (x) : (y))


double fibonacci(int n) {
	double fib1, fib2, fibt;
	int i;

	if (n > 100000)
		return -1.0;

	fib1 = 0;
	fib2 = fibt = 1;
	for (i = 1; i < n; i++) {
		fibt = fib1 + fib2;
		fib1 = fib2;
		fib2 = fibt;
	}

	return fibt;
}


double stirling(int n, int k, time_t dummy) {
	double *generation;
	double result;
	int i, j;
	time_t start;

	if ((generation = calloc(k+1, sizeof(double))) == NULL) {
		printf("Calloc failed!\n");
		return -1;
	}

	start = time(NULL);
	generation[0] = 1;
	for (i = 1; i <= n; i++) {
		for (j = min(i, k); j > 0; j--) {
			generation[j] = j*generation[j] + generation[j-1];
		}
		generation[0] = 0;
		if (time(NULL) > start + 5)
			return -1.0f;
			
	}

	result = generation[k];
	free(generation);
	return result;
}


void sendHelp(const char *from) {
	ircMessage(from, "<s <n>, <k> - Calculate S(n, k)");
	ircMessage(from, "<fibo n - Canculate the n:th fibonacci number");
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
	int i, j;
	double k;
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
			sprintf(buff, "S(%i, %i) = %f", i, j, k);
		ircMessage(channel, buff);
	} else if (strstr(message, "<fibo") == message) {
		i = 0;
		sscanf(message, "<fibo %i", &i);
		k = fibonacci(i);
		if (k > 0)
			sprintf(buff, "Fibonacci number %i is %.0f", i, k);
		else
			sprintf(buff, "Fibonacci number %i is inf", i);
		ircMessage(channel, buff);
	}
	

	return;
}
