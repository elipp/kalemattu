#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "fcgi.h"
#include "types.h"
#include "poem.h"

#include <time.h>


static char *get_timestring() {

	static char timebuf[128];
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	sprintf(timebuf, "%02d.%02d.%d/%02d:%02d:%02d", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

	return timebuf;

}


extern int running;

static int num_poems = 0;
static int num_boems = 0;
static int num_noems = 0;
static int num_moems = 0;
static int num_woems = 0;

static void return_poem(FCGX_Request *r, kstate_t *state) {
	FCGX_FPrintF(r->out, "Content-Type: text/html; charset=utf-8\r\n\r\n");

	poem_t poem = generate_poem(state);
	int poemlen_bytes;
	char* p = poem_print_to_fcgi_buffer(&poem, &poemlen_bytes);
	FCGX_PutStr(p, poemlen_bytes, r->out);
	poem_free(&poem);
	free(p);

}

static char *titlepage;

static int read_titlepage() {
	FILE *fp = fopen("index.html", "r");

	if (!fp) {
		fprintf(stderr, "read_titlepage: failed to read index.html! X(\n");
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	rewind(fp);

	titlepage = malloc(size + 1);
	fread(titlepage, 1, size, fp);

	titlepage[size] = '\0';

	fclose(fp);

	return size;
}
static void return_titlepage(FCGX_Request *r) {

	FCGX_FPrintF(r->out, "Content-Type: text/html; charset=utf-8\r\n\r\n");
	FCGX_FPrintF(r->out, titlepage);

}

enum {
	REQUEST_UNKNOWN = 0x0,

	REQUEST_POEM = 1,
	REQUEST_BOEM = 3,
	REQUEST_NOEM = 5,
	REQUEST_MOEM = 7,

	REQUEST_WOEM = 9,

	REQUEST_TITLE_PAGE = 2,
	REQUEST_FAVICON = 4,

	REQUEST_IRRELEVANT = 0xf0,
};

static int handle_fcgi_request(FCGX_Request *r, kstate_t *state) {
        char *value;

	if ((value = FCGX_GetParam("REQUEST_URI", r->envp)) != NULL) {
		if (strcmp(value, "/") == 0) {
			//FCGX_FPrintF(r->out, "\n");
			return_titlepage(r);
			return REQUEST_TITLE_PAGE;
		}
		else if (strcmp(value, "/favicon.ico") == 0) {
			FCGX_FPrintF(r->out, "\n");
			return REQUEST_FAVICON;
		}
		else if (strcmp(value, "/p") == 0) {
			state->synth_enabled = 0;
			state->newsynth_enabled = 0;
			state->rules_apply = 1;
			return_poem(r, state);

			++num_poems;
			return REQUEST_POEM;
		}
		else if (strcmp(value, "/n") == 0) {
			state->synth_enabled = 1;
			state->newsynth_enabled = 0;
			state->rules_apply = 1;
			return_poem(r, state);

			++num_noems;
			return REQUEST_NOEM;
		}
		else if (strcmp(value, "/b") == 0) {
			state->synth_enabled = 0;
			state->newsynth_enabled = 0;
			state->rules_apply = 0;
			return_poem(r, state);

			++num_boems;
			return REQUEST_BOEM;
		}
		else if (strcmp(value, "/m") == 0) {
			state->synth_enabled = 1;
			state->newsynth_enabled = 0;
			state->rules_apply = 0;
			return_poem(r, state);

			++num_moems;
			return REQUEST_MOEM;
		}
		else if (strcmp(value, "/w") == 0) {
			state->synth_enabled = 0;
			state->newsynth_enabled = 1;
			state->rules_apply = 0;
			return_poem(r, state);

			++num_woems;
			return REQUEST_WOEM;

		}

		else {
			return REQUEST_UNKNOWN;
		}
	}


	 return REQUEST_IRRELEVANT;

//        if ((value = FCGX_GetParam("REQUEST_METHOD", request->envp)) != NULL) {
//                FCGX_FPrintF(request->out, "%s ", value);
//        }
//               if ((value = FCGX_GetParam("QUERY_STRING", request->envp)) != NULL) {
//                FCGX_FPrintF(request->out, "?%s", value);
//        }
//        if ((value = FCGX_GetParam("SERVER_PROTOCOL", request->envp)) != NULL) {
//                FCGX_FPrintF(request->out, " %s", value);
//        }
}

static void *run_fcgi(void *arg) {


	kstate_t *t = (kstate_t*)arg;

	int sock;

	if (FCGX_Init() != 0) { 
		fprintf(stderr, "FCGX_Init failed!\n");
		return 0;
	}

	sock = FCGX_OpenSocket(t->fcgi_addr, 5);

	if (sock == -1) {
		fprintf(stderr, "FCGX_OpenSocket failed %d\n", errno);
		return 0;
	}

	if (!read_titlepage()) {
		return 0;
	}

	FCGX_Request request;
	FCGX_InitRequest(&request, sock, 0);

	while (FCGX_Accept_r(&request) >= 0 && running) {

		int r = handle_fcgi_request(&request, t);
		FCGX_Finish_r(&request);

		if (r & REQUEST_POEM) {
			fprintf(stderr, "[%s] p/b/n/m = %d/%d/%d/%d = %d\n", get_timestring(), num_poems, num_boems, num_noems, num_moems, num_poems + num_boems + num_noems + num_moems);
		}

	}

	return arg;
}

int start_fcgi_thread(struct kstate_t *state, pthread_t *t) {

	int e = pthread_create(t, NULL, &run_fcgi, state);
	if (e != 0) {
		fprintf(stderr, "creating thread for run_irc failed (pthread_create)\n");
		return 0;
	}

	fprintf(stderr, "Successfully started fastcgi app on address %s!\n", state->fcgi_addr);

	return 1;

}
