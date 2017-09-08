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
	char* pattern;
	int length_class;
} vcp_t;

typedef struct kstate_t {
	unsigned int numeric_seed;
	int LaTeX_output;
	int rules_apply;

	int irc_enabled;
	char* const* irc_channels;
	const char* irc_nick;
	long num_irc_channels;
	int synth_enabled;

	int fcgi_enabled;
	const char* fcgi_addr;
} kstate_t;

typedef struct syl_t {
	wchar_t *chars;
	int length;
	vcp_t vcp;
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

typedef struct verse_t {
	wchar_t *verse;
	long length;
} verse_t;

typedef struct stanza_t {
	verse_t *verses;
	long num_verses;
} stanza_t;

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
	stanza_t *stanzas;
	long num_stanzas;

} poem_t;

typedef struct filter_state_t {
	wchar_t prev_first_c;
	int vharm;
	sylvec_t new_syllables;
	int syln;
	int syln_target;
} filter_state_t;

typedef struct sylsrc_args_t {
	bool ignore_last;
	int num_syllables;
	int length_class;
} sylsrc_args_t;

typedef syl_t (*SYLLABLE_SOURCE_FUNC)(sylsrc_args_t *);

#endif
