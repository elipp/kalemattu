#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "aesthetics.h"
#include "types.h"
#include "dict.h"
#include "stringutil.h"
#include "distributions.h"

static const vcp_t VC_PATTERNS[] = {
    {"V", '1'},
    {"VC",'1'},
    {"VV", '2' },
    {"VVC", '2' },
    {"VCC", '1' },
    {"CV", '1' },
    {"CVC", '1' },
    {"CVV", '2' },
    {"CVVC", '2' },
    {"CVCC", '1' },
    {"CCV", '1' },
    {"CCVC", '1' },
    {"CCVV", '2' },
    {"CCVVC", '2' },
    {"CCVCC", '1' },
    {"CCCVC", '1' },
    {"CCCVCC", '1' },
    { NULL, '0'}
};

enum {
	P_V 	= 0x1,
	P_VC 	= 0x1,
	P_VCC 	= 0x1,
	P_CV 	= 0x2,
	P_CVC 	= 0x2,
	P_CVCC 	= 0x2,
	P_VV 	= 0x3,
	P_VVC 	= 0x3,
	P_CCV 	= 0x4,
	P_CCVC 	= 0x4,
	P_CCVCC	= 0x4,
	P_CVV 	= 0x6,
	P_CVVC 	= 0x6,
	P_CCCVC	= 0x8,
	P_CCCVCC= 0x8,
	P_CCVV 	= 0xC,
	P_CCVVC	= 0xC,
};
	
static unsigned char vc_patterns_binary[][5] = {
	{ P_V },
	{ P_VC, P_CV, P_VV }, 
	{ P_VCC, P_CVC, P_VVC, P_CCV, P_CVV }, 
	{ P_CVCC, P_CCVC, P_CVVC, P_CCVV }, 
	{ P_CCVCC, P_CCCVC, P_CCVVC, }, 
	{ P_CCCVCC } 
};


static char vc_map(wchar_t c) {

	static const wchar_t *vowels = L"aeiouyäöå";

	if (iswalpha(c)) {
		if (wcschr(vowels, c)) {
			return 'V';
		}
		else return 'C';
	}
	else return '?';
}

static int vc_map_binary(wchar_t c) {

	static const wchar_t *vowels = L"aeiouyäöå";
	if (iswalpha(c)) {
		if (wcschr(vowels, c)) {
			return 1;
		}
		else return 0;
	}

	else return -1;

}

// the longest word in the finnish language is nowhere near 64 characters in length
uint64_t get_vc_binary(const wchar_t *input) {
	size_t len = wcslen(input);
	if (len > 64) {
		fprintf(stderr, "(warning: get_vc_binary: wcslen of word \"%ls\" > 64, truncating)\n", input);
		len = 64;
	}

	uint64_t r = 0;

	for (int i = 0; i < len; ++i) {
		int s = vc_map_binary(input[i]);
		if (s < 0) { 
			fprintf(stderr, "vc_map_binary returned -1, rip\n");
			return 0;
		}
		r |= (uint64_t)s << i;
	}

	fprintf(stderr, "returning %lu for word \"%ls\"\n", r, input);

	return r;
}

#define RESET   "\033[0m"
#define ATTENTION "\033[1m\033[31m"      

static void printbits(unsigned char v, int num) {
  for(int i = 0; i < num; i++) fputc('0' + ((v >> i) & 1), stderr);
  fputc('\n', stderr);
}

static int has_two_consecutive_vowels(unsigned char p) {
	static const unsigned char twobits = 0x3;

	for (int i = 0; i < 7; ++i) {
		if (((p & (twobits << i)) >> i) == twobits) {
			return 1;
		}
	}

	return 0;

}

vcb_t find_longest_vcp_binary(const wchar_t *word, uint64_t vcp, long length, long offset) {
	long rem = length - offset;

	int begin = ((rem > 6) ? 6 : rem) - 1;
	uint64_t var_base = (vcp >> offset);

	vcb_t r;
	memset(&r, 0, sizeof(r));

	for (int i = begin; i >= 0; --i) {
		long pattern_length = i+1;
		uint64_t var = var_base & (0x3F >> (5 - i)); // 0x3f == 0011 1111 in binary

		for (int j = 0; j < 5; ++j) {
			unsigned char pat = vc_patterns_binary[i][j];
			if (!pat) break;  // this is some stupid shit right here... :D:D
			if (var == pat) {  // pattern match
				// get last bit			
//				printbits(pat, pattern_length);
				bool nextvowel = var_base & (1 << pattern_length);
				bool lastc = (pat & (1 << i)) == 0;

				if (has_two_consecutive_vowels(pat)) {
					wchar_t *sub = get_subwstring(word, offset, pattern_length);
					bool good = has_diphthong(sub) || has_double_vowel(sub);
					free(sub);

					if (good) {
						r.pattern = pat;
						r.length = (lastc && !nextvowel) ? pattern_length : pattern_length - 1;
						fprintf(stderr, "exit #1, nextvowel = %d, lastc = %d\n", nextvowel, lastc);
						printbits(pat, pattern_length);
						return r;
					}

				}

				if (lastc && !nextvowel) {
					// if the last char is a consonant, see if the next one is a vowel;
					// and if it is, then ignore this pattern
					r.pattern = pat;
					r.length = pattern_length;
					fprintf(stderr, "exit #2, nextvowel = %d, lastc = %d\n", nextvowel, lastc);
					printbits(pat, pattern_length);
					return r;
				}
				
				r.pattern = pat;
				r.length = (lastc && !nextvowel) ? pattern_length : pattern_length - 1;
				
				fprintf(stderr, "exit #3, nextvowel = %d, lastc = %d\n", nextvowel, lastc);
				printbits(pat, pattern_length);

				return r;
			
		}
	}
}

fprintf(stderr, "(warning: get_longest_binary returned 0)\n");
return r;

}

// a  11,90
// i  10,64
// t  9,77
// n  8,67
// e  8,21
// s  7,85
// l  5,68
// k  5,34
// o  5,24
// u  5,06
// ä  4,59
// m  3,30
// v  2,52
// r  2,32
// j  1,91
// h  1,83
// y  1,79
// p  1,74
// d  0,85
// ö  0,49
// g  0,13
// b  0,06
// f  0,06
// c  0,04
// w  0,01
// å  0,00
// q  0,00

typedef struct char_frequency_t {
	wchar_t c;
	double freq;
} char_frequency_t;

static const char_frequency_t CHAR_FREQUENCIES[] = {
	{ L'a', 0.11940196341735},
	{ L'i', 0.106705906559502},
	{ L't', 0.09795303211924},
	{ L'n', 0.086937246907322},
	{ L'e', 0.082320425749348},
	{ L's', 0.078789157516709},
	{ L'l', 0.056996763071229},
	{ L'k', 0.053548844043169},
	{ L'o', 0.05256435706155},
	{ L'u', 0.05080033355788},
	{ L'ä', 0.046004734395881},
	{ L'm', 0.033127080961874},
	{ L'v', 0.025278565523951},
	{ L'r', 0.023282211145681},
	{ L'j', 0.019162666643044},
	{ L'h', 0.018321926840336},
	{ L'y', 0.017985711449886},
	{ L'p', 0.017427634167054},
	{ L'd', 0.008483902127901},
	{ L'ö', 0.004907536741094},
	{ L'\0', 0 }
};

static const char_frequency_t CHAR_FREQUENCIES_CUMULATIVE[] = {
	{L'a', 0.11940196341735},
	{L'i', 0.226107869976851},
	{L't', 0.324060902096092},
	{L'n', 0.410998149003413},
	{L'e', 0.493318574752761},
	{L's', 0.57210773226947},
	{L'l', 0.629104495340699},
	{L'k', 0.682653339383868},
	{L'o', 0.735217696445418},
	{L'u', 0.786018030003298},
	{L'ä', 0.832022764399179},
	{L'm', 0.865149845361053},
	{L'v', 0.890428410885004},
	{L'r', 0.913710622030685},
	{L'j', 0.932873288673729},
	{L'h', 0.951195215514065},
	{L'y', 0.969180926963951},
	{L'p', 0.986608561131004},
	{L'd', 0.995092463258906},
	{L'ö', 1},
	{L'\0', 0 },
};


static wchar_t synth_get_random_char() {
	double r = get_randomf();

	const char_frequency_t *i = &CHAR_FREQUENCIES_CUMULATIVE[0];
	while (i->c && r < i->freq) ++i;
	
	return i->c;
}

static wchar_t synth_get_random_consonant() {
	wchar_t c = synth_get_random_char();
	while (vc_map_binary(c)) c = synth_get_random_char();

	return c;
}

static wchar_t synth_get_random_vowel() {
	wchar_t c = synth_get_random_char();
	while (!vc_map_binary(c)) c = synth_get_random_char();

	return c;
}



int experimental_synth_get_syllable(long length, wchar_t *buffer) {

	buffer[0] = synth_get_random_consonant();
	for (int i = 1; i < length; ++i) {
		buffer[i] = synth_get_random_char();
	}

	buffer[length] = L'\0';
}


int experimental_longest(const wchar_t *word, uint64_t vcp, long length, long offset) {

	static const unsigned char cv = 0x2;
	static const unsigned char vv = 0x3;

	int ignore_first = !(vcp & 0x1) && offset == 0;
	fprintf(stderr, "offset = %ld, ignore_first: %d\n", offset, ignore_first);

	for (long i = offset; i < length; ++i) {
		uint64_t b = (vcp >> i) & 0x3;

		if (b == cv) {
			//fprintf(stderr, "have a CV at offset %ld (%ls)\n", i, sub);
			if (ignore_first) { 
				ignore_first = 0; 
				fprintf(stderr, "ig0, i = %ld\n", i);
			}
			else { 
				fprintf(stderr, "(exit 1, CV) %ld\n", i);
			}
		}

		else if (b == vv) {
			if (ignore_first) { 
				ignore_first = 0; 
				fprintf(stderr, "ig0, i = %ld\n", i);
				++i;
			}

			else {
				wchar_t *sub = get_subwstring(word, i, 2);
				bool good = (has_diphthong(sub) || has_double_vowel(sub));
				free(sub);

				if (good) {
					fprintf(stderr, "(exit 2, DIPHTHONG/DOUBLE) %ld\n", i);

				} else {
					fprintf(stderr, "(exit 3 VOKAALIYHTYMA) %ld\n", i);
				}
			}
		}


	}

	return -1;
}

syl_t get_next_syllable(const wchar_t *word, uint64_t vcp, long length, long offset) {

	syl_t s;
	memset(&s, 0, sizeof(s));
	vcb_t longest = find_longest_vcp_binary(word, vcp, length, offset);

	s.chars = get_subwstring(word, offset, longest.length);
	s.length = longest.length;
	//	s.length_class = has_two_consecutive_vowels(longest.pattern) ? 2 : 1;

	return s;

}

char *get_vc_pattern(const wchar_t* input) {
	size_t len = wcslen(input);
	char *vc = malloc(len + 1);
	for (int i = 0; i < len; ++i) {
		vc[i] = vc_map(input[i]);
	}

	vc[len] = '\0';

	//	printf("get_vc_pattern: input: %ls, returning %s\n", input, vc);
	return vc;
}

char *get_vc_pattern_grep(const wchar_t* input) {
	size_t len = wcslen(input);
	char *vc = get_vc_pattern(input);
	char *r = malloc((len + 3)*sizeof(char)); // space for ^ and $ and '\0'

	r[0] = '^';
	strncpy(r + 1, vc, len);
	r[len+1] = '$';
	r[len+2] = '\0';

	free(vc);

	return r;
}

const vcp_t *find_longest_vc_match(const char* vc, long offset) {
	static const vcp_t error_pat = {"", '0'};

	const vcp_t *longest = &error_pat;
	long vclen = strlen(vc);
	const vcp_t *current = &VC_PATTERNS[0];

	while (current->pattern != NULL) {

		bool full_match = false;
		size_t plen = strlen(current->pattern);

		if ((vclen - offset) >= plen) {
			if (strncmp(current->pattern, vc + offset, plen) == 0) {
				full_match = true;
			}

			if (full_match) {
				if ((vclen - offset) > plen) {
					if (current->pattern[plen-1] == 'C') {
						if (vc[plen+offset] == 'V') {
							//							printf("warning: nextvowel check for %s failed at offset %lu!\n", current->pattern, offset);
							full_match = false;
						}
					}
				}

				if (full_match && strlen(longest->pattern) < plen) {
					//					printf("new longest matching pattern = %s (input %s, offset %ld)\n", current->pattern, vc, offset);
					longest = current;
				}
			}
		}

		++current;
	}

	//	printf("vc: %s, longest pattern: %s, offset = %lu\n", vc, longest->pattern, offset);

	return longest;

}



bool has_diphthong(const wchar_t* syllable) {

	static const wchar_t* DIPHTHONGS[] = {
		L"yi", L"öi", L"äi", L"ui", L"oi",
		L"ai", L"äy", L"au", L"yö", L"öy",
		L"uo", L"ou", L"ie", L"ei", L"eu",
		L"iu", L"ey", L"iy", NULL
	};

	static const wchar_t* NON_DIPHTHONGS[] = { // not really used 
		L"ae", L"ao", L"ea", L"eo", L"ia",
		L"io", L"iä", L"oa", L"oe", L"ua",
		L"ue", L"ye", L"yä", L"äe", L"äö",
		L"öä", L"eä", L"iö", L"eö", L"öe",
		L"äa", L"aä", L"oö", L"öo", L"yu",
		L"uy", L"ya", L"yu", L"äu", L"uä",
		L"uö", L"öu", L"öa", L"aö", NULL
	};

	const wchar_t **d = &DIPHTHONGS[0];
	while (*d) {
		if (wcsstr(syllable, *d) != NULL) { return true; }
		++d;
	}
	return false;
}

bool has_double_vowel(const wchar_t* syllable) {

	static const wchar_t* DOUBLEVOWELS[] = {
		L"aa", L"ee", L"ii", L"oo", L"uu", L"yy", L"ää", L"öö", NULL
	};

	const wchar_t **s = &DOUBLEVOWELS[0];
	while (*s) {
		if (wcsstr(syllable, *s) != NULL) { return true; }
		++s;
	}

	return false;
}

static const wchar_t *ALLOWED_CCOMBOS[] = {
	L"kk", L"ll", L"mm", L"nn", L"pp", L"rr", L"ss", L"tt",
	L"sd", L"lk", L"lt", L"rt", L"tr", L"st", L"tk", L"mp", NULL
};

static const wchar_t *FORBIDDEN_CCOMBOS[] = {
	L"nm", L"mn", L"sv", L"vs", L"kt", 
	L"tk", L"sr", L"sn", L"tv", L"sm", 
	L"ms", L"tm", L"tl", L"nl", L"tp", 
	L"pt", L"tn", L"np", L"sl", L"th", 
	L"td", L"dt", L"tf", L"ln", L"mt", 
	L"kn", L"kh", L"lr", L"kp", L"nr",
	L"ml", L"mk", L"km", L"nv", L"sh",
	L"ls", L"hn", L"tj", L"sj", L"pk", 
	L"rl", L"kr", L"mj", L"kl", L"kj",
	L"nj", L"kv", L"hs", L"hl", L"nh",
	L"pm", L"mr", L"tg", L"mh", L"hp",
	L"kd", L"dk", L"dl", L"ld", L"mv", 
	L"vm", L"pr", L"hh", L"pn", L"tr",
	L"ts", L"ks", L"md", L"pj", L"jp",
	L"kg", L"pv", L"ph", NULL
};

static bool has_forbidden_ccombos(const wchar_t* input) {
	const wchar_t **c = &FORBIDDEN_CCOMBOS[0];

	while (*c) {
		if (wcsstr(input, *c) != NULL) { return true; }
		++c;
	}

	return false;
}

static const wchar_t *FORBIDDEN_STARTCONSONANTS = L"d";

static bool has_forbidden_startconsonant(const wchar_t *input) {
	wchar_t l = input[0];

	const wchar_t *c = &FORBIDDEN_STARTCONSONANTS[0];
	while (*c) {
		if (l == *c) { return true; }
	}

	return false;

}

static const wchar_t *FORBIDDEN_ENDCONSONANTS = L"pkrmhvsl";

bool has_forbidden_endconsonant(const wchar_t *input) {
	wchar_t l = input[wcslen(input)-1];

	const wchar_t *c = &FORBIDDEN_ENDCONSONANTS[0];
	while (*c) {
		if (l == *c) { return true; }
		++c;
	}

	return false;
}

static int get_vowel_harmony_state(const wchar_t* word) {
	int state = 0;

	if (str_hasanyof(word, L"aou")) state |= 0x1;
	if (str_hasanyof(word, L"äöy")) state |= 0x2;

	return state;
}

static int get_num_trailing_vowels(const wchar_t *word) {
	int num = 0;
	size_t len = wcslen(word);
	while (vc_map(word[len-num-1]) == 'V' && num < len) ++num;

	return num;

}

static int get_num_beginning_vowels(const wchar_t *str) {
	int num = 0;
	size_t len = wcslen(str);
	while (vc_map(str[num]) == 'V' && num < len) ++num;

	return num;
}

static wchar_t get_first_consonant(const wchar_t *str) {
	size_t len = wcslen(str);
	int i = 0;
	while (vc_map(str[i]) != 'C' && i < len) ++i;
	if (i == len) return L'\0';

	else return str[i];
}

static const wchar_t *FORBIDDEN_VOWELENDINGS[] = {
	L"ai", L"ei", L"ou", L"ae", L"au", L"iu", L"oe", L"ue", L"äy", L"ii", L"yy", L"äi", L"eu", L"öy",  NULL
};

static bool ends_in_wrong_vowelcombo(const wchar_t *str) {

	char* vcp = get_vc_pattern_grep(str);
	bool wrong = false;

	if (strstr(vcp, "VV$")) {

		wchar_t *substr = get_subwstring(str, wcslen(str) - 2, 2);
		const wchar_t **f = FORBIDDEN_VOWELENDINGS;

		while (*f) {
			if (wcsncmp(*f, substr, 2) == 0) { 
				wrong = true; 
				break; 
			}
			++f;
		}

		free(substr);
	}
	free(vcp);
	return wrong;
}

static int filter_vharm_mismatch(ae_filter_state_t *fs) {

	int syl_vharm = get_vowel_harmony_state(fs->syl->chars);
	if (syl_vharm > 0 && fs->vharm_state != 0 && syl_vharm != fs->vharm_state) {
		return 0;
	}
	return 1;
}
static int filter_too_short(ae_filter_state_t *fs) {

	if (fs->n > 0 && fs->syl->length < 2) { return 0; }
	return 1;
}

static int filter_already_contains(ae_filter_state_t *fs) {
	if (sylvec_contains(&fs->new_syllables, fs->syl->chars)) {
		return 0;
	}
	return 1;
}

static int filter_forbidden_ccombos(ae_filter_state_t *fs) {
	wchar_t *concatd = wstring_concat(fs->buffer, fs->syl->chars);

	int r = has_forbidden_ccombos(concatd) ? 0 : 1;
	free(concatd);

	return r;
}

static int filter_too_many_consecutive_vowels(ae_filter_state_t *fs) {
	if (get_num_trailing_vowels(fs->buffer) + get_num_beginning_vowels(fs->syl->chars) > 2) {
		return 0;
	}
	return 1;
}

static int filter_forbidden_ending(ae_filter_state_t *fs) {
	if ((fs->n == fs->num_syllables - 1) && 
			(has_forbidden_endconsonant(fs->syl->chars) || ends_in_wrong_vowelcombo(fs->syl->chars))) {
		return 0;
	}

	return 1;
}

static int filter_redundant_firstc(ae_filter_state_t *fs) {

	wchar_t first_c = get_first_consonant(fs->syl->chars);

	if (first_c && first_c == fs->prev_first_c) {
		return 0;
	}
	return 1;
}

static ae_filterfunc FILTER_FUNCS[] = {
	filter_vharm_mismatch,
	filter_too_short,
	filter_already_contains,
	filter_forbidden_ccombos,
	filter_too_many_consecutive_vowels,
	filter_forbidden_ending,
	filter_redundant_firstc,
	NULL
};

static int apply_filters(ae_filter_state_t *fs) {
	ae_filterfunc *filter = &FILTER_FUNCS[0];

	while (*filter) {
		if (!(*filter)(fs)) return 0;
		++filter;
	}

	return 1;
}

static void get_filtered_syllable(ae_filter_state_t *fs) {
	bool ignore_last = (fs->n == 0);

	fs->syl = dict_get_random_syllable_any(ignore_last);

	while (1) {
		if (!apply_filters(fs)) {
			fs->syl = dict_get_random_syllable_any(ignore_last);
		}
		else break;
	}

}

int make_valid_word(wchar_t *buffer, long num_syllables) {

	ae_filter_state_t s;
	memset(&s, 0, sizeof(s));

	s.new_syllables = sylvec_create();
	s.num_syllables = num_syllables;
	s.buffer = buffer;

	for (; s.n < s.num_syllables; ++s.n) {

		get_filtered_syllable(&s);

		if (s.vharm_state == 0) {
			// we're still in "undefined vocal harmony" => only either 'e's or 'i's have been encountered
			int syl_vharm = get_vowel_harmony_state(s.syl->chars);
			if (syl_vharm > 0) {
				s.vharm_state = syl_vharm;
			}
		}

		s.prev_first_c = get_first_consonant(s.syl->chars);
		sylvec_pushsyl(&s.new_syllables, s.syl);
		wcscat(buffer, s.syl->chars);

	}

	sylvec_destroy(&s.new_syllables);

	return 1;

}

int make_any_word(wchar_t *buffer, long num_syllables) {
	for (int i = 0; i < num_syllables; ++i) {
		const syl_t *syl = dict_get_random_syllable_any(false);
		wcscat(buffer, syl->chars);
	}

	return 1;
}
