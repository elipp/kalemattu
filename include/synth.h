#ifndef SYNTH_H
#define SYNTH_H

#include "types.h"
#include "stringutil.h"

enum {
	SYNTH_ANY,
	SYNTH_VOWEL,
	SYNTH_CONSONANT
};

syl_t synth_get_syllable(sylsrc_args_t *args);
word_t newsynth_get_word(int num_syllables);

const char *synth_get_sylp(int num_syllables);

#endif
