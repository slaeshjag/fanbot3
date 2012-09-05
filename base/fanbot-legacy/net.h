#ifndef _NET_H_
#define _NET_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>


#define NET_NO_ERROR 0
#define NET_ERROR_BAD_PROTOCOL -1
#define NET_ERROR_NO_SUCH_HOST -2
#define NET_ERROR_CONN_FAILED -3
#define NET_ERROR_NO_DATA_FROM_SERVER -4
#define NET_ERROR_GENERAL_LOCAL_ERROR -255

#define NET_USER_AGENT "User-Agent: Mozilla/5.0 Fanbot³ Legacy wrapper\r\n"



int getPageFromURL(char *rurl, char *usrname, char *password, char **buff);

#endif
