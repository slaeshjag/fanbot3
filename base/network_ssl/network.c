#include "network.h"
#include <config/api.h>


unsigned int pluginType() {
	return PLUGIN_TYPE_NETWORK;
}


const char *pluginName() {
	return "ssl";
}


void *pluginConnect(const char *host, int port) {
	NETWORK_SSL *connection;

	if ((connection = malloc(sizeof(BIO))) == NULL) {
		configErrorPush("Unable to malloc");
		return NULL;
	}

	connection->ctx = SSL_CTX_new(SSLv23_client_method());

	if ((connection->bio = BIO_new_ssl_connect(connection->ctx)) == NULL) {
		configErrorPush("Unable to allocate a BIO");
		free(connection);
		return NULL;
	}

	BIO_get_ssl(connection->bio, &connection->ssl);
	SSL_set_mode(connection->ssl, SSL_MODE_AUTO_RETRY);
	BIO_set_conn_hostname(connection->bio, host);
	BIO_set_conn_int_port(connection->bio, &port);

	if (BIO_do_connect(connection->bio) <= 0) {
		configErrorPush("Unable to connect");
		free(connection);
		return NULL;
	}

	return connection;
}


int pluginSocketGet(NETWORK_SSL *connection) {
	return BIO_get_fd(connection->bio, NULL);
}


int pluginReadData(NETWORK_SSL *connection, char *buffer, int buffer_len) {
	int ret;

	if ((ret = BIO_read(connection->bio, buffer, buffer_len)) == 0)
		return ECONNRESET;
	else if (ret == -1)
		return (!BIO_should_retry(connection->bio)) ? ECONNRESET : EWOULDBLOCK;
	return ret;
}


int pluginSendData(NETWORK_SSL *connection, const char *buffer, int buffer_len) {
	int ret;
	if ((ret = BIO_write(connection->bio, buffer, buffer_len)) <= 0) {
		if (!BIO_should_retry(connection->bio))
			return ECONNRESET;
		return EWOULDBLOCK;
	}

	return ret;
}


void *pluginSocketDone(NETWORK_SSL *connection) {
	BIO_free_all(connection->bio);
	SSL_CTX_free(connection->ctx);
	
	free(connection);

	return NULL;
}
