#include "config.h"


void timerInit() {
	struct NETWORK_ENTRY *network;

	if ((network = networkFind(config->net.network_active)) == NULL)
		return;
	
	network->timer = NULL;
	network->timers = 0;
	network->timer_e.last_ping = time(NULL);
}


int timerAdd(time_t when, const char *plugin) {
	struct NETWORK_ENTRY *network;
	struct TIMER_ENTRY *timer;

	if ((network = networkFind(config->net.network_active)) == NULL) {
		return -1;
	}
	if (when <= time(NULL))
		return -1;
	
	if ((timer = malloc(sizeof(struct TIMER_ENTRY))) == NULL)
		return -1;
	timer->when = when;
	timer->plugin = plugin;
	
	timer->next = network->timer;
	network->timer = timer;
	timer->id = network->timers;
	network->timers++;

	return timer->id;
}


void timerDelete(int id) {
	struct NETWORK_ENTRY *network;
	struct TIMER_ENTRY *timer, *old;

	if ((network = networkFind(config->net.network_active)) == NULL)
		return;
	if (network->timer == NULL)
		return;
	if (network->timer->id == id) {
		timer = network->timer;
		network->timer = timer->next;
		free(timer);
		return;
	}

	old = network->timer;
	timer = old->next;
	while (timer != NULL) {
		if (timer->id == id) {
			old->next = timer->next;
			free(timer);
			return;
		}
		old = timer;
		timer = timer->next;
	}

	return;
}



void timerDeleteAll() {
	struct NETWORK_ENTRY *network;
	struct TIMER_ENTRY *timer, *next;

	if ((network = networkFind(config->net.network_active)) == NULL)
		return;
	
	timer = network->timer;
	network->timer = NULL;
	while (timer != NULL) {
		next = timer->next;
		free(timer);
		timer = next;
	}

	return;
}


void timerProcess() {
	struct NETWORK_ENTRY *network;
	NETWORK_PLUGIN_DATA *plug_data;
	struct TIMER_ENTRY *timer;
	struct PLUGIN_FILTER_ENTRY *filter;
	time_t now;
	
	if ((network = networkFind(config->net.network_active)) == NULL) {
		configErrorPush("Couldn't find the network o_O");
		return;
	}

	if (now - network->timer_e.last_ping > 600) {
		network->timer_e.last_ping = now;
		ircPing("Connection alive?");
	}

	timer = network->timer;
	now = time(NULL);
	while (timer != NULL) {
		if (timer->when > now);
		else if ((filter = filterFind(timer->plugin)) == NULL) {
			timerDelete(timer->id);
		} else if ((plug_data = networkPluginDataGet(filter->name)) == NULL) {
			configErrorPush("Couldn't get plugin data for the filter. This shouldn't happen.");
			timerDelete(timer->id);
		} else {
			(filter->timerPoke)(plug_data->handle, timer->id);
		}

		timer = timer->next;
	}

	return;
}
