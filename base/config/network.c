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


struct NETWORK_CHANNEL *networkChannel(const char *network, const char *channel) {
	struct NETWORK_ENTRY *network_e;
	struct NETWORK_CHANNEL *channel_e;

	if ((network_e = networkFind(network)) == NULL) 
		return NULL;

	channel_e = network_e->channel;
	while (channel_e != NULL) {
		if (strcmp(channel, channel_e->name) == 0)
			return channel_e;
		channel_e = channel_e->next;
	}

	return NULL;
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


void networkReconnDelaySet(const char *name, int delay) {
	struct NETWORK_ENTRY *next;

	if ((next = networkFind(name)) == NULL)
		return;
	next->reconnect_delay = delay;
	
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
	stringToUpper(channel->name);
	strncpy(channel->key, key, 128);
	channel->last_sent = 0;
	channel->cap = NETWORK_CHANNEL_SEND_CAP;
	channel->next = network->channel;
	channel->start = channel->end = 0;
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
	network->reconnect_delay = 600;
	*network->active_buffer = 0;
	*network->process_buffer = 0;
	network->buff_pos = 0;
	network->disconnect = 0;

	network->next = config->network;
	config->network = network;

	fprintf(stderr, "Debug: Added network %s\n", name);

	return;
}


void networkProcess(struct NETWORK_ENTRY *network) {
	int i, j, k, error = 0;
	do {
		if (network->ready == NETWORK_NOT_CONNECTED)
			return;
		i = layerRead(network->layer, network->network_handle, &network->process_buffer[network->buff_pos], 512 - network->buff_pos, &error);
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

	timerProcess();

	return;
}


void networkInit() {
	config->network = NULL;

	return;
}


NETWORK_PLUGIN_DATA *networkPluginDataGet(const char *name) {
	struct NETWORK_ENTRY *network;
	int i;

	if ((network = networkFind(config->net.network_active)) == NULL)
		return NULL;
	if (network->plugin == NULL)
		return NULL;
	
	for (i = 0; i < config->plugin.filters; i++) {
		if (strcmp(network->plugin[i].name, name) == 0)
			return &network->plugin[i];
	}

	return NULL;
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
	if (network->ready == NETWORK_NOT_CONNECTED)
		return;

	if ((network->plugin = malloc(sizeof(NETWORK_PLUGIN_DATA) * config->plugin.filters)) == NULL) {
		configErrorPush("Unable to malloc()");
		return;
	}

	filter = config->plugin.filter_plug;
	for (i = 0; filter != NULL; i++) {
		network->plugin[i].handle = filterInit(filter->name, network->name);
		network->plugin[i].name = filter->name;
		filter = filter->next;
	}
	
	return;
}


void networkPluginInitAll() {
	struct NETWORK_ENTRY *network;
	
	network = config->network;
	while (network != NULL) {
		networkPluginInit(network->name);
		network = network->next;
	}
	
	return;
}


void networkDeleteAll(const char *reason) {
	struct NETWORK_ENTRY *network, *old;
	struct NETWORK_CHANNEL *chan, *oldchan;

	network = config->network;

	while (network != NULL) {
		config->net.network_active = network->name;
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
	config->network = NULL;

	return;
}


void networkDisconnect(const char *name, const char *reason) {
	struct NETWORK_ENTRY *network;

	if ((network = networkFind(name)) == NULL) 
		return;
	
	if (network->ready == NETWORK_NOT_CONNECTED)
		return;
	
	ircQuit(reason);
	network->network_handle = layerDisconnect(network->layer, network->network_handle);
	close(network->socket);
	network->ready = NETWORK_NOT_CONNECTED;
	network->disconnect = time(NULL);

	timerDeleteAll();
	networkPlugindataDelete(name);

	return;
}


void networkDisconnectAll(const char *reason) {
	struct NETWORK_ENTRY *network;

	network = config->network;
	while (network != NULL) {
		config->net.network_active = network->name;
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
	timerInit();
	networkPluginInit(name);
	
	return;
}


void networkConnectAll() {
	struct NETWORK_ENTRY *network;

	network = config->network;
	while (network != NULL) {
		config->net.network_active = network->name;
		networkConnect(network->name);
		network = network->next;
	}

	config->net.network_active = "CONFIG";

	return;
}


const char *networkNick() {
	struct NETWORK_ENTRY *network;

	network = config->network;
	if ((network = networkFind(config->net.network_active)) == NULL)
		return "Unknown";
	return network->nick;
}


void networkPushLine(const char *network, const char *channel, const char *buffer) {
	struct NETWORK_ENTRY *network_e;
	struct NETWORK_CHANNEL *channel_e;
	int i, error;
	time_t now = time(NULL);
	
	if ((network_e = networkFind(network)) == NULL)
		return;
	if ((channel_e = networkChannel(network, channel)) == NULL);
	else {
		i = channel_e->end;
		if ((i + 1) % NETWORK_BUFFER_QUEUE == channel_e->start)
			return;
		if (channel_e->cap == 0 && channel_e->last_sent == now) {
			strcpy(&channel_e->buffer[i * 513], buffer);
			channel_e->end = (i + 1 == NETWORK_BUFFER_QUEUE) ? 0 : i + 1;
			return;
		}
	
		if (channel_e->last_sent != now) {
			channel_e->last_sent = now;
			channel_e->cap = NETWORK_CHANNEL_SEND_CAP;
		}
	
		channel_e->cap--;

		layerWrite(network_e->layer, network_e->network_handle, buffer, strlen(buffer), &error);

		return;
	}
	
	layerWrite(network_e->layer, network_e->network_handle, buffer, strlen(buffer), &error);
	
	return;
}


void networkProcessBuffers() {
	struct NETWORK_ENTRY *network;
	struct NETWORK_CHANNEL *channel;
	time_t now = time(NULL);
	int i, error;

	if ((network = networkFind(config->net.network_active)) == NULL)
		return;
	
	channel = network->channel;
	while (channel != NULL) {
		if (channel->last_sent == now);
		else {
			channel->cap = NETWORK_CHANNEL_SEND_CAP;
			channel->last_sent = now;
			for (i = channel->start; i != channel->end && channel->cap > 0; i++) {
				if (i == NETWORK_BUFFER_QUEUE) i = 0;
				if (i == channel->end) break;
				layerWrite(network->layer, network->network_handle, &channel->buffer[i * 513], strlen(&channel->buffer[i * 513]), &error);
				channel->cap--;
			}
			if (i == NETWORK_BUFFER_QUEUE) i = 0;
			channel->start = i;
		}
		channel = channel->next;
	}

	return;
}


void networkReconnect() {
	struct NETWORK_ENTRY *next;
	time_t now;

	now = time(NULL);

	if ((next = networkFind(config->net.network_active)) == NULL)
		return;
	if (next->ready != NETWORK_NOT_CONNECTED)
		return;
	if (now - next->disconnect < next->reconnect_delay)
		return;

	configErrorPush("Reconnecting now");
	networkConnect(config->net.network_active);

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
			if (next->ready == NETWORK_NOT_CONNECTED)
				continue;
			if (next->ready == NETWORK_CONNECTING) {
				ircNick(next->nick);
				next->disconnect = time(NULL);
				next->ready = NETWORK_JOIN;
			}
		} else if (next->ready == NETWORK_JOIN) {
				channel = next->channel;
				while (channel != NULL) {
					ircJoin(channel->name, channel->key);
					channel->start = channel->end = 0;
					channel->cap = NETWORK_CHANNEL_SEND_CAP;
					channel = channel->next;
				}
				next->ready = NETWORK_READY;
		}
		networkProcessBuffers();
		networkReconnect();
		next = next->next;
	}

	return;
}
