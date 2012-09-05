#ifndef __LEGACY_H__
#define	__LEGACY_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <config/api.h>

#include "net.h"


#define	COMMANDLIST		"http://1.rdw.se:81/commandlist"
#define	HOST_PREFIX		"http://1.rdw.se:81/"

typedef struct {
	char			*buffer;
	char			**command;
	char			**file;
	int			commands;
} LEGACY;


#endif
