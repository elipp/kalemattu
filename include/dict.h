#ifndef DICT_H
#define DICT_H

#include "types.h"

dict_t read_file_to_words(const char* filename); 

word_t *get_random_word(dict_t *dict);
syl_t *get_random_syllable_from_word(word_t *w, bool ignore_last);

const syl_t *get_random_syllable_any(dict_t *dict, bool ignore_last);

sylvec_t sylvec_create(); 
int sylvec_contains(sylvec_t *s, const wchar_t *str);
int sylvec_pushsyl(sylvec_t *s, const syl_t *syl);
int sylvec_pushstr(sylvec_t *s, const wchar_t *syl); 

#endif
