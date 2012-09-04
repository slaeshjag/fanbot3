#ifndef __IRC_H__
#define	__IRC_H__

void ircMessage(const char *channel, const char *message);
void ircPong(const char *msg);
void ircQuit(const char *msg);
void ircNick(const char *nick);
void ircJoin(const char *channel, const char *key);
void ircLine();

#endif
