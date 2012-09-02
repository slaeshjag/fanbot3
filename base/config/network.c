#include "config.h"


void networkAdd(const char *name) {
	struct NETWORK_ENTRY *network;

	if ((network = malloc(sizeof(struct NETWORK_ENTRY))) == NULL)
		return;
	
	strncpy(network->name, name, 64);
	*network->host = *network->layer = 0;
	network->port = 6667;
	strncpy(network->nick, "fanbot3", 64);
	network->network_handle = NULL;
	network->channel = NULL;
	network->next = 0;

	network->next = config->network;
	config->network = network;

	fprintf(stderr, "Debug: Added network %s\n", name);

	return;
}


void networkInit() {
	config->network = NULL;
	
	return;
}
