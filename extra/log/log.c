#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <config/api.h>

#include <sys/types.h>
#include <sys/stat.h>


unsigned int pluginType() {
	return PLUGIN_TYPE_FILTER;
}


const char *pluginName() {
	return "simple-logger";
}


void *pluginDestroy(void *handle) {
	if (handle)
		fclose(handle);
	return NULL;
}


void *pluginDoInit(const char *network) {
	char blah[512];
	mkdir("data/log", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	
	sprintf(blah, "data/log/%s", network);
	return fopen(blah, "a");
}


void pluginTimerPoke(void *handle, int id) {
	return;
}


void pluginFilter(void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	if (!handle)
		return;

	fprintf(handle, "%s, %s: <%s> %s\n", command, channel, from, message);

	return;
}
