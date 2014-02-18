#include "net.h"


char *getURLData(BIO *bio, char *host) {
	char *buff, *temp, c;
	int i, cnt, len;
	
	c = i = cnt = 0;
	buff = malloc(1);
	buff[0] = 0;
	while (strstr(buff, "Content-Length:") == NULL) {
		c = 0;
		while (c != '\n') {
			if (BIO_read(bio, &c, 1) < 1) {
				printf("Dying...\n");
				free(buff);
				return NULL;
			}
			
			if (c != '\r') {
				i++;
				if ((temp = realloc(buff, i)) == NULL) {
					fprintf(stderr, "Error: Out of RAM.\n");
					free(buff);
					return NULL;
				}
				
				buff = temp;
				temp = NULL;
			}
			
			
			if (c != '\n' && c != '\r') {
				buff[i-1] = c;
				cnt = 0;
			} else if (c != '\r') {
				buff[i-1] = '\n';
				cnt++;
			}
			
			if (cnt == 2)
				break;
		}
		
		if ((temp = realloc(buff, i+1)) == NULL) {
			fprintf(stderr, "Error: Out of RAM.\n");
			free(buff);
			return NULL;
		}
		
		buff = temp;
		temp = NULL;
		buff[i] = '\0';
		
		if (cnt == 2)
			break;
	}
	temp = strstr(buff, "Content-Length: ");
	len = 0;
	
	if (temp != NULL)
		sscanf(temp, "Content-Length: %i", &len);
	
	while (cnt != 2) {
		if (BIO_read(bio, &c, 1) < 1) {
			free(buff);
			return NULL;
		}
		if (c == '\n')
			cnt++;
		else if (c != '\r')
			cnt = 0;
	}
	
	if (len == 0) {
		cnt = 0;
		i = 0;
		
		while (1) {
			if ((temp = realloc(buff, len+201)) == NULL) {
				fprintf(stderr, "Error: Out of RAM.\n");
				free(buff);
				return NULL;
			}
			
			buff = temp;
			temp = NULL;
			i = len;
			len += BIO_read(bio, &buff[len], 200);
			if (len-i == 0) {
				buff[len] = 0;
				return buff;
			}
		}
	}
	
	if ((temp = realloc(buff, len+1)) == NULL) {
		fprintf(stderr, "Error: Out of RAM.\n");
		free(buff);
		return NULL;
	}
	
	buff = temp;
	temp = NULL;
	for (i = 0; i < len;) {
		if ((i += BIO_read(bio, &buff[i], len-i)) < 1) {
			buff[len] = 0;
			return buff;
		}
	}
	
	buff[len] = 0;
	
	return buff;
}


int getPageFromURL(char *rurl, char *usrname, char *password, char **buff) {
	int port;
	char *host, *bitbucket, *url, *foo;
	BIO *bio;
	SSL_CTX *ctx;
	SSL *ssl;
	
	if (rurl == NULL) {
		return NET_ERROR_NO_SUCH_HOST;
	}
	
	if ((url = malloc(strlen(rurl) + 1)) == NULL) {
		fprintf(stderr, "Error: Out of RAM.\n");
		return NET_ERROR_GENERAL_LOCAL_ERROR;
	}
	
	sprintf(url, "%s", rurl);
	
	if (strstr(url, "https://") != url) {
		fprintf(stderr, "Error: unhandled protocol in URL: %s\n", url);
		free(url);
		return NET_ERROR_BAD_PROTOCOL;
	}
	
	url += 8;
	
	if (strstr(url, "/") != NULL) {
		bitbucket = strstr(url, "/");
		*bitbucket = '\0';
	} else
		bitbucket = NULL;
		
	if ((host = malloc(strlen(url) + 1)) == NULL) {
		url -= 8;
		free(url);
		fprintf(stderr, "Error: Out of RAM.\n");
		return NET_ERROR_GENERAL_LOCAL_ERROR;
	}
		
	sprintf(host, "%s", url);
	if ((foo = strstr(host, ":")) != NULL) {
		*foo = 0;
	}
	
	if (strstr(url, ":") != NULL) {
		port = atoi(1 + strstr(url, ":"));
	} else {
		port = 443;
	}
	
	if (bitbucket != NULL) 
		*bitbucket = '/';
	
	ctx = SSL_CTX_new(SSLv23_client_method());
	if (!(bio = BIO_new_ssl_connect(ctx))) {
		fprintf(stderr, "Unable to allocate a BIO\n");
		return NET_ERROR_GENERAL_LOCAL_ERROR;
	}

	BIO_get_ssl(bio, &ssl);
	SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);
	BIO_set_conn_hostname(bio, host);
	BIO_set_conn_int_port(bio, &port);

	if (BIO_do_connect(bio) < 0)
		return NET_ERROR_NO_SUCH_HOST;
	
	BIO_write(bio, "GET ", 4);
	fprintf(stderr, "Contacting %s port %i url %s\n", host, port, bitbucket);
	
	if (bitbucket != NULL)
		BIO_write(bio, bitbucket, strlen(bitbucket));
	else 
		BIO_write(bio, "/", 1);
	
	BIO_write(bio, " HTTP/1.0\r\n", 11);
	BIO_write(bio, NET_USER_AGENT, strlen(NET_USER_AGENT));
	BIO_write(bio, "Host: ", 6);
	BIO_write(bio, host, strlen(host));
	BIO_write(bio, "\r\n", 2);
	
	BIO_write(bio, "\r\n\r\n", 4);
	
	*buff = getURLData(bio, host);
	url -= 8;
	free(url);
	free(host);
	BIO_free_all(bio);
	SSL_CTX_free(ctx);
	
	if (*buff == NULL)
		return NET_ERROR_NO_DATA_FROM_SERVER;
	
	return NET_NO_ERROR;
}

