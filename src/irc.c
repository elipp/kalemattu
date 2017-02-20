#include "irc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void event_connect_callback(irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count) {

	fprintf(stderr, "event: %s, origin: %s\n", event, origin);

	if (strcmp(event, "CONNECT") == 0) {
		if (irc_cmd_join(session, "#dumuIItest", 0)) {
			fprintf(stderr, "irc_cmd_join for #dumuIItest failed\n");
		}
	}

	fprintf(stderr, "irc_cmd_join ok\n");

}

static void event_channel_callback(irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count) {

	fprintf(stderr, "event: %s, channel: %s: message: %s\n", event, params[0], params[1]);

}
static void event_numeric_callback(irc_session_t *session, unsigned int event, const char *origin, const char **params, unsigned int count) {
//	fprintf(stderr, "event: %u, origin: %s\n", event, origin);

}

static irc_session_t *irc_session;
static irc_settings_t irc_settings;

static int irc_settings_exist = 0;

int irc_connection_setup(const char* servaddr, const char* botname, const char* username, const char* realname, const char** channels, int num_channels) {
	irc_settings.servaddr = strdup(servaddr);
	irc_settings.botname = strdup(botname);
	irc_settings.username= strdup(username);
	irc_settings.realname = strdup(realname);
	irc_settings.channels = malloc(num_channels * sizeof(const char*));
	for (int i = 0; i < num_channels; ++i) {
		irc_settings.channels[i] = strdup(channels[i]);
	}

	irc_settings_exist = 1;
}

static void *run_irc(void *arg) {

	irc_callbacks_t callbacks;

	memset(&callbacks, 0, sizeof(callbacks));

	callbacks.event_connect = &event_connect_callback;
	callbacks.event_channel = &event_channel_callback;
	callbacks.event_numeric = &event_numeric_callback;

	irc_session = irc_create_session(&callbacks);

	if (!irc_session) {
		fprintf(stderr, "connect_to_irc_server: irc_create_session failed!\n");
		return NULL;
	}

	fprintf(stderr, "irc_create_session ok\n");

	irc_option_set(irc_session, LIBIRC_OPTION_STRIPNICKS);

	if (irc_connect(irc_session, connection_settings.servaddr, 6667, NULL, connection_settings.botname, "dasd", connection_settings.realname)) {
		fprintf(stderr, "irc_connect failed!\n");
		return 0;
	}

	fprintf(stderr, "irc_connect ok\n");

	connection_settings_t *settings = (connection_settings_t*)arg;

	if (irc_run(irc_session)) {
		fprintf(stderr, "irc_run failed!\n");
		return NULL;
	}

	fprintf(stderr, "irc_run ok\n");

	return arg;
}

pthread_t start_irc_thread() {

	if (!irc_settings_exist) { 
		fprintf(stderr, "start_irc_thread(): irc settings haven't been set up!\n");
		return 0; 
	}

	pthread_t thread_id;
	int e = pthread_create(&thread_id, NULL, &run_irc, NULL);
	if (e != 0) {
		fprintf(stderr, "creating thread for run_irc failed (pthread_create)\n");
		return 0;
	}

	return thread_id;

}


