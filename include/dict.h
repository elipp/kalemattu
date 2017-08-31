#ifndef DICT_H
#define DICT_H

#include "types.h"

int read_file_to_words(const char* filename); 
int add_words_to_dictionary(const word_t *words, size_t num_words); //nyt

const word_t *dict_get_random_word();
syl_t dict_get_random_syllable_any(bool ignore_last);

syl_t syl_create(const wchar_t *w, int length_class);
void syl_free(syl_t *s);

sylvec_t sylvec_create(); 
int sylvec_contains(sylvec_t *s, const wchar_t *str);
int sylvec_pushsyl(sylvec_t *s, const syl_t *syl);
int sylvec_pushstr(sylvec_t *s, const wchar_t *syl); 
void sylvec_destroy(sylvec_t *s);

const dict_t *get_dictionary();

#endif
