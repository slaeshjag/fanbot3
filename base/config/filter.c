#include "config.h"

struct PLUGIN_FILTER_ENTRY *filterFind(const char *name) {
	struct PLUGIN_FILTER_ENTRY *filter;

	filter = config->plugin.filter_plug;
	while (filter != NULL) {
		if (strcmp(name, filter->name) == 0)
			break;
		filter = filter->next;
	}

	return filter;
}


void *filterInit(const char *name, const char *network) {
	struct PLUGIN_FILTER_ENTRY *filter;

	if ((filter = filterFind(name)) == NULL)
		return NULL;
	
	return (filter->init)(network);
}


void filterDo(const char *name, void *handle, const char *from, const char *host, const char *command, const char *channel, const char *message) {
	struct PLUGIN_FILTER_ENTRY *filter;

	if ((filter = filterFind(name)) == NULL) {
		fprintf(stderr, "Filter does not exist!\n");
		return;
	}
	
	(filter->filter)(handle, from, host, command, channel, message);
	return;
}


void *filterDestroy(const char *name, void *handle) {
	struct PLUGIN_FILTER_ENTRY *filter;

	if ((filter = filterFind(name)) == NULL)
		return NULL;
	
	return (filter->destroy)(handle);
}


void filterProcess(const char *nick, const char *hoststr, const char *command, const char *arg, const char *string) {
	struct NETWORK_ENTRY *network;
	int i;

	if ((network = networkFind(config->net.network_active)) == NULL)
		return;
	
	for (i = 0; i < config->plugin.filters; i++)
		filterDo(network->plugin[i].name, network->plugin[i].handle, nick, hoststr, command, arg, string);
	
	return;
}
