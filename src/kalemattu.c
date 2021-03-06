#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include "aesthetics.h"
#include "distributions.h"
#include "irc.h"
#include "types.h"
#include "dict.h"
#include "poem.h"
#include "stringutil.h"
#include "fcgi.h"

#include <libircclient/libircclient.h>


static unsigned long hash(unsigned char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

static int validate_irc_options(kstate_t *state) {

	int err = 0;

	if (state->irc_enabled && !state->irc_nick) {
		fprintf(stderr, "(warning: IRC option (-i) supplied but no nick with -N specified; defaulting to kalemattu)\n");
		state->irc_nick = strdup("kalemattu");
	}
	
	if (state->irc_nick && !state->irc_enabled) {
		fprintf(stderr, "error: IRC nick option (-N) supplied but IRC daemon mode (-i) not enabled!\n");
		err = 1;
	}

	return !err;

}

static int get_commandline_options(int argc, char **argv, kstate_t *state) {
	char *endptr;

	opterr = 0;
	int c;
	while ((c = getopt (argc, argv, "clSws:n:i:N:f:")) != -1) {
		switch (c)
		{
			case 'c':
				state->rules_apply = 0;
				break;
			case 'l':
				state->LaTeX_output = 1;
				break;
			case 'n': {
				size_t len = strlen(optarg);
				state->numeric_seed = strtol(optarg, &endptr, 10);
				if (endptr < optarg+len) {
					fprintf(stderr, "(option -n: strtol: warning: the argument string \"%s\" couldn't be fully converted to long!)\n", optarg); 
				}
				break;
			}
			case 'S':
				 state->synth_enabled = 1;
				 break;
			case 's':
				state->numeric_seed = hash((unsigned char*)optarg);
				break;
			case 'i':
				state->irc_enabled = 1;
				state->irc_channels = tokenize(optarg, ",", &state->num_irc_channels);
				break;
			case 'N':
				state->irc_nick = strdup(optarg);
				break;
			case 'f':
				state->fcgi_enabled = 1;
				state->fcgi_addr = strdup(optarg);
				break;
			case 'w':
				state->newsynth_enabled = 1;
				break;
			case '?':
				if (optopt == 'n') {
					fprintf(stderr, "error: the numeric seed option (-n) requires a (base-10) numeric argument.\n");
				}
				else if (optopt == 'N') {
					fprintf(stderr, "error: the IRC nick option (-N) requires a string as argument.\n");
				}
				else if (optopt == 'i') {
					fprintf(stderr, "error: the IRC option (-i) requires a channel as argument.\n");
				}
				else if (optopt == 's') {
					fprintf(stderr, "error: the seed option (-s) requires a string as argument.\n");
				}
				else if (optopt == 'f') {
					fprintf(stderr, "error: the fastcgi-mode option (-f) requires a bind address as argument (e.g. :2005)\n");
				}
				else if (isprint (optopt)) {
					fprintf(stderr, "error: unknown option `-%c'.\n", optopt);
				}
				else {
					fprintf(stderr, "error: unknown option character `\\x%x'.\n", optopt);
				}
				return 0;
			default:
				abort ();
		}
	}

	return validate_irc_options(state);

}

kstate_t get_default_state() {
	kstate_t defaults;

	defaults.numeric_seed = 0;
	defaults.LaTeX_output = 0;
	defaults.rules_apply = 1;
	defaults.irc_enabled = 0;
	defaults.irc_channels = NULL;
	defaults.irc_nick = NULL;
	defaults.num_irc_channels = 0;
	defaults.fcgi_enabled = 0;
	defaults.synth_enabled = 0;
	defaults.newsynth_enabled = 0;

	return defaults;
}

int running = 0;

int main(int argc, char *argv[]) {

	
	setlocale(LC_ALL, ""); // for whatever reason, this is needed. using fi_FI.UTF-8 doesn't work

	if (!read_file_to_words("/home/elias/kalemattu/kalevala.txt")) {
		fprintf(stderr, "kalemattu: fatal: couldn't open input file kalevala.txt, aborting!\n");
		return 1;
	}

	kstate_t state = get_default_state();

	if (!get_commandline_options(argc, argv, &state)) return 1;

	unsigned int seed = state.numeric_seed != 0 ? state.numeric_seed : time(NULL);
	state.numeric_seed = seed;
	srand(seed);

//	fprintf(stderr, "(info: using %u as random seed)\n\n", seed);

	running = 1;
	pthread_t irc_thread;
	pthread_t fcgi_thread;

	if (state.irc_enabled) {

		irc_connection_setup("open.ircnet.net", state.irc_nick, "gallentau", "Seka S. Tibetiel", state.irc_channels, state.num_irc_channels);
	       	if (!start_irc_thread(&irc_thread)) return 1;
	} 

	if (state.fcgi_enabled) {
		if (!start_fcgi_thread(&state, &fcgi_thread)) return 1;
	}

	if (!state.irc_enabled && !state.fcgi_enabled) { 
		poem_t poem = generate_poem(&state);
		poem_print(&poem, state.LaTeX_output ? POEM_FORMAT_LATEX : POEM_FORMAT_VANILLA);
		poem_free(&poem);
		running = 0;
	}

	while (running) {
		usleep(10000000);
	}

	return 0;
}
