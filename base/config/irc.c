#include "config.h"


void ircMessage(const char *channel, const char *message) {
	struct NETWORK_ENTRY *network;
	char sendbuff[512];
	char message_copy[512];
	int message_len, error;

	if ((network = networkFind(config->net.network_active)) == NULL)
		return;

	message_len = 512 - 3; /* \r\n + NULL-terminator */
	message_len -= (strlen("PRIVMSG ") + 1); /* extra char for space after channel */
	message_len -= (strlen(channel) + 1); /* Extra char for message ':' */
	
	if (strlen(message) > message_len) {
		memcpy(message_copy, message, message_len);
		message_copy[message_len] = 0;
	} else
		sprintf(message_copy, "%s", message);
	
	sprintf(sendbuff, "PRIVMSG %s :%s\r\n", channel, message_copy);
	layerWrite(network->layer, network->network_handle, sendbuff, strlen(sendbuff), &error);

	return;
}


void ircPong(const char *msg) {
	struct NETWORK_ENTRY *network;
	char sendbuff[512];
	int error;

	if ((network = networkFind(config->net.network_active)) == NULL)
		return;
	
	sprintf(sendbuff, "PONG %s\r\n", msg);
	layerWrite(network->layer, network->network_handle, sendbuff, strlen(sendbuff), &error);

	return;
}


void ircQuit(const char *msg) {
	struct NETWORK_ENTRY *network;
	char sendbuff[512];
	int error;

	if ((network = networkFind(config->net.network_active)) == NULL)
		return;

	sprintf(sendbuff, "QUIT :%s\r\n", msg);
	layerWrite(network->layer, network->network_handle, sendbuff, strlen(sendbuff), &error);

	return;
}


void ircNick(const char *nick) {
	struct NETWORK_ENTRY *network;
	char sendbuff[512];
	int error;

	if ((network = networkFind(config->net.network_active)) == NULL)
		return;
	
	sprintf(sendbuff, "USER %s %s %s :Fanbot³\r\n", nick, nick, nick);
	layerWrite(network->layer, network->network_handle, sendbuff, strlen(sendbuff), &error);
	sprintf(sendbuff, "NICK %s\r\n", nick);
	layerWrite(network->layer, network->network_handle, sendbuff, strlen(sendbuff), &error);

	return;
}


void ircJoin(const char *channel, const char *key) {
	struct NETWORK_ENTRY *network;
	char sendbuff[512];
	int error;

	if ((network = networkFind(config->net.network_active)) == NULL)
		return;
	sprintf(sendbuff, "JOIN %s %s\r\n", channel, key);
	layerWrite(network->layer, network->network_handle, sendbuff, strlen(sendbuff), &error);

	return;
}


void ircLine() {
	struct NETWORK_ENTRY *network;
	char *hoststr, *nick, *command, *arg, *string;


	if ((network = networkFind(config->net.network_active)) == NULL)
		return;

	if (*network->active_buffer == ':') {		/* Second argument is likely a command */
		nick = network->active_buffer + 1;
		if ((hoststr = strstr(nick, "!")) == NULL)	/* This should never happen, unless it's not importment */
			return;
		*hoststr = 0;
		hoststr++;
	
		if ((command = strstr(hoststr, " ")) == NULL)	/* Eek, bad line */
			return;
		*command = 0;
		command++;

		if ((arg = strstr(command, " ")) == NULL) 	/* Hm. No argument? */
			arg = NULL;
		else {
			*arg = 0;
			arg++;
		}

		if (arg != NULL) {
			if ((string = strstr(arg, " :")) == NULL)	/* No string. This is fine */
				string = NULL;
			else {
				*string = 0;
				string += 2;
			}
		} else
			string = NULL;

		if (string != NULL) {
			if (string[strlen(string) - 1] == ' ')
				string[strlen(string) - 1] = 0;
		}
	
		filterProcess(nick, hoststr, command, arg, string);
	} else if (strstr(network->active_buffer, "PING ") == network->active_buffer) {		/* Aha! It's a ping! */
		if ((arg = strstr(network->active_buffer, ":")) == NULL)	/* o_O */
			return;
		if (arg[strlen(arg) - 1] == ' ')
			arg[strlen(arg) - 1] = 0;
		ircPong(arg);
	} else {
		configErrorPush("Following line was received, but appears to be invalid");
		configErrorPush(network->active_buffer);
		return;
	}

	return;
}
