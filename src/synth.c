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

typedef struct combo_freq_t {
	const wchar_t *combo;
	double freq;
} combo_freq_t;

static const combo_freq_t ccombo_freqs[] = {

{ L"ll", 1212 },
{ L"st", 1082 },
{ L"tt", 745 },
{ L"ss", 517 },
{ L"kk", 420 },
{ L"ks", 413 },
{ L"nn", 408 },
{ L"nt", 342 },
{ L"mm", 337 },
{ L"lm", 327 },
{ L"nk", 322 },
{ L"lt", 317 },
{ L"ht", 313 },
{ L"ts", 249 },
{ L"sk", 230 },
{ L"lk", 230 },
{ L"ns", 193 },
{ L"tk", 191 },
{ L"rt", 183 },
{ L"hd", 175 },
{ L"lj", 159 },
{ L"hm", 109 },
{ L"ng", 94 },
{ L"mp", 93 },
{ L"rm", 89 },
{ L"lv", 86 },
{ L"sv", 83 },
{ L"hk", 80 },
{ L"rk", 78 },
{ L"ps", 78 },
{ L"rh", 73 },
{ L"rv", 65 },
{ L"rr", 62 },
{ L"rj", 53 },
{ L"pp", 48 },
{ L"rs", 41 },
{ L"np", 35 },
{ L"nh", 34 },
{ L"lp", 31 },
{ L"hr", 30 },
{ L"hj", 28 },
{ L"rtt", 25 },
{ L"ltt", 25 },
{ L"nv", 23 },
{ L"rkk", 21 },
{ L"nl", 21 },
{ L"lkk", 20 },
{ L"rsk", 18 },
{ L"rp", 18 },
{ L"hn", 18 },
{ L"mpp", 17 },
{ L"nss", 15 },
{ L"hl", 15 },
{ L"sl", 11 },

};

static combo_freq_t *ccombo_freqs_cumulative;

static combo_freq_t vcombo_freqs[] = {

{L"a", 2908},
{L"i", 1826},
{L"ä", 1491},
{L"aa", 1211},
{L"ai", 1036},
{L"uu", 851},
{L"e", 836},
{L"oi", 718},
{L"ei", 670},
{L"ie", 636},
{L"au", 568},
{L"ää", 550},
{L"ui", 526},
{L"uo", 506},
{L"ii", 491},
{L"ee", 487},
{L"o", 274},
{L"äi", 257},
{L"u", 199},
{L"ia", 176},
{L"ea", 165},
{L"ua", 158},
{L"yö", 138},
{L"eä", 132},
{L"yy", 130},
{L"ou", 104},
{L"äy", 101},
{L"iä", 93},
{L"oa", 87},
{L"oo", 84},
{L"ae", 73},
{L"eu", 60},
{L"aai", 59},
{L"yi", 55},
{L"iu", 45},
{L"öi", 39},
{L"y", 35},
{L"öy", 30},
{L"äe", 29},
{L"eää", 26},
{L"io", 25},
{L"ö", 23},
{L"aua", 22},
{L"iia", 20},
{L"ue", 18},
{L"ioi", 18},
{L"yä", 17},
{L"ey", 15},
{L"eaa", 15},
{L"oe", 12},
{L"uaa", 11},
{L"öö", 10},
{L"oaa", 10},
{L"iai", 10},

};

static combo_freq_t *vcombo_freqs_cumulative;

static void compute_combo_freqs() {
	static bool initialized = false;
	if (initialized) return;

	int num_cc = sizeof(ccombo_freqs)/sizeof(ccombo_freqs[0]);
	int num_vv = sizeof(vcombo_freqs)/sizeof(vcombo_freqs[0]);

	ccombo_freqs_cumulative = malloc(num_cc*sizeof(combo_freq_t));
	combo_freq_t *cc = ccombo_freqs_cumulative;

	vcombo_freqs_cumulative = malloc(num_vv*sizeof(combo_freq_t));
	combo_freq_t *vv = vcombo_freqs_cumulative;

	double cc_total = 0;
	for (int i = 0; i < num_cc; ++i) {
		cc_total += ccombo_freqs[i].freq;
		cc[i].combo = ccombo_freqs[i].combo;
		cc[i].freq = cc_total;
	}

	for (int i = 0; i < num_cc; ++i) {
		cc[i].freq /= cc_total;
	}

	double vv_total = 0;
	for (int i = 0; i < num_vv; ++i) {
		vv_total += vcombo_freqs[i].freq;
		vv[i].combo = vcombo_freqs[i].combo;
		vv[i].freq = vv_total;
	}

	for (int i = 0; i < num_vv; ++i) {
		vv[i].freq /= vv_total;
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


static wchar_t synth_get_letter(int want_vowel, int vharm_state) {

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
			if (is_vowel(c)) {
				wchar_t cs[2];
				cs[0] = c;
				cs[1] = L'\0';
				int h = get_vowel_harmony_state(cs);
				if (h > 0) {
					if (h == vharm_state) {
						return c;
					}
					else return synth_get_letter(want_vowel, vharm_state);
				}
				else return c;
			}
			else return synth_get_letter(want_vowel, vharm_state);
			break;

		case SYNTH_CONSONANT:
			if (!is_vowel(c)) return c;
			else return synth_get_letter(want_vowel, vharm_state);

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
			syl[i] = synth_get_letter(SYNTH_CONSONANT, 0);
		} else {
			syl[i] = synth_get_letter(SYNTH_VOWEL, 0);
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

	return iter->pattern;

}

const wchar_t *get_vowel_combo(int clen, int vharm_state) {
	
	double r = get_randomf();
	combo_freq_t *iter = &vcombo_freqs_cumulative[0];

	while (r >= iter->freq) ++iter;

	if (wcslen(iter->combo) != clen) {
		return get_vowel_combo(clen, vharm_state);
	} else {
		int h = get_vowel_harmony_state(iter->combo);
		if (h > 0) {
			if (h == vharm_state) return iter->combo;
			else return get_vowel_combo(clen, vharm_state);
		}
		else return iter->combo;
	}



}

const wchar_t *get_consonant_combo(int clen) {
	
	double r = get_randomf();
	combo_freq_t *iter = &ccombo_freqs_cumulative[0];

	while (r >= iter->freq) ++iter;

	if (wcslen(iter->combo) != clen) return get_consonant_combo(clen);

	return iter->combo;


}
const wchar_t *get_combo(int vowel, int clen, int vharm_state) {
	if (vowel) {
		return get_vowel_combo(clen, vharm_state);
	}
	else {
		return get_consonant_combo(clen);
	}
}

char *get_vcp_from_sylp(const char* sylp, int num_syllables) {
	
	char *vcp = malloc(num_syllables*4*sizeof(char));
	vcp[0] = '\0';

	for (int i = 0; i < num_syllables; ++i) {
		int length_class = sylp[i] - '0';
		vcp_t v = get_random_vcp_with_length_class(length_class);
		strcat(vcp, v.pattern);
	}

	return vcp;

}

static char *get_grep_format(const char* w) {
	int len = strlen(w);
	char *r = malloc((len+2+1)*sizeof(char));
	r[0] = '^';
	memcpy(r+1, w, len);
	r[len+1] = '$';
	r[len+2] = '\0';

	return r;

}

static int append_new_combo(wchar_t *buffer, const char* vcp, int index, int target_len, int vharm_state) {
	int i = index;

	char streak_beg = vcp[i];
	int ibeg = i;

	while (vcp[i] == streak_beg) ++i; 

	int streak_len = i - ibeg;
	printf("streak_len: %d\n", streak_len);

	if (streak_len == 1) {
		wchar_t c[2];
		c[1] = L'\0';

		if (i >= target_len - 1) {
			do {
				c[0] = synth_get_letter(streak_beg == 'V' ? SYNTH_VOWEL : SYNTH_CONSONANT, vharm_state);
				printf("checking if %ls is forbidden\n", c);
			} while (is_forbidden_endconsonant(c[0])); // this actually works for vowels also
		}
		else {
			c[0] = synth_get_letter(streak_beg == 'V' ? SYNTH_VOWEL : SYNTH_CONSONANT, vharm_state);
			printf("got single letter: %ls\n", c);
		}
		wcscat(buffer, c);
	}

	else {
		const wchar_t *p = NULL;
		if (i >= target_len - 1) {
			do {
				p = get_combo(streak_beg == 'V' ? 1 : 0, streak_len, vharm_state);
				printf("checking if %ls is forbidden\n", p);
			} while (is_forbidden_endconsonant(p[streak_len-1])); 
		}

		else {
			p = get_combo(streak_beg == 'V' ? 1 : 0, streak_len, vharm_state);
		}

		printf("got combo: %ls\n", p);
		wcscat(buffer, p);
	}

	printf("wordbuf: %ls\n", buffer);

	return i;

}

word_t synth_get_word(int num_syllables) {

	word_t word;

	wchar_t wordbuf[64];
	wordbuf[0] = L'\0';

	compute_combo_freqs();

	if (num_syllables == 1) {
		word = word_create(get_single_syllable_word());
		return word;
	}

	const char *sylp = NULL;
	char *sgrep = NULL;
	char *vcp = NULL;

	while (1) {
		sylp = synth_get_sylp(num_syllables);
		vcp = get_vcp_from_sylp(sylp, num_syllables);
		sgrep = get_grep_format(vcp);

		printf("%s, %s\n", sylp, sgrep);

		if (!strstr(sgrep, "VVV") && !strstr(sgrep, "^CC") && !strstr(sgrep, "CC$")) break;

		free(sgrep);
		free(vcp);
	}

	free(sgrep);


	int vclen = strlen(vcp);
	printf("VC PATTERN: %s\n", vcp);

	int i = 0;
	int vharm_state = 0;

	while (i < vclen) {
		i = append_new_combo(wordbuf, vcp, i, vclen, vharm_state);
		int h = get_vowel_harmony_state(wordbuf);
		if (vharm_state == 0) {
			if (h > 0) {
				vharm_state = h;
			}
		}
	}

	printf("\n");
	free(vcp);

	word = word_create(wordbuf);
	return word;
}
