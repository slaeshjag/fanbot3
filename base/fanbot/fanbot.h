#ifndef __FANBOT_H__
#define	__FANBOT_H__


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <signal.h>


typedef struct {
	void			*(*init)();
	void			(*reload)();
	int			(*destroy)(void *handle, const char *reason);
	void			*library;
	void			*handle;
} FANBOT;

FANBOT fanbot;


#endif
