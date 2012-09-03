#ifndef __CONFIG_API_H__
#define	__CONFIG_API_H__

#include <errno.h>


#define		PLUGIN_TYPE_NETWORK		0x1
#define		PLUGIN_TYPE_FILTER		0x2


void configErrorPush(const char *err);


/* Common API for plugins */
unsigned int pluginType();
const char *pluginName();


#ifndef __NETWORK_H__
	/* API for network plugins */
	void *pluginConnect(const char *host, int port);
	int pluginSocketGet(void *connection);
	int pluginReadData(void *connection, char *buffer, int buffer_len, int *error);
	int pluginSendData(void *connection, const char *buffer, int buffer_len, int *error);
	void *pluginSocketDone(void *connection);
#endif


#ifndef __FILTER_H__
	/* API for filter plugins */
	void *pluginDoInit();
	void pluginFilter(void *handle, const char *from, const char *host, const char *channel, const char *message);
	void *pluginDestroy(void *handle);
#endif


#endif
