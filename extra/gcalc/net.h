#ifndef _NET_H_
#define _NET_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>


#define NET_NO_ERROR 0
#define NET_ERROR_BAD_PROTOCOL -1
#define NET_ERROR_NO_SUCH_HOST -2
#define NET_ERROR_CONN_FAILED -3
#define NET_ERROR_NO_DATA_FROM_SERVER -4
#define NET_ERROR_GENERAL_LOCAL_ERROR -255

#define NET_USER_AGENT "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:24.0) Gecko/20100101 Firefox/24.0\r\n"



int getPageFromURL(char *rurl, char *usrname, char *password, char **buff);

#endif
