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
	char* const* irc_channels;
	long num_irc_channels;
} kstate_t;

typedef struct syl_t {
	wchar_t *chars;
	char *vcp;
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

typedef struct ae_filter_state_t {
       int vharm_state;
       wchar_t prev_first_c;
       sylvec_t new_syllables;
       const syl_t *syl;
       wchar_t *buffer;
       long n;
       long num_syllables;
} ae_filter_state_t;

typedef int (*ae_filterfunc)(ae_filter_state_t*);

typedef struct aesthetics_t {
	int vowel_harmony;
} aesthetics_t;

typedef struct vcb_t {
	unsigned char pattern;
	unsigned char length;
} vcb_t;

#endif
