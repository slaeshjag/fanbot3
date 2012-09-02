#ifndef __NETWORK_H__
#define	__NETWORK_H__


#define	NETWORK_CHANNEL_SEND_CAP		5


struct NETWORK_CHANNEL {
	char			name[128];
	char			key[128];
	int			last_sent;
	int			cap;
	struct NETWORK_CHANNEL	*next;
};


struct NETWORK_ENTRY {
	char			name[64];
	char			host[128];
	int			port;
	char			nick[64];
	char			layer[64];
	
	void			*network_handle;
	struct NETWORK_CHANNEL	*channel;

	struct NETWORK_ENTRY	*next;
};


void networkAdd(const char *name);
void networkHostSet(const char *name, const char *host);
void networkPortSet(const char *name, int port);
void networkNickSet(const char *name, const char *nick);
void networkLayerSet(const char *name, const char *layer);
void networkChannelAdd(const char *name, const char *channel, const char *key);
void networkInit();


#endif
