#ifndef TYPES_H
#define TYPES_H

typedef struct strvec_t {
	wchar_t **strs;
	int length;
	int capacity;
} strvec_t;

typedef struct vcp_t {
	const char* pattern;
	char length_class;
} vcp_t;

typedef struct kstate_t {
	unsigned int numeric_seed;
	int LaTeX_output;
	int rules_apply;
	int irc_enabled;
	const char* irc_channel;
} kstate_t;

typedef struct syl_t {
	wchar_t *chars;
	long length;
	char length_class;
} syl_t;

typedef struct sylvec_t {
	syl_t *syllables;
	long length;
	size_t capacity;
} sylvec_t;

typedef struct word_t {
	wchar_t *chars;
	long length;
	sylvec_t syllables;
} word_t;

typedef struct dict_t {
	word_t *words;
	long num_words;
} dict_t;

typedef struct foot_t {
    char **spats; // "syllable patterns" :D
    long num_spats;
} foot_t;

#endif
