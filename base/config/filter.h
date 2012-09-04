#ifndef __FILTER_H__
#define	__FILTER_H__

#ifdef __CONFIG_H__
struct PLUGIN_FILTER_ENTRY *filterFind(const char *name);
#endif
void *filterInit(const char *name, const char *network);
void filterDo(const char *name, void *handle, const char *from, const char *host, const char *channel, const char *message);
void *filterDestroy(const char *name, void *handle);


#endif
