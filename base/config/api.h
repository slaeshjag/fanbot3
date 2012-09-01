#ifndef __CONFIG_API_H__
#define	__CONFIG_API_H__


#define		PLUGIN_TYPE_NETWORK		0x1


void configErrorPush(const char *err);


/* Common API for plugins */
unsigned int pluginType();
const char *pluginName();


#ifndef __NETWORK_H__
	/* API for network plugins */
	void *pluginConnect(const char *host, int port);
	int pluginSocketGet(void *connection);
	int pluginReadData(void *connection, char *buffer, int buffer_len);
	int pluginSendData(void *connection, const char *buffer, int buffer_len);
	void *pluginSocketDone(void *connection);
#endif


#endif
