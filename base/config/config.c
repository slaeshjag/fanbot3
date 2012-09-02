#include "config.h"


void configProcess(const char *command, const char *arg1, const char *arg2) {
	if (strcmp(command, "plugin") == 0) {
		if (strcmp(arg1, "scan") == 0)
			pluginCrawl(arg2);
	}

	return;
}


int configRead(const char *path) {
	FILE *fp;
	char command[64], arg1[128], arg2[128], buff[512];

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
		configProcess(command, arg1, arg2);
	}

	return 0;
}


void configErrorPush(const char *error) {
	/* TODO: Make queue system */
	fprintf(stderr, error);
	
	return;
}


void *init() {
	if ((config = malloc(sizeof(CONFIG))) == NULL) {
		fprintf(stderr, "[CONFIG] First malloc failed. Let's just give up :P\n");
		return NULL;
	}

	pluginInit();
	configRead("base/fanbot3.conf");

	return NULL;
}
