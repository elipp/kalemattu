#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <wctype.h>
#include <stdbool.h>
#include <stdio.h>

#include "types.h"
#include "aesthetics.h"
#include "dict.h"
#include "stringutil.h"
#include "synth.h"

static const vcp_t VC_PATTERNS[] = {
    {"V", 1},
    {"VC",1 },
    {"VV", 2 },
    {"VVC", 2 },
    {"VCC", 1 },
    {"CV", 1 },
    {"CVC", 1 },
    {"CVV", 2 },
    {"CVVC", 2 },
    {"CVCC", 1 },
    {"CCV", 1 },
    {"CCVC", 1 },
    {"CCVV", 2 },
    {"CCVVC", 2 },
    {"CCVCC", 1 },
    {"CCCVC", 1 },
    {"CCCVCC", 1 },
    { NULL, 0}
};

static const wchar_t* DIPHTHONGS[] = {
L"yi", L"öi", L"äi", L"ui", L"oi",
L"ai", L"äy", L"au", L"yö", L"öy",
L"uo", L"ou", L"ie", L"ei", L"eu",
L"iu", L"ey", L"iy", NULL
};

static const wchar_t* DOUBLEVOWELS[] = {
 L"aa", L"ee", L"ii", L"oo", L"uu", L"yy", L"ää", L"öö", NULL
};

static const wchar_t* NON_DIPHTHONGS[] = {
 L"ae", L"ao", L"ea", L"eo", L"ia",
 L"io", L"iä", L"oa", L"oe", L"ua",
 L"ue", L"ye", L"yä", L"äe", L"äö",
 L"öä", L"eä", L"iö", L"eö", L"öe",
 L"äa", L"aä", L"oö", L"öo", L"yu",
 L"uy", L"ya", L"yu", L"äu", L"uä",
 L"uö", L"öu", L"öa", L"aö", NULL
};

bool has_diphthong(const wchar_t* syllable) {
	const wchar_t **d = &DIPHTHONGS[0];
	while (*d) {
		if (wcsstr(syllable, *d) != NULL) { return true; }
		++d;
	}
	return false;
}

bool has_double_vowel(const wchar_t* syllable) {
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
L"kg", L"pv", L"ph", L"jr", L"jv",
L"dm", L"jn", L"vn", L"vt", L"jt",
L"vj", L"vl", L"dr", L"vk", L"ds",
L"js", L"dp", L"jh", L"dn", L"jl",
L"jm", L"jj", L"vv", L"jk", L"vd",
L"dv", L"jd", L"sd",
NULL
};

bool has_forbidden_ccombos(const wchar_t* input) {
	const wchar_t **c = &FORBIDDEN_CCOMBOS[0];

	while (*c) {
		if (wcsstr(input, *c) != NULL) { return true; }
		++c;
	}

	return false;
}

static const wchar_t *FORBIDDEN_BEGINCONSONANTS= L"bdcgqf";
static const wchar_t *FORBIDDEN_ENDCONSONANTS = L"pkrmhvsljdg";

bool has_forbidden_beginconsonant(const wchar_t *input) {
	wchar_t c = input[0];

	int i = 0;
	while (i < wcslen(FORBIDDEN_BEGINCONSONANTS)) {
		if (c == FORBIDDEN_BEGINCONSONANTS[i]) {
			return true;
		}
		++i;
	}

	return false;
}

bool has_forbidden_endconsonant(const wchar_t *input) {
	wchar_t c = input[wcslen(input)-1];
	int i = 0;

	while (i < wcslen(FORBIDDEN_ENDCONSONANTS)) {
		if (c == FORBIDDEN_ENDCONSONANTS[i]) { return true; }
		++i;
	}

	return false;
}

int get_vowel_harmony_state(const wchar_t* word) {
	int state = 0;

	if (str_hasanyof(word, L"aou")) state |= 0x1;
	if (str_hasanyof(word, L"äöy")) state |= 0x2;

	return state;
}

int get_num_trailing_vowels(const wchar_t *word) {
	int num = 0;
	size_t len = wcslen(word);
	while (vc_map(word[len-num-1]) == 'V' && num < len) ++num;

	return num;

}

int get_num_beginning_vowels(const wchar_t *str) {
	int num = 0;
	size_t len = wcslen(str);
	while (vc_map(str[num]) == 'V' && num < len) ++num;

	return num;
}

wchar_t get_first_consonant(const wchar_t *str) {
	size_t len = wcslen(str);
	int i = 0;
	while (vc_map(str[i]) != 'C' && i < len) ++i;
	if (i == len) return L'\0';
	else return str[i];
}

static const wchar_t *FORBIDDEN_VOWELENDINGS[] = {
L"ai", L"ei", L"ou", L"ae", L"au", L"iu", L"oe", L"ue", L"äy", L"ii", L"yy", L"äi", L"eu", L"eo", L"ao", NULL
};

bool ends_in_wrong_vowelcombo(const wchar_t *str) {

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

static const wchar_t *vowels = L"aeiouyäöå";

bool is_vowel(wchar_t c) {
	if (wcschr(vowels, c)) return true;
	else return false;
}

char vc_map(wchar_t c) {
	if (iswalpha(c)) {
		if (is_vowel(c)) {
			return 'V';
		}
		else return 'C';
	}
	else return '?';
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

//	println!("syllable: {}, pattern: {}, length class: {}", vc, longest.P, longest.L);

//	printf("vc: %s, longest pattern: %s, offset = %lu\n", vc, longest->pattern, offset);

	return longest;

}

int make_valid_word(wchar_t *buffer, long num_syllables, SYLLABLE_SOURCE_FUNC SYLLABLE_SOURCE) {

	int vharm_state = 0;
	wchar_t prev_first_c = L'\0';

	sylvec_t new_syllables = sylvec_create();

	for (int n = 0; n < num_syllables; ++n) {
		bool ignore_last = (n == 0);

		syl_t syl = SYLLABLE_SOURCE(ignore_last);
		const wchar_t *s = syl.chars;

		int syl_vharm = get_vowel_harmony_state(s);

		while (1) {
			wchar_t first_c = get_first_consonant(s);
			wchar_t *concatd = wstring_concat(buffer, s);
			syl_vharm = get_vowel_harmony_state(s);

			if (syl_vharm > 2) goto new_syllable; // this means the vharm was mixed

			else if (syl_vharm > 0 && vharm_state != 0 && syl_vharm != vharm_state) {
				goto new_syllable;
			}
			else if (n == 0 && has_forbidden_beginconsonant(s)) {
				goto new_syllable;
			}
			else if (n > 0 && syl.length < 2) {
				goto new_syllable;
			}
			else if (sylvec_contains(&new_syllables, s)) {
				goto new_syllable;
			}
			else if (has_forbidden_ccombos(concatd)) {
				goto new_syllable;
			}
			else if (get_num_trailing_vowels(buffer) + get_num_beginning_vowels(s) > 2) {
				goto new_syllable;
			}
			else if ((n == num_syllables - 1) && (has_forbidden_endconsonant(s) || ends_in_wrong_vowelcombo(s))) {
				goto new_syllable;
			}
			else if (first_c && first_c == prev_first_c) {
				goto new_syllable;
			}
			else { 
				free(concatd);
				prev_first_c = first_c;
				break;
			}

new_syllable:
			syl_free(&syl);
			syl = SYLLABLE_SOURCE(ignore_last);
			s = syl.chars;

			free(concatd);

		}


		if (vharm_state == 0) {
			// we're still in "undefined vocal harmony" => only either 'e's or 'i's have been encountered
			if (syl_vharm > 0) {
				vharm_state = syl_vharm;
			}
		}

		sylvec_pushstr(&new_syllables, s);
		wcscat(buffer, s);
		syl_free(&syl);

	}

	sylvec_destroy(&new_syllables);

	return 1;

}

int make_any_word(wchar_t *buffer, long num_syllables, SYLLABLE_SOURCE_FUNC SYLLABLE_SOURCE) {
	for (int i = 0; i < num_syllables; ++i) {
		syl_t syl = SYLLABLE_SOURCE(false);
		wcscat(buffer, syl.chars);
		syl_free(&syl);
	}

	return 1;
}
