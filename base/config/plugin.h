#ifndef __PLUGIN_H__
#define	__PLUGIN_H__


struct PLUGIN_NETWORK_ENTRY {
	void				*lib_handle;
	const char			*name;
	void				*(*connect)(const char *host, int port);
	int				(*socket)(void *handle);
	int				(*read)(void *handle, char *buffer, int buffer_len, int *error);
	int				(*write)(void *handle, char *buffer, int buffer_len, int *error);
	void				*(*disconnect)(void *handle);
	struct PLUGIN_NETWORK_ENTRY	*next;
};


struct PLUGIN_FILTER_ENTRY {
	void				*lib_handle;
	const char			*name;
	int				trig_type;
	void				*(*init)();
	void				(*filter)(void *handle, const char *from, const char *host,  const char *channel, const char *message);
	void				*(*destroy)(void *handle);
	struct PLUGIN_FILTER_ENTRY	*next;
};


typedef struct {
	struct PLUGIN_NETWORK_ENTRY	*network_plug;
	struct PLUGIN_FILTER_ENTRY	*filter_plug;
} PLUGIN;


void pluginInit();
struct PLUGIN_NETWORK_ENTRY *pluginFindNetwork(const char *name);
void pluginCrawl(const char *path);


#endif
