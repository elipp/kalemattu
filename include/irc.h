#ifndef IRC_H
#define IRC_H

#include <pthread.h>
#include <libircclient/libircclient.h>

typedef struct irc_settings_t {
	const char* servaddr;
	const char* botnick;
	const char* username;
	const char* realname;
	const char** channels;
	int num_channels;

} irc_settings_t;

int irc_connection_setup(const char* servaddr, const char* botnick, const char* username, const char* realname, char* const* channels, int num_channels);
pthread_t start_irc_thread();

#endif
