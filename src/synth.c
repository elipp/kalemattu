#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "synth.h"
#include "distributions.h"
#include "aesthetics.h"
#include "dict.h"

typedef struct letter_freq_t {
	wchar_t character;
	double freq;
} letter_freq_t;

static const letter_freq_t letter_freqs[] = {
{ L'a', 457350},
{ L'i', 421366 },
{ L't', 388711 },
{ L'n', 341181 },
{ L'e', 323087 },
{ L's', 309350 },
{ L'l', 226627 },
{ L'o', 208923 },
{ L'k', 207520 },
{ L'u', 196678 },
{ L'ä', 189134 },
{ L'm', 137972 },
{ L'v', 96316 },
{ L'r', 85116 },
{ L'j', 75961 },
{ L'h', 71733 },
{ L'y', 71316 },
{ L'p', 65358 },
{ L'd', 33148 },
{ L'ö', 18655 },
//{ L'g', 4151 },
//{ L'b', 2068 },
//{ L'f', 1934 },
//{ L'c', 1091 },
//{ L'w', 329 },
//{ L'å', 52 },
//{ L'q', 26 },

};

static wchar_t *common_single_syllable_words[] = {

L"ei", L"he", L"hän", L"ja",
L"jo", L"jos", L"kuin", L"kun",
L"me", L"myös", L"ne", L"niin",
L"noin", L"nuo", L"nyt", L"näin",
L"pian", L"pois", L"päin", L"se",
L"siis", L"taas", L"tai", L"te",
L"tuo", L"vaan", L"vai", L"vain",
L"voi",

};

static wchar_t *rare_single_syllable_words[] = {

L"hai", L"hius", L"ien", L"ies",
L"jae", L"jaos", L"koe", L"koi",
L"kuu", L"kyy", L"luo", L"luu",
L"maa", L"mies", L"muu", L"oas",
L"pii", L"puu", L"pyy", L"pää",
L"rae", L"ruis", L"saos", L"sees",
L"sei", L"seis", L"seos", L"suo",
L"suu", L"syy", L"syys", L"säe",
L"sää", L"tae", L"taos", L"taus",
L"tee", L"teos", L"tie", L"tiu",
L"työ", L"vuo", L"vyö", L"yö",
L"äes",

};

static wchar_t *get_rare_single_syllable_word() {

	static const int num_rares = sizeof(rare_single_syllable_words)/sizeof(rare_single_syllable_words[0]);
	
	double r = get_randomf();
	int index = r * num_rares;

	return rare_single_syllable_words[index];

}

static wchar_t *get_common_single_syllable_word() {

	static const int num_commons = sizeof(common_single_syllable_words)/sizeof(common_single_syllable_words[0]);
	
	double r = get_randomf();
	int index = r * num_commons;

	return common_single_syllable_words[index];

}

static wchar_t *get_single_syllable_word() {

	double r = get_randomf();
	return r < 0.95 ? get_common_single_syllable_word() : get_rare_single_syllable_word();

}

static letter_freq_t *letter_freqs_cumulative;

static void compute_letter_freqs() {
	static bool initialized = false;
	if (initialized) return;

	double total = 0;
	int num_letters = sizeof(letter_freqs)/sizeof(letter_freqs[0]);

	letter_freqs_cumulative = malloc(num_letters*sizeof(letter_freq_t));
	letter_freq_t *lfc = letter_freqs_cumulative;

	for (int i = 0; i < num_letters; ++i) {
		total += letter_freqs[i].freq;
		lfc[i].character = letter_freqs[i].character;
		lfc[i].freq = total;
	}

	for (int i = 0; i < num_letters; ++i) {
		lfc[i].freq /= total;
	}

	initialized = true;

}

// CV	15088	15088
// CVC	10124	25212
// CVV	4338	29550
// CVVC	3689	33239
// V	1710	34949
// VC	1681	36630
// VV	654	37284
// CVCC	136	37420
// CCV	131	37551
// VCC	5	37556


typedef struct sylp_freq_t {
	char *pattern;
	double freq;
} sylp_freq_t;

const struct sylp_freq_t sylp_freqs[] = {

{"21", 1311},
{"11", 1278},
{"31", 1089},
{"3", 1063},
{"2", 912},
{"1", 910},
{"12", 601},
{"23", 592},
{"32", 580},
{"121", 564},
{"22", 551},
{"33", 507},
{"13", 485},
{"111", 364},
{"221", 341},
{"321", 264},
{"112", 259},
{"211", 257},
{"212", 225},
{"311", 189},
{"312", 186},
{"131", 182},
{"231", 161},
{"123", 120},
{"331", 107},
{"2121", 107},
{"223", 100},
{"132", 97},
{"323", 94},
{"313", 87},
{"232", 83},
{"3121", 82},
{"113", 82},
{"322", 78},
{"1212", 76},
{"1121", 75},
{"122", 73},
{"213", 72},
{"222", 70},
{"1211", 64},
{"233", 62},
{"332", 47},
{"1111", 45},
{"2211", 44},
{"3211", 41},
{"2112", 41},
{"1112", 39},
{"2111", 32},
{"133", 32},
{"3212", 31},
{"1221", 31},
{"2212", 29},
{"333", 27},
{"1131", 27},
{"2131", 26},
{"2123", 26},
{"2311", 24},
{"1321", 21},
{"1213", 21},

}; 

static sylp_freq_t *sylp_freqs_cumulative;

static void compute_sylp_freqs() {
	static bool initialized = false;
	if (initialized) return;

	double total = 0;
	int num_patterns = sizeof(sylp_freqs)/sizeof(sylp_freqs[0]);

	sylp_freqs_cumulative = malloc(num_patterns*sizeof(sylp_freq_t));
	sylp_freq_t *sfc = sylp_freqs_cumulative;

	for (int i = 0; i < num_patterns; ++i) {
		total += sylp_freqs[i].freq;
		sfc[i].pattern = sylp_freqs[i].pattern;
		sfc[i].freq = total;
	}

	for (int i = 0; i < num_patterns; ++i) {
		sfc[i].freq /= total;
	}

	initialized = true;

}

#define TOTAL_VCS (37556.0)

typedef struct vcp_freq_t {
	vcp_t vc;
	double freq;
} vcp_freq_t;

static const vcp_freq_t vc_freqs_cumulative[] = {
	{ {"CV", 1},  15088.0/TOTAL_VCS },
	{ {"CVC", 2}, 25212.0/TOTAL_VCS },
	{ {"CVV", 3}, 29550.0/TOTAL_VCS },
	{ {"CVVC", 3}, 33239.0/TOTAL_VCS },
	{ {"V", 1}, 34949.0/TOTAL_VCS },
	{ {"VC", 2}, 36630.0/TOTAL_VCS },
	{ {"VV", 3}, 37284.0/TOTAL_VCS },
	{ {"CVCC", 2}, 37420.0/TOTAL_VCS },
	{ {"CCV", 2}, 37551.0/TOTAL_VCS },
	{ {"VCC", 2}, 37556.0/TOTAL_VCS },

};


wchar_t synth_get_letter(int want_vowel) {

	compute_letter_freqs(); // this is run only once

	double r = get_randomf();
	const letter_freq_t *iter = &letter_freqs_cumulative[0];

	while (r >= iter->freq) {
	       	++iter;
	}
	//printf("%f -> %f (%lc)\n\n", r, iter->freq, iter->character);

	wchar_t c = iter->character;

	switch(want_vowel) {
		case SYNTH_VOWEL:
			if (is_vowel(c)) return c;
			else return synth_get_letter(want_vowel);
			break;

		case SYNTH_CONSONANT:
			if (!is_vowel(c)) return c;
			else return synth_get_letter(want_vowel);

		case SYNTH_ANY:
			return c;
			break;

		default:
			return L'?';
	}

}

static vcp_t get_random_vcp() {
	double r = get_randomf();

	const vcp_freq_t *iter = &vc_freqs_cumulative[0];

	while (r >= iter->freq) { 
		++iter;
	}
	
	return iter->vc;
}

static vcp_t get_random_vcp_with_length_class(int length_class) {
	vcp_t v = get_random_vcp();

	while (v.length_class != length_class) {
		v = get_random_vcp();
	}


	return v;
}

syl_t synth_get_syllable(sylsrc_args_t *arg) {

//	vcp_t p = get_random_vcp();

	vcp_t p = get_random_vcp_with_length_class(arg->length_class);
	long plen = strlen(p.pattern);

//	printf("%s, %d\n", p.pattern, p.length_class);
	wchar_t *syl = malloc((plen+1)*sizeof(wchar_t));
	
	for (int i = 0; i < plen; ++i) {
		if (p.pattern[i] == 'C') {
			syl[i] = synth_get_letter(SYNTH_CONSONANT);
		} else {
			syl[i] = synth_get_letter(SYNTH_VOWEL);
		}
	}

	syl[plen] = L'\0';

	syl_t r = syl_create(syl, p.length_class);
	free(syl);

	return r;
}

const char *synth_get_sylp(int num_syllables) {

	compute_sylp_freqs();

	double r = get_randomf();
	const sylp_freq_t *iter = &sylp_freqs_cumulative[0];

	while (r >= iter->freq) { 
		++iter;
	}
	
	if (strlen(iter->pattern) != num_syllables) return synth_get_sylp(num_syllables);

	printf("returning sylp %s\n", iter->pattern);
	return iter->pattern;

}
