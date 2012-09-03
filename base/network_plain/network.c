#include "network.h"
#include <config/api.h>


unsigned int pluginType() {
	return PLUGIN_TYPE_NETWORK;
}


const char *pluginName() {
	return "plain";
}


void *pluginConnect(const char *host, int port) {
	NETWORK_PLAIN *connection;
	struct sockaddr_in address;
	struct hostent *hp;

	if ((connection = malloc(sizeof(NETWORK_PLAIN))) == NULL) {
		configErrorPush("Unable to malloc");
		return NULL;
	}

	if ((hp = gethostbyname(host)) == NULL) {
		configErrorPush("Unable to resolve hostname");
		free(connection);
		return NULL;
	}

	if ((connection->socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		configErrorPush("Unable to create a socket");
		free(connection);
		return NULL;
	}

	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = *(u_long *) hp->h_addr;
	
	if (connect(connection->socket, (struct sockaddr *) &address, sizeof(struct sockaddr)) == -1) {
		configErrorPush("Unable to establish a connection");
		free(connection);
		return NULL;
	}

	/* TODO: Fixa non-blocking */

	return connection;
}


int pluginSocketGet(NETWORK_PLAIN *connection) {
	return connection->socket;
}


int pluginReadData(NETWORK_PLAIN *connection, char *buffer, int buffer_len, int *error) {
	int ret;

	errno = 0;
	if ((ret = recv(connection->socket, buffer, buffer_len, 0)) == -1)
		*error = errno;
	return ret;
}


int pluginSendData(NETWORK_PLAIN *connection, char *buffer, int buffer_len, int *error) {
	int ret;

	errno = 0;
	if ((ret = send(connection->socket, buffer, buffer_len, 0)) == -1)
		*error = errno;
	return ret;
}


void *pluginSocketDone(NETWORK_PLAIN *connection) {
	close(connection->socket);
	free(connection);
	return NULL;
}
