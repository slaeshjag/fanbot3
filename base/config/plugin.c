#include "config.h"



void pluginInit() {
	config->plugin.network_plug = NULL;
	config->plugin.filter_plug = NULL;
	config->plugin.filters = 0;

	return;
}


void pluginAddNetwork(void *lib_handle, const char *name) {
	struct PLUGIN_NETWORK_ENTRY *plugin;

	if ((plugin = malloc(sizeof(struct PLUGIN_NETWORK_ENTRY))) == NULL) {
		dlclose(lib_handle);
		return;
	}

	plugin->lib_handle = lib_handle;
	plugin->name = name;
	plugin->connect = dlsym(lib_handle, "pluginConnect");
	plugin->socket = dlsym(lib_handle, "pluginSocketGet");
	plugin->read = dlsym(lib_handle, "pluginReadData");
	plugin->write = dlsym(lib_handle, "pluginSendData");
	plugin->disconnect = dlsym(lib_handle, "pluginSocketDone");

	if (!plugin->connect || !plugin->socket || !plugin->read || !plugin->write || !plugin->disconnect) {
		dlclose(lib_handle);
		free(plugin);
		return;
	}

	plugin->next = config->plugin.network_plug;
	config->plugin.network_plug = plugin;

	fprintf(stderr, "Debug: Network plugin %s added\n", name);

	/* I think I'm done now o_O */
	return;
}


struct PLUGIN_NETWORK_ENTRY *pluginFindNetwork(const char *name) {
	struct PLUGIN_NETWORK_ENTRY *next;

	next = config->plugin.network_plug;
	while (next != NULL) {
		if (strcmp(name, next->name) == 0)
			return next;
		next = next->next;
	}

	return NULL;
}


void pluginAddFilter(void *lib_handle, const char *name) {
	struct PLUGIN_FILTER_ENTRY *plugin;

	if ((plugin = malloc(sizeof(struct PLUGIN_NETWORK_ENTRY))) == NULL) {
		dlclose(lib_handle);
		return;
	}


	plugin->lib_handle = lib_handle;
	plugin->name = name;

	plugin->init = dlsym(lib_handle, "pluginDoInit");
	plugin->filter = dlsym(lib_handle, "pluginFilter");
	plugin->timerPoke = dlsym(lib_handle, "pluginTimerPoke");
	plugin->destroy = dlsym(lib_handle, "pluginDestroy");

	if (!plugin->init || !plugin->filter || !plugin->destroy || !plugin->timerPoke) {
		dlclose(lib_handle);
		free(plugin);
		return;
	}

	plugin->next = config->plugin.filter_plug;
	config->plugin.filter_plug = plugin;
	config->plugin.filters++;

	fprintf(stderr, "Debug: Filter plugin %s added\n", name);

	return;
}


void pluginProcess(const char *path, const char *name, unsigned int what) {
	void *lib_handle;
	char fname[512], *longname, *usename;
	unsigned int (*pluginType)(), type;
	const char *(*pluginName)();

	longname = NULL;
	if (strlen(path) + strlen(name) + 2 > 512) {
		longname = malloc(strlen(path) + strlen(name) + 2);
		usename = longname;
	} else
		usename = fname;
	
	sprintf(usename, "%s/%s", path, name);
	if ((lib_handle = dlopen(usename, RTLD_LOCAL | RTLD_NOW)) == NULL) {
		/* This is not a plugin, ignore */
		free(longname);
		return;
	}

	free(longname);
	pluginType = dlsym(lib_handle, "pluginType");
	pluginName = dlsym(lib_handle, "pluginName");

	if (pluginType == NULL || pluginName == NULL) {
		dlclose(lib_handle);
		return;
	}

	type = (pluginType)();

	if (type == PLUGIN_TYPE_NETWORK && (what & CONFIG_PLUGIN_NETWORK))
		pluginAddNetwork(lib_handle, (pluginName)());
	else if (type == PLUGIN_TYPE_FILTER && (what & CONFIG_PLUGIN_FILTER))
		pluginAddFilter(lib_handle, (pluginName)());
	else
		dlclose(lib_handle);

	return;
}


void pluginFilterUnload() {
	struct NETWORK_ENTRY *network;
	struct PLUGIN_FILTER_ENTRY *filter, *old;
	
	network = config->network;
	while (network != NULL) {
		networkPlugindataDelete(network->name);
		network = network->next;
	}

	config->plugin.filters = 0;
	filter = config->plugin.filter_plug;
	config->plugin.filter_plug = NULL;
	while (filter != NULL) {
		dlclose(filter->lib_handle);
		old = filter;
		filter = filter->next;
		free(old);
	}

	return;
}


void pluginNetworkUnload(const char *reason) {
	struct PLUGIN_NETWORK_ENTRY *network, *old;
	
	networkDeleteAll(reason);

	network = config->plugin.network_plug;
	config->plugin.network_plug = NULL;
	while (network != NULL) {
		dlclose(network->lib_handle);
		old = network;
		network = network->next;
		free(old);
	}

	return;
}


void pluginCrawl(const char *path, unsigned int what) {
	DIR *dir;
	struct dirent *file;
	
	if ((dir = opendir(path)) == NULL)
		return;

	do {
		file = readdir(dir);
		if (file == NULL)
			break;
		pluginProcess(path, file->d_name, what);
	} while (1);

	closedir(dir);

	return;
}
