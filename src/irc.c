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



typedef struct connection_settings_t {
	const char* servaddr;
	const char* botname;
	const char* realname;
	const char** channels;
	int num_channels;

} connection_settings_t;

static irc_session_t *irc_session;
static connection_settings_t connection_settings;

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

pthread_t start_irc_thread(const char* servaddr, const char *botname, const char* realname) {

	connection_settings.servaddr = servaddr;
	connection_settings.botname = botname;
	connection_settings.realname = realname;

	pthread_t thread_id;
	int e = pthread_create(&thread_id, NULL, &run_irc, NULL);
	if (e != 0) {
		fprintf(stderr, "creating thread for run_irc failed (pthread_create)\n");
		return 0;
	}

	return thread_id;

}


