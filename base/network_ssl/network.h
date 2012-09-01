#ifndef __NETWORK_H__
#define	__NETWORK_H__


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <config/api.h>


typedef struct {
	BIO		*bio;
	SSL_CTX		*ctx;
	SSL		*ssl;
} NETWORK_SSL;


#endif
