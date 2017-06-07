#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>

#include "fcgi.h"
#include "types.h"
#include "poem.h"

extern int running;

static void handle_fcgi_request(FCGX_Request *request) {
        char *value;
        FCGX_FPrintF(request->out, "Content-Type: text/plain; charset=utf-8\r\n\r\n");
        if ((value = get_param("REQUEST_METHOD")) != NULL) {
                FCGX_FPrintF(request->out, "%s ", value);
        }
        if ((value = get_param("REQUEST_URI")) != NULL) {
                FCGX_FPrintF(request->out, "%s", value);
        }
        if ((value = get_param("QUERY_STRING")) != NULL) {
                FCGX_FPrintF(request->out, "?%s", value);
        }
        if ((value = get_param("SERVER_PROTOCOL")) != NULL) {
                FCGX_FPrintF(request->out, " %s", value);
        }
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

	FCGX_Request request;

	FCGX_InitRequest(&request, sock, 0);
	while (FCGX_Accept_r(&request) >= 0 && running) {

		handle_fcgi_request(&request);
		poem_t poem = generate_poem(t);
		int poemlen_bytes;
		char* p = poem_print_to_buffer(&poem, &poemlen_bytes);
//		if (printf("%ls\n", p) == -1) {
//			fprintf(stderr, "FUUCK\n");
//		}
		FCGX_PutStr(p, poemlen_bytes, request.out);
		//fprintf(stderr, "%ls\n", p);
		poem_free(&poem);
		free(p);


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
