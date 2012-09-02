#include "config.h"


struct NETWORK_ENTRY *networkFind(const char *name) {
	struct NETWORK_ENTRY *next;

	next = config->network;

	do {
		if (strcmp(name, next->name) == 0)
			break;
		next = next->next;
	} while (next != NULL);

	return next;
}


void networkHostSet(const char *name, const char *host) {
	struct NETWORK_ENTRY *next;

	if ((next = networkFind(name)) == NULL)
		return;
	strncpy(next->host, host, 128);

	return;
}


void networkPortSet(const char *name, int port) {
	struct NETWORK_ENTRY *next;

	if ((next = networkFind(name)) == NULL)
		return;
	next->port = port;
	
	return;
}


void networkNickSet(const char *name, const char *nick) {
	struct NETWORK_ENTRY *next;

	if ((next = networkFind(name)) == NULL)
		return;
	strncpy(next->nick, nick, 64);

	return;
}


void networkLayerSet(const char *name, const char *layer) {
	struct NETWORK_ENTRY *next;

	if ((next = networkFind(name)) == NULL)
		return;
	strncpy(next->layer, layer, 64);
	
	return;
}


void networkChannelAdd(const char *name, const char *chan, const char *key) {
	struct NETWORK_ENTRY *network;
	struct NETWORK_CHANNEL *channel;

	if ((network = networkFind(name)) == NULL)
		return;

	if ((channel = malloc(sizeof(struct NETWORK_CHANNEL))) == NULL)
		return;
	
	strncpy(channel->name, chan, 128);
	strncpy(channel->key, key, 128);
	channel->last_sent = 0;
	channel->cap = NETWORK_CHANNEL_SEND_CAP;
	channel->next = network->channel;
	network->channel = channel;

	fprintf(stderr, "Debug: Added channel %s <%s> to network %s\n", chan, key, name);

	return;
}


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
