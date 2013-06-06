#include "config.h"


void configProcess(const char *command, const char *arg1, const char *arg2, unsigned int what) {
	char *network = config->parse_buf;

	if (strcmp(command, "}") == 0)
		*network = 0;
	else if (strlen(network) != 0) {
		if (strcmp(command, "nick") == 0)
			networkNickSet(network, arg1);
		else if (strcmp(command, "layer") == 0)
			networkLayerSet(network, arg1);
		else if (strcmp(command, "server") == 0)
			networkHostSet(network, arg1);
		else if (strcmp(command, "port") == 0)
			networkPortSet(network, atoi(arg1));
		else if (strcmp(command, "channel") == 0)
			networkChannelAdd(network, arg1, arg2);
		else if (strcmp(command, "identify") == 0)
			networkIdentifyAdd(network, arg1, arg2);
		else if (strcmp(command, "reconnect") == 0) {
			if (strcmp(arg1, "delay") == 0)
				networkReconnDelaySet(network, atoi(arg2));
		}
	} else if (strcmp(command, "plugin") == 0) {
		if (strcmp(arg1, "scan") == 0)
			pluginCrawl(arg2, what);
	} else if (strcmp(command, "network") == 0 && (what & CONFIG_NETWORKS)) {
		strncpy(config->parse_buf, arg1, 64);
		networkAdd(arg1);
	}


	return;
}


int configRead(const char *path, unsigned int what) {
	FILE *fp;
	char command[64], arg1[128], arg2[128], buff[512];

	*config->parse_buf = 0;
	if (what == CONFIG_ALL)
		networkInit();

	if ((fp = fopen(path, "r")) == NULL) {
		fprintf(stderr, "[CONFIG] Unable to open configuration file %s. We're doomed\n", path);
		return -1;
	}

	while (!feof(fp)) {
		*buff = 0;
		*command = 0;
		*arg1 = 0;
		*arg2 = 0;

		fgets(buff, 512, fp);
		sscanf(buff, "%s %s %s\n", command, arg1, arg2);
		configProcess(command, arg1, arg2, what);
	}

	return 0;
}


void configErrorPush(const char *error) {
	struct tm *time_rep;
	time_t time_sec;

	time_sec = time(NULL);
	time_rep = localtime(&time_sec);

	fprintf(stderr, "(%04i-%02i-%02i %02i:%02i:%02i) [%s] %s\n", time_rep->tm_year + 1900, time_rep->tm_mon + 1, time_rep->tm_mday, time_rep->tm_hour, time_rep->tm_min, time_rep->tm_sec, config->net.network_active, error);
	
	return;
}


void *init() {
	if ((config = malloc(sizeof(CONFIG))) == NULL) {
		fprintf(stderr, "[CONFIG] First malloc failed. Let's just give up :P\n");
		return NULL;
	}

	config->net.network_active = "CONFIG";

	pluginInit();
	configRead("conf/fanbot3.conf", CONFIG_ALL);
	networkConnectAll();
	config->reload = config->reload_filters = 0;

	for (;!config->reload;) {
		networkWait();
		if (config->reload_filters) {
			config->net.network_active = "CONFIG";
			filterReload("conf/fanbot3.conf");
		}
	}


	return config;
}


void reload() {
	config->reload = 1;

	return;
}


void configFilterReload() {
	config->reload_filters = 1;
	
	return;
}


void destroy(void *handle, const char *reason) {
	/* TODO: Implement */

	fprintf(stderr, "\nDestroying myself..\n");
	pluginNetworkUnload(reason);
	pluginFilterUnload();

	free(config);

	return;
}


void stringToUpper(char *string) {
	int i;

	if (string == NULL)
		return;

	for (i = 0; string[i] != 0; i++)
		if (string[i] >= 'a' && string[i] <= 'z')
			string[i] -= ('a' - 'A');
	
	return;
}
