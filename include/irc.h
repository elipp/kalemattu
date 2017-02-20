#ifndef IRC_H
#define IRC_H

#include <pthread.h>
#include <libircclient/libircclient.h>

pthread_t start_irc_thread(const char* servaddr, const char *botname, const char* realname);

#endif
