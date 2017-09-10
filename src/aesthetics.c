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
    {"VC",2 },
    {"VV", 3 },
    {"VVC", 3 },
    {"VCC", 2 },
    {"CV", 1 },
    {"CVC", 2 },
    {"CVV", 3 },
    {"CVVC", 3 },
    {"CVCC", 2 },
    {"CCV", 1 },
    {"CCVC", 2 },
    {"CCVV", 3 },
    {"CCVVC", 3 },
    {"CCVCC", 2 },
    {"CCCVC", 2 },
    {"CCCVCC", 2 },
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

// seems like the only c-combos with j are 'rj', 'hj' and 'lj'
// for v, they're 'rv', 'lv', 'sv', 'hv', 'tv'
// m:  'mm', 'lm', 'rm', 'mp', 'hm'

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
L"dv", L"jd", L"sd", L"vr", L"vh",
L"vp", L"dh", L"dj", L"pd", 
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
	while (is_vowel(word[len-num-1]) && num < len) ++num;

	return num;

}

int get_num_beginning_vowels(const wchar_t *str) {
	int num = 0;
	size_t len = wcslen(str);
	while (is_vowel(str[num]) && num < len) ++num;

	return num;
}

wchar_t get_first_consonant(const wchar_t *str) {
	size_t len = wcslen(str);
	int i = 0;
	while (!is_vowel(str[i]) && i < len) ++i;
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


static int apply_filters(const syl_t *syl, const wchar_t *buffer, filter_state_t *fs) {

	const wchar_t *s = syl->chars;
	wchar_t first_c = get_first_consonant(s);
	wchar_t *concatd = wstring_concat(buffer, s);
	int syl_vharm = get_vowel_harmony_state(s);

	int r = -1;

	if (syl_vharm > 2) r = 0;// this means the vharm was mixed

	else if (syl_vharm > 0 && fs->vharm != 0 && syl_vharm != fs->vharm) {
		r = 0;
	}
	else if (fs->syln == 0 && has_forbidden_beginconsonant(s)) {
		r = 0;
	}
	else if (fs->syln > 0 && syl->length < 2) {
		r = 0;
	}
	else if (sylvec_contains(&fs->new_syllables, s)) {
		r = 0;
	}
	else if (has_forbidden_ccombos(concatd)) {
		r = 0;
	}
	else if (get_num_trailing_vowels(buffer) + get_num_beginning_vowels(s) > 2) {
		r = 0;
	}
	else if ((fs->syln == fs->syln_target - 1) && (has_forbidden_endconsonant(s) || ends_in_wrong_vowelcombo(s))) {
		r = 0;
	}
	else if (first_c && first_c == fs->prev_first_c) {
		r = 0;
	}
	else { 
		fs->prev_first_c = first_c;
		r = 1;
	}

	free(concatd);

	return r;

}

int make_valid_word(wchar_t *buffer, long num_syllables, SYLLABLE_SOURCE_FUNC SYLLABLE_SOURCE, const char* sylp) {

	filter_state_t fstate = filter_state_new(num_syllables);
	sylsrc_args_t sargs;
	
	sargs.num_syllables = num_syllables;

	for (; fstate.syln < fstate.syln_target; ++fstate.syln) {

		printf("sylp: %s -> %ls, %ld\n", sylp, buffer, num_syllables);

		int length_class = sylp ? sylp[fstate.syln] - '0' : 0;
		sargs.length_class = length_class;

		bool ignore_last = (fstate.syln == 0);
		sargs.ignore_last = ignore_last;

		syl_t syl = SYLLABLE_SOURCE(&sargs);

		while (!apply_filters(&syl, buffer, &fstate)) { 
			syl_free(&syl);
			syl = SYLLABLE_SOURCE(&sargs); 
		}


		if (fstate.vharm == 0) {
			// we're still in "undefined vocal harmony" => only either 'e's or 'i's have been encountered
			int newsyl_vharm = get_vowel_harmony_state(syl.chars);
			if (newsyl_vharm > 0) {
				fstate.vharm = newsyl_vharm;
			}
		}

		sylvec_pushstr(&fstate.new_syllables, syl.chars);
		wcscat(buffer, syl.chars);
		syl_free(&syl);

	}

	filter_state_free(&fstate);


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

filter_state_t filter_state_new(int num_syllables) {
	filter_state_t fs;

	fs.vharm = 0;
	fs.prev_first_c = L'\0';
	fs.new_syllables = sylvec_create();
	fs.syln = 0;
	fs.syln_target = num_syllables;

	return fs;
}

void filter_state_free(filter_state_t *fs) {
	sylvec_destroy(&fs->new_syllables);
}
