#ifndef __NETWORK_H__
#define	__NETWORK_H__

#include <sys/select.h>
#include <unistd.h>
#include <fcntl.h>


#define	NETWORK_CHANNEL_SEND_CAP		5

#define	NETWORK_NOT_CONNECTED			0
#define	NETWORK_CONNECTING			1
#define	NETWORK_READY				2
#define	NETWORK_JOIN				3


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

	char			process_buffer[512];
	char			active_buffer[512];
	int			buff_pos;

	int			ready;
	int			socket;
	void			*network_handle;
	struct NETWORK_CHANNEL	*channel;

	struct NETWORK_ENTRY	*next;
};


typedef struct {
	fd_set			read;
	struct timeval		time;
} NETWORK_MAIN;


void networkAdd(const char *name);
void networkHostSet(const char *name, const char *host);
void networkPortSet(const char *name, int port);
void networkNickSet(const char *name, const char *nick);
void networkLayerSet(const char *name, const char *layer);
void networkChannelAdd(const char *name, const char *channel, const char *key);
void networkInit();


#endif
