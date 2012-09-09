#ifndef __CONFIG_H__
#define	__CONFIG_H__


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <signal.h>
#include <time.h>

#include "api.h"
#include "plugin.h"
#include "network.h"
#include "layer.h"
#include "irc.h"
#include "filter.h"
#include "timer.h"


#define	CONFIG_PLUGIN_NETWORK		0x1
#define	CONFIG_PLUGIN_FILTER		0x2
#define	CONFIG_NETWORKS			0x4

#define	CONFIG_ALL			0xF

typedef struct {
	PLUGIN				plugin;
	struct NETWORK_ENTRY		*network;
	NETWORK_MAIN			net;
	char				parse_buf[64];
	int				reload;
	int				reload_filters;
} CONFIG;

int configRead(const char *path, unsigned int what);
void stringToUpper(char *string);


CONFIG *config;

#endif
