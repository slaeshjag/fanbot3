#include "config.h"


void *init() {
	if ((config = malloc(sizeof(CONFIG))) == NULL) {
		fprintf(stderr, "[CONFIG] First malloc failed. Let's just give up :P\n");
		return NULL;
	}

	
