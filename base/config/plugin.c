#include "config.h"


void pluginInit() {
	config.plugin.network_plug = config.plugin.filter_plug = NULL;

	return;
}


void pluginProcess(const char *path, const char *file->d_name) {
	
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
	}

	closedir(dir);

	return;
}
