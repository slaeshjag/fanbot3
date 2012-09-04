#ifndef __LAYER_H__
#define	__LAYER_H__

void *layerConnect(const char *layer, const char *host, int port);
int layerSocketGet(const char *layer, void *handle);
int layerRead(const char *layer, void *handle, char *buffer, int buffer_len, int *error);
int layerWrite(const char *layer, void *handle, const char *buffer, int buffer_len, int *error);
void *layerDisconnect(const char *layer, void *handle);


#endif
