#ifndef __CONFIG_H__
#define	__CONFIG_H__


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <dlfcn.h>
#include <signal.h>

#include "api.h"
#include "plugin.h"
#include "network.h"
#include "layer.h"


typedef struct {
	PLUGIN				plugin;
	struct NETWORK_ENTRY		*network;
	NETWORK_MAIN			net;
	char				parse_buf[64];
} CONFIG;


CONFIG *config;

#endif
