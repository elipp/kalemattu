#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "types.h"
#include "aesthetics.h"
#include "dict.h"
#include "stringutil.h"

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

static int has_two_consecutive_vowels(unsigned char p) {
	static const unsigned char twobits = 0x3;

	for (int i = 0; i < 7; ++i) {
		if (((p & (twobits << i)) >> i) == twobits) {
			fprintf(stderr, "pattern %u has two consecutive vowels.\n", p);
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
		uint64_t var = var_base & (0x3F >> (5 - i));
		wchar_t *sub = get_subwstring(word, offset, i + 1);
		fprintf(stderr, "%ls\n", sub);

		for (int j = 0; j < 5; ++j) {
			unsigned char pat = vc_patterns_binary[i][j];

			if ((var ^ pat) == 0) { 
				if (var_base & (1 << (i + 1))) {
					// ^^^ is basically a nextvowel check
					if (has_diphthong(sub) || has_double_vowel(sub)) {
						fprintf(stderr, "%ls has a passing double vowel! (pattern: %u)\n", sub, pat);
						r.pattern = pat;
						r.length = i + 1;
						return r;
					} 
					else {
						fprintf(stderr, "%ls had a vowel as next letter!\n",sub);
						r.pattern = pat & (0x3F >> (6 - i));
						r.length = i + 1;
						return r;
					}

					
				} else {
					fprintf(stderr, "%ls pattern match, no next vowel\n", sub);
					r.pattern = pat;
					r.length = i + 1;
					return r;
				}
			}
		}
	}

	//fprintf(stderr, "var_base: %lu\n", var_base);
	return r;

}


syl_t get_next_syllable(const wchar_t *word, uint64_t vcp, long length, long offset) {

	syl_t s;
	memset(&s, 0, sizeof(s));
	vcb_t longest = find_longest_vcp_binary(word, vcp, length, offset);

	if (has_two_consecutive_vowels(longest.pattern)) {
		wchar_t *sub = get_subwstring(word, offset, longest.length);

		fprintf(stderr, "need to check if syllable \"%ls\" has diphthong or double vowelz\n", sub);
		free(sub);
		return s;
	}


//	if (offset > 0 && str_contains(pat, "VV") && (!has_diphthong(new_syl)) && (!has_double_vowel(new_syl))) {
//		size_t vv_offset = strstr(pat, "VV") - pat;
//		//				printf("pattern: %s, vv_offset = %lu\n", pat, vv_offset);
//
//		wchar_t *p1 = get_subwstring(word->chars, offset, vv_offset + 1);
//		wchar_t *p2 = get_subwstring(word->chars, offset+vv_offset+1, plen - wcslen(p1));
//
//		word_push_syllable(word, p1);
//		word_push_syllable(word, p2);
//
//		free(p1);
//		free(p2);
//	}


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
