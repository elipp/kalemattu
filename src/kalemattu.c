#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <locale.h>
#include <wctype.h>
#include <wchar.h>
#include <unistd.h>
#include <pthread.h>

#include <libircclient/libircclient.h>

#include "aesthetics.h"
#include "distributions.h"
#include "irc.h"
#include "types.h"
#include "dict.h"
#include "poem.h"

static unsigned long hash(unsigned char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}


static int get_commandline_options(int argc, char **argv, kstate_t *state) {
	char *endptr;

	opterr = 0;
	int c;
	while ((c = getopt (argc, argv, "cls:n:i:")) != -1) {
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
					fprintf(stderr, "(option -n: strtol: warning: the argument string \"%s\" couldn't be fully converted!)\n", optarg); 
				}
				break;
			}
			case 's':
				state->numeric_seed = hash((unsigned char*)optarg);
				break;
			case 'i':
				state->irc_enabled = 1;
				state->irc_channel = strdup(optarg);
				break;
			case '?':
				if (optopt == 'n') {
					fprintf(stderr, "error: the numeric seed option (-n) requires a (base-10) numeric argument.\n");
				}
				else if (optopt == 'i') {
					fprintf(stderr, "error: the irc option (-i) requires a channel as argument.\n");
				}
				else if (optopt == 's') {
					fprintf(stderr, "error: the seed option (-s) requires a string as argument.\n");
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

	return 1;

}

kstate_t get_default_state() {
	kstate_t defaults;

	defaults.numeric_seed = 0;
	defaults.LaTeX_output = 0;
	defaults.rules_apply = 1;
	defaults.irc_enabled = 0;
	defaults.irc_channel = NULL;

	return defaults;
}

static int running = 1;

int main(int argc, char *argv[]) {

	setlocale(LC_ALL, ""); // for whatever reason, this is needed. using fi_FI.UTF-8 doesn't work

	dict_t dict = read_file_to_words("kalevala.txt");

	if (dict.num_words < 1) {
		fprintf(stderr, "kalemattu: fatal: couldn't open input file kalevala.txt, aborting!\n");
		return 1;
	}

	kstate_t state = get_default_state();
	get_commandline_options(argc, argv, &state);

	unsigned int seed = state.numeric_seed != 0 ? state.numeric_seed : time(NULL);
	srand(seed);

	fprintf(stderr, "(info: using %u as random seed)\n\n", seed);

	wchar_t *poem = generate_poem(&dict, &state);
	printf("\n%ls\n", poem);
	free(poem);

	pthread_t thread_id;
	if (state.irc_enabled) {
		running = 1;
		thread_id = start_irc_thread();
		fprintf(stderr, "irc thread id: 0x%lX\n", thread_id);
	}

	while (running) {
		usleep(1000000);
	}

	return 0;
}
