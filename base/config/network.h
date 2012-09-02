#ifndef __NETWORK_H__
#define	__NETWORK_H__

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
void networkInit();


#endif
