#ifndef __PLUGIN_H__
#define	__PLUGIN_H__


struct PLUGIN_NETWORK_ENTRY {
	void				*lib_handle;
	const char			*name;
	void				*(*connect)(const char *host, int port);
	int				(*socket)(void *handle);
	int				(*read)(void *handle, char *buffer, int buffer_len);
	int				(*write)(void *handle, char *buffer, int buffer_len);
	void				*(*disconnect)(void *handle);
	struct PLUGIN_NETWORK_ENTRY	*next;
};


struct PLUGIN_FILTER_ENTRY {
	void				lib_handle;
	const char			*name;
	int				trig_type;
	void				*(*init)();
	void				(*filter)(void *handle, const char *from, const char *host,  const char *channel, const char *message);
	void				*(destroy)(void *handle);
};


typedef struct {
	PLUGIN_NETWORK_ENTRY		*network_plug;
	PLUGIN_FILTER_ENTRY		*filter_plug;
} PLUGIN;


#endif
