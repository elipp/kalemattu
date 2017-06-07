#ifndef TYPES_H
#define TYPES_H

#include <ctype.h>
#include <stdbool.h>
#include <locale.h>
#include <wctype.h>
#include <wchar.h>

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
	char* const* irc_channels;
	const char* irc_nick;
	long num_irc_channels;

	int fcgi_enabled;
	const char* fcgi_addr;
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

typedef struct poem_t {
	wchar_t *title;
	wchar_t **stanzas;
	long num_stanzas;

} poem_t;

typedef struct aesthetics_t {
	int vowel_harmony;

} aesthetics_t;

#endif
