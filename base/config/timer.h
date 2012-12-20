#ifndef __TIMER_H__
#define	__TIMER_H__


struct TIMER_ENTRY {
	time_t			when;
	int			id;
	const char		*plugin;
	struct TIMER_ENTRY	*next;
};

typedef struct {
	time_t			last_ping;
} TIMER;

void timerInit();
int timerAdd(time_t when, const char *plugin);
void timerDelete(int id);
void timerDeleteAll();
void timerProcess();

#endif
