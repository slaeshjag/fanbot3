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


typedef struct {
	PLUGIN				plugin;
	struct NETWORK_ENTRY		*network;
} CONFIG;


CONFIG *config;

#endif
