#include "net.h"


char *getURLData(int sock, char *host) {
	char *buff, *temp, c;
	int i, cnt, len;
	
	c = i = cnt = 0;
	buff = malloc(1);
	buff[0] = 0;
	while (strstr(buff, "Content-Length:") == NULL) {
		c = 0;
		while (c != '\n') {
			if (recv(sock, &c, 1, 0) < 1) {
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
		recv(sock, &c, 1, 0);
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
			len += recv(sock, &buff[len], 200, 0);
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
		if ((i += recv(sock, &buff[i], len-i, 0)) < 1) {
			buff[len] = 0;
			return buff;
		}
	}
	
	buff[len] = 0;
	
	return buff;
}


int getPageFromURL(char *rurl, char *usrname, char *password, char **buff) {
	int sock, port;
	char *host, *bitbucket, *url, *foo;
	
	struct sockaddr_in address;
	struct hostent *hp;
	
	if (rurl == NULL) {
		return NET_ERROR_NO_SUCH_HOST;
	}
	
	if ((url = malloc(strlen(rurl) + 1)) == NULL) {
		fprintf(stderr, "Error: Out of RAM.\n");
		return NET_ERROR_GENERAL_LOCAL_ERROR;
	}
	
	sprintf(url, "%s", rurl);
	
	if (strstr(url, "http://") != url) {
		fprintf(stderr, "Error: unhandled protocol in URL: %s\n", url);
		free(url);
		return NET_ERROR_BAD_PROTOCOL;
	}
	
	url += 7;
	
	if (strstr(url, "/") != NULL) {
		bitbucket = strstr(url, "/");
		*bitbucket = '\0';
	} else
		bitbucket = NULL;
		
	if ((host = malloc(strlen(url) + 1)) == NULL) {
		url -= 7;
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
		port = 80;
	}
	
	if (bitbucket != NULL) 
		*bitbucket = '/';
	
	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ) {
		fprintf(stderr, "Error: Unable to create a socket.\n");
		free(host);
		url -= 7;
		free(url);
		return NET_ERROR_GENERAL_LOCAL_ERROR;
	}
	
	if ((hp = gethostbyname(host)) == NULL) {
		fprintf(stderr, "Failed to resolve host '%s'.\n", host);
		url -= 7;
		free(url);
		free(host);
		return NET_ERROR_NO_SUCH_HOST;
	}
	
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = *( u_long * ) hp->h_addr;
	
	if (connect(sock, (struct sockaddr *) &address, sizeof(struct sockaddr)) == -1) {
		fprintf(stderr, "Error: Failed to connect to host '%s'.\n", host);
		free(host);
		url -= 7;
		free(url);
		return NET_ERROR_CONN_FAILED;
	}
	
	send(sock, "GET ", 4, 0);
	
	if (bitbucket != NULL)
		send(sock, bitbucket, strlen(bitbucket), 0);
	else 
		send(sock, "/", 1, 0);
	
	send(sock, " HTTP/1.0\r\n", 11, 0);
	send(sock, NET_USER_AGENT, strlen(NET_USER_AGENT), 0);
	send(sock, "Host: ", 6, 0);
	send(sock, host, strlen(host), 0);
	send(sock, "\r\n", 2, 0);
	
	/*
	
	*** Borken, needs code to convert everything to base64 first. ***
	
	if (usrname != NULL && password != NULL) {
		printf("Sending auth...\n");
		send(sock, "Authorization: basic ", 20 , 0);
		send(sock, usrname, strlen(usrname), 0);
		send(sock, ":", 1, 0);
		send(sock, password, strlen(password), 0);
	}*/
	
	send(sock, "\r\n\r\n", 4, 0);
	
	*buff = getURLData(sock, host);
	url -= 7;
	free(url);
	free(host);
	close(sock);
	
	if (*buff == NULL)
		return NET_ERROR_NO_DATA_FROM_SERVER;
	
	return NET_NO_ERROR;
}

