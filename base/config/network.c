#include "config.h"


struct NETWORK_ENTRY *networkFind(const char *name) {
	struct NETWORK_ENTRY *next;

	next = config->network;

	while (next != NULL) {
		if (strcmp(name, next->name) == 0)
			break;
		next = next->next;
	}

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
	network->socket = -1;
	network->ready = NETWORK_NOT_CONNECTED;
	*network->active_buffer = 0;
	*network->process_buffer = 0;
	network->buff_pos = 0;
	network->disconnect = 0;
	networkPluginInit(name);

	network->next = config->network;
	config->network = network;

	fprintf(stderr, "Debug: Added network %s\n", name);

	return;
}


void networkProcess(struct NETWORK_ENTRY *network) {
	int i, j, k, error = 0;
	do {
		i = layerRead(network->layer, network->network_handle, &network->process_buffer[network->buff_pos], 511 - network->buff_pos, &error);
		if (i <= 0) {
			if (error == EWOULDBLOCK)
				break;	
			configErrorPush("Connection reset");
			networkDisconnect(network->name, "Socket error");
			return;
		}

		network->process_buffer[i + network->buff_pos] = 0;
	
		for (j = k = 0; j < i + network->buff_pos; j++) {
			if (network->process_buffer[j] == '\r') {
				network->process_buffer[j] = ' ';
			}
			if (network->process_buffer[j] == '\n') {
				memcpy(network->active_buffer, &network->process_buffer[k], j - k);
				network->active_buffer[j - k] = 0;
				ircLine(network);
				k = j + 1;
			}
		}

		memmove(network->process_buffer, &network->process_buffer[k], network->buff_pos + i - k);
		network->process_buffer[network->buff_pos + i - k] = 0;
		network->buff_pos = network->buff_pos + i - k;
	} while (1);

	return;
}


void networkInit() {
	config->network = NULL;

	return;
}


void networkPlugindataDelete(const char *name) {
	struct NETWORK_ENTRY *network;
	int i;

	if ((network = networkFind(name)) == NULL)
		return;
	
	if (network->plugin == NULL)
		return;

	for (i = 0; i < config->plugin.filters; i++)
		filterDestroy(network->plugin[i].name, network->plugin[i].handle);

	free(network->plugin);
	network->plugin = NULL;
}


void networkPluginInit(const char *name) {
	struct NETWORK_ENTRY *network;
	struct PLUGIN_FILTER_ENTRY *filter;
	int i;

	if ((network = networkFind(name)) == NULL)
		return;

	if ((network->plugin = malloc(sizeof(NETWORK_PLUGIN_DATA) * config->plugin.filters)) == NULL) {
		configErrorPush("Unable to malloc()");
		return;
	}

	filter = config->plugin.filter_plug;
	for (i = 0; filter != NULL; i++) {
		network->plugin[i].handle = filterInit(name, network->name);
		network->plugin[i].name = filter->name;
		filter = filter->next;
	}
	
	return;
}


void networkDeleteAll(const char *reason) {
	struct NETWORK_ENTRY *network, *old;
	struct NETWORK_CHANNEL *chan, *oldchan;

	network = config->network;
	config->network = NULL;

	while (network != NULL) {
		chan = network->channel;
		network->channel = NULL;
		while (chan != NULL) {
			oldchan = chan;
			chan = chan->next;
			free(oldchan);
		}

		networkDisconnect(network->name, reason);
		old = network;
		network = network->next;
		free(old);
	}

	return;
}


void networkDisconnect(const char *name, const char *reason) {
	struct NETWORK_ENTRY *network;

	if ((network = networkFind(name)) == NULL)
		return;
	
	if (network->ready != NETWORK_CONNECTING)
		return;
	
	ircQuit(reason);
	network->network_handle = layerDisconnect(network->layer, network->network_handle);
	network->socket = -1;
	network->ready = NETWORK_NOT_CONNECTED;
	network->disconnect = time(NULL);

	networkPlugindataDelete(name);

	return;
}


void networkDisconnectAll(const char *reason) {
	struct NETWORK_ENTRY *network;

	network = config->network;
	while (network != NULL) {
		networkDisconnect(network->name, reason);
		network = network->next;
	}

	return;
}


void networkConnect(const char *name) {
	struct NETWORK_ENTRY *network;
	unsigned int flags;
	
	if ((network = networkFind(name)) == NULL)
		return;
	if ((network->network_handle = layerConnect(network->layer, network->host, network->port)) == NULL) {
		network->disconnect = time(NULL);
		configErrorPush("Unable to connect");
		return;
	}

	network->ready = NETWORK_CONNECTING;
	network->socket = layerSocketGet(network->layer, network->network_handle);

	/* Make socket non-blocking */
	if ((flags = fcntl(network->socket, F_GETFL, 0)) == -1)
		flags = 0;
	fcntl(network->socket, F_SETFL, flags | O_NONBLOCK);

	network->buff_pos = 0;
	networkPluginInit(name);
	
	return;
}


void networkConnectAll() {
	struct NETWORK_ENTRY *network;

	network = config->network;
	while (network != NULL) {
		networkConnect(network->name);
		network = network->next;
	}

	return;
}


void networkWait() {
	struct NETWORK_ENTRY *next;
	struct NETWORK_CHANNEL *channel;
	int max_fd;
	
	next = config->network;
	FD_ZERO(&config->net.read);
	max_fd = 0;

	while (next != NULL) {
		if (next->ready != NETWORK_NOT_CONNECTED) {
			FD_SET(next->socket, &config->net.read);
			if (max_fd < next->socket) max_fd = next->socket;
		}

		next = next->next;
	}

	max_fd++;
	config->net.time.tv_sec = 1;
	config->net.time.tv_usec = 0;

	select(max_fd, &config->net.read, NULL, NULL, &config->net.time);
	
	next = config->network;
	while (next != NULL) {
		config->net.network_active = next->name;
		if (next->ready == NETWORK_NOT_CONNECTED);
		else if (FD_ISSET(next->socket, &config->net.read)) {
			networkProcess(next);
			if (next->ready == NETWORK_JOIN) {
				channel = channel;
				while (channel != NULL) {
					ircJoin(channel->name, channel->key);
					channel = channel->next;
				}
				next->ready = NETWORK_READY;
			} else if (next->ready == NETWORK_CONNECTING) {
				ircNick(next->nick);
				next->ready = NETWORK_JOIN;
			}
		}
		next = next->next;
	}

	return;
}
