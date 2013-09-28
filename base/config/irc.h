#ifndef __IRC_H__
#define	__IRC_H__

#define	IRC_NICK_IN_USE			"433"
#define	IRC_NICK_NOT_REGISTERED		"451"


void ircMessage(const char *channel, const char *message);
void ircPong(const char *msg);
void ircPing(const char *msg);
void ircQuit(const char *msg);
void ircNick(const char *nick);
void ircJoin(const char *channel, const char *key);
void ircRaw(const char *raw);
void ircIdentify(const char *who, const char *key);
void ircLine();

#endif
