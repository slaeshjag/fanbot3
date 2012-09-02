#include "config.h"


void pluginInit() {
	config->plugin.network_plug = NULL;
	config->plugin.filter_plug = NULL;

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


void pluginAddFilter(void *lib_handle, const char *name) {
	struct PLUGIN_FILTER_ENTRY *plugin;
	int (*trig_type)();

	if ((trig_type = dlsym(lib_handle, "pluginFilterType")) == NULL) {
		dlclose(lib_handle);
		return;
	}

	if ((plugin = malloc(sizeof(struct PLUGIN_NETWORK_ENTRY))) == NULL) {
		dlclose(lib_handle);
		return;
	}


	plugin->lib_handle = lib_handle;
	plugin->name = name;
	plugin->trig_type = (trig_type)();

	plugin->init = dlsym(lib_handle, "pluginDoInit");
	plugin->filter = dlsym(lib_handle, "pluginFilter");
	plugin->destroy = dlsym(lib_handle, "pluginDestroy");

	if (!plugin->init || !plugin->filter || !plugin->destroy) {
		dlclose(lib_handle);
		free(plugin);
		return;
	}

	plugin->next = config->plugin.filter_plug;
	config->plugin.filter_plug = plugin;

	fprintf(stderr, "Debug: Filter plugin %s added\n", name);

	return;
}


void pluginProcess(const char *path, const char *name) {
	void *lib_handle;
	char fname[512], *longname, *usename;
	unsigned int (*pluginType)();
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

	switch ((pluginType)()) {
		case PLUGIN_TYPE_NETWORK:
			pluginAddNetwork(lib_handle, (pluginName)());
			break;
		case PLUGIN_TYPE_FILTER:
			pluginAddFilter(lib_handle, (pluginName)());
			break;
		default:
			dlclose(lib_handle);
			fprintf(stderr, "[CONFIG] %s/%s is of a type not implemented\n", path, name);
			break;
	}

	return;
}


void pluginCrawl(const char *path) {
	DIR *dir;
	struct dirent *file;
	
	dir = opendir(path);

	do {
		file = readdir(dir);
		if (file == NULL)
			break;
		pluginProcess(path, file->d_name);
	} while (1);

	closedir(dir);

	return;
}
