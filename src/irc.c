#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>

#include "poem.h"
#include "irc.h"
#include "types.h"
#include "stringutil.h"

static irc_session_t *irc_session;
static irc_settings_t irc_settings;

static int irc_settings_exist = 0;

extern int running;

static void event_connect_callback(irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count) {

	fprintf(stderr, "event: %s, origin: %s\n", event, origin);

	if (strcmp(event, "CONNECT") == 0) {
		for (int i = 0; i < irc_settings.num_channels; ++i) {
			const char* channel = irc_settings.channels[i];
			if (irc_cmd_join(session, channel, 0)) {
				fprintf(stderr, "irc_cmd_join for %s failed\n", channel);
			}
			fprintf(stderr, "irc_cmd_join for %s ok!\n", channel);
		}
	}

	fprintf(stderr, "irc_cmd_join ok\n");

}

static void send_multiline_message(const char *msg, const char* channel) {
	char *dup = strdup(msg);
	char *endptr;
	char *token = strtok_r(dup, " ", &endptr);
	while (token) {
		irc_cmd_msg(irc_session, channel, token);
		token = strtok_r(NULL, "\n", &endptr);
	}
}

static void send_poem_to_channel(const poem_t *poem, const char* channel) {
	char *converted = convert_to_multibyte(poem->title, wcslen(poem->title));
	irc_cmd_msg(irc_session, channel, converted);
	irc_cmd_msg(irc_session, channel, " ");
	free(converted);

	for (int i = 0; i < poem->num_stanzas; ++i) {
		converted = convert_to_multibyte(poem->stanzas[i], wcslen(poem->stanzas[i]));
		send_multiline_message(converted, channel);
		irc_cmd_msg(irc_session, channel, " ");
		free(converted);
	}
}

static void event_channel_callback(irc_session_t *session, const char *event, const char *origin, const char **params, unsigned int count) {

	fprintf(stderr, "event: %s, channel: %s: message: %s\n", event, params[0], params[1]);

	if (strstr(params[1], "!poem") == params[1]) {
		kstate_t s;
		memset(&s, 0, sizeof(s));
		s.rules_apply = 1;
		poem_t poem = generate_poem(&s);

		send_poem_to_channel(&poem, params[0]);

		poem_free(&poem);
	}
	else if (strstr(params[1], "!boem") == params[1]) {
		kstate_t s;
		memset(&s, 0, sizeof(s));
		s.rules_apply = 0;

		poem_t poem = generate_poem(&s);

		send_poem_to_channel(&poem, params[0]);

		poem_free(&poem);
	}

}
static void event_numeric_callback(irc_session_t *session, unsigned int event, const char *origin, const char **params, unsigned int count) {
//	fprintf(stderr, "event: %u, origin: %s\n", event, origin);

}


int irc_connection_setup(const char* servaddr, const char* botnick, const char* username, const char* realname, char* const* channels, int num_channels) {

	irc_settings.servaddr = strdup(servaddr);
	irc_settings.botnick = strdup(botnick);
	irc_settings.username = strdup(username);
	irc_settings.realname = strdup(realname);
	irc_settings.channels = malloc(num_channels * sizeof(const char*));

	for (int i = 0; i < num_channels; ++i) {
		irc_settings.channels[i] = strdup(channels[i]);
	}

	irc_settings.num_channels = num_channels;
	irc_settings_exist = 1;

	return 1;
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

	if (irc_connect(irc_session, irc_settings.servaddr, 6667, NULL, irc_settings.botnick, irc_settings.username, irc_settings.realname)) {
		fprintf(stderr, "irc_connect failed!\n");
		return 0;
	}

	fprintf(stderr, "irc_connect ok\n");

	if (irc_run(irc_session)) {
		fprintf(stderr, "irc_run failed!\n");
		running = 0;
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


