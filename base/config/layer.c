#include "config.h"
/* Layer as in network layer */

void *layerConnect(const char *layer, const char *host, int port) {
	struct PLUGIN_NETWORK_ENTRY *network;

	if ((network = pluginFindNetwork(layer)) == NULL) {
		configErrorPush("Unable to connect using non-existing network layer");
		return NULL;
	}

	return (network->connect)(host, port);
}


int layerSocketGet(const char *layer, void *handle) {
	struct PLUGIN_NETWORK_ENTRY *network;

	if ((network = pluginFindNetwork(layer)) == NULL) {
		configErrorPush("Unable to get the socket of a non-existing network layer");
		return -1;
	}

	return (network->socket)(handle);
}


int layerRead(const char *layer, void *handle, char *buffer, int buffer_len, int *error) {
	struct PLUGIN_NETWORK_ENTRY *network;

	if ((network = pluginFindNetwork(layer)) == NULL) {
		configErrorPush("Unable to read data using a non-existing network layer");
		return -1;
	}

	return (network->read)(handle, buffer, buffer_len, error);
}


int layerWrite(const char *layer, void *handle, const char *buffer, int buffer_len, int *error) {
	struct PLUGIN_NETWORK_ENTRY *network;

	if ((network = pluginFindNetwork(layer)) == NULL) {
		configErrorPush("Unable to write data using a non-existing network layer");
		return -1;
	}

	return (network->write)(handle, buffer, buffer_len, error);
}


void *layerDisconnect(const char *layer, void *handle) {
	struct PLUGIN_NETWORK_ENTRY *network;

	if ((network = pluginFindNetwork(layer)) == NULL) {
		configErrorPush("Unable to disconnect using a non-existing network layer");
		return NULL;
	}

	return (network->disconnect)(handle);
}
