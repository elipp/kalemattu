#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <locale.h>
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

const vcp_t *find_longest_vc_match(const char* vc, long offset);

strvec_t strvec_create() {
	strvec_t s;
	memset(&s, 0, sizeof(s));
	return s;
}

int strvec_push(strvec_t* vec, const wchar_t* str) {

	printf("strvec_push: capacity = %d, length = %d\n", vec->capacity, vec->length);

	if (vec->length < 1) {
		vec->capacity = 2;
		vec->strs = malloc(vec->capacity*sizeof(wchar_t*));
		vec->length = 1;
	}
	else if (vec->length + 1 > vec->capacity) {
		vec->capacity *= 2;
		vec->strs = realloc(vec->strs, vec->capacity*sizeof(wchar_t*));
		vec->length += 1;
	}

	vec->strs[vec->length - 1] = wcsdup(str);

	return 1;
}

typedef struct kstate_t {
	unsigned int numeric_seed;
	int LaTeX_output;
	int rules_apply;
} kstate_t;

struct meter_t {

};

static const wchar_t *vowels = L"aeiouyäöå";

char vc_map(wchar_t c) {
	if (isalpha(c)) {
		if (wcschr(vowels, c)) {
			return 'V';
		}
		else return 'C';
	}
	else return '?';
}

// wikipedia:
// Suomessa on 10 perustavaa tavutyyppiä, jos konsonanttiyhtymillä alkavia tavuja
// ei lasketa mukaan. Tavun ytimessä on lyhyt vokaali (V) tai pitkä vokaali tai
// diftongi (VV). Ne voivat muodostaa tavun yksinkin, mutta niiden edessä voi
// olla yksi konsonantti (C) ja jäljessä yksi tai kaksi konsonanttia. Perustavat
// tavutyypit ovat siis CV (ta.lo), CVC (tas.ku), CVV (saa.ri), CVVC (viet.to),
// VC (es.te), V (o.sa), VV (au.to), CVCC (kilt.ti), VVC (aal.to) ja VCC (ark.ku).
// Kahdella konsonantilla alkavia tavutyyppejä ovat CCV (pro.sentti), CCVC (pris.ma),
// CCVV (kruu.nu), CCVVC (staat.tinen) ja CCVCC (prons.si). Harvinaisempia ovat
// kolmella konsonantilla alkavat CCCV (stra.tegia), CCCVC (stres.si) ja CCCVCC
// (sprint.teri).[84]

char *get_substring(const char* input, size_t offset, size_t n) {
	size_t len = strlen(input);
	char *r = malloc((len-offset) + n + 1);
	strncpy(r, input + offset, n);
	r[n] = '\0';
	return r;
}

wchar_t *get_subwstring(const wchar_t *input, size_t offset, size_t n) {
	size_t len = wcslen(input);
	wchar_t *r = malloc(((len - offset) + n + 1) * sizeof(wchar_t));

	wmemcpy(r, input + offset, n);
	r[n] = L'\0';

	return r;
}

char *get_vc_pattern(const wchar_t* input) {
	size_t len = wcslen(input);
	char *vc = malloc(len + 1);
	for (int i = 0; i < len; ++i) {
		vc[i] = vc_map(input[i]);
	}

	vc[len] = '\0';

	printf("get_vc_pattern: input: %ls, returning %s\n", input, vc);
	return vc;
}



//fn get_vc_pattern_grep(word: &str) -> String {
//	let mut p = String::new();

//	p.push('^');
//	p.push_str(&get_vc_pattern(word));
//	p.push('$');

//	return p;

//}

char *get_vc_pattern_grep(const wchar_t* input) {
	size_t len = wcslen(input);
	char *vc = get_vc_pattern(input);
	char *r = malloc((len + 3)*sizeof(char)); // space for ^ and $ and '\0'

	r[0] = '^';
	strncpy(r + 1, vc, len);
	vc[len] = '$';
	vc[len+1] = '\0';

	free(vc);

	return r;
}


typedef struct syl_t {
	wchar_t *chars;
	long length;
	char length_class;
} syl_t;

syl_t syl_create(const wchar_t* syl, char length_class) {
	syl_t s;
	s.chars = wcsdup(syl);
	s.length = wcslen(syl);
	s.length_class = length_class;

	return s;
}

typedef struct sylvec_t {
	syl_t *syllables;
	long length;
	size_t capacity;
} sylvec_t;

sylvec_t sylvec_create() {
	sylvec_t s;
	memset(&s, 0, sizeof(s));

	return s;
}

int sylvec_contains(sylvec_t *s, const wchar_t *str) {
	for (int i = 0; i < s->length; ++i) {
		if (wcscmp(s->syllables[i].chars, str) == 0) return 1;
	}

	return 0;
}

char *string_concat(const char* str1, const char* str2) {
	size_t l1 = strlen(str1);
	size_t l2 = strlen(str2);

	char *buf = malloc(l1 + l2 + 1);

	strcat(buf, str1);
	strcat(buf, str2);

	return buf;

}

wchar_t *string_wconcat(const wchar_t* str1, const wchar_t* str2) {

	size_t l1 = wcslen(str1);
	size_t l2 = wcslen(str2);

	wchar_t *buf = malloc((l1 + l2 + 1)*sizeof(wchar_t));

	wcscat(buf, str1);
	wcscat(buf, str2);

	return buf;

}


char *string_concat_with_delim(const char* str1, const char* str2, const char* delim_between) {
	// an unnecessary allocation is made but w/e
	char *first = string_concat(str1, delim_between);
	char *final = string_concat(first, str2);
	free(first);

	return final;
}


int sylvec_pushsyl(sylvec_t *s, const syl_t *syl) {
	if (s->length < 1) {
		s->capacity = 2;
		s->syllables = malloc(s->capacity*sizeof(syl_t));
		s->length = 1;
	}
	else if (s->length + 1 > s->capacity) {
		s->capacity *= 2;
		s->syllables = realloc(s->syllables, s->capacity*sizeof(syl_t));
		s->length += 1;
	}

	s->syllables[s->length - 1] = *syl;

	return 1;

}

int sylvec_pushstr(sylvec_t *s, const wchar_t *syl) {

	printf("sylvec_pushstr: capacity = %lu, length = %lu\n", s->capacity, s->length);

	char *vc = get_vc_pattern(syl);
	const vcp_t *L = find_longest_vc_match(vc, 0);
	free(vc);
	syl_t new_syl = syl_create(syl, L->length_class);

	sylvec_pushsyl(s, &new_syl);

	return 1;

}


int sylvec_push_slice(sylvec_t *s, const sylvec_t *in) {
	for (long i = 0; i < in->length; ++i) {
		sylvec_pushsyl(s, &in->syllables[i]);
	}

	return 1;
}

wchar_t *sylvec_get_word(sylvec_t *s) {
	long total_length = 0;
	for (int i = 0; i < s->length; ++i) {
		total_length += s->syllables[i].length;
	}

	wchar_t *buf = malloc((total_length + 1)*sizeof(wchar_t));
	
	long offset = 0;
	for (int i = 0; i < s->length; ++i) {
		wcscpy(buf + offset, s->syllables[i].chars);
		offset += s->syllables[i].length;
	}

	buf[offset] = L'\0';

	return buf;
}

typedef struct word_t {
	wchar_t *chars;
	long length;
	sylvec_t syllables;
} word_t;

void word_syllabify(word_t *w);

word_t word_create(const wchar_t* chars) {
	word_t w;
	w.chars = wcsdup(chars);
	w.length = wcslen(chars);
	w.syllables = sylvec_create();
	word_syllabify(&w);

	return w;
}

int word_push_syllable(word_t *w, const wchar_t *s) {
	sylvec_pushstr(&w->syllables, s);
	return 1;
}

typedef struct dict_t {
	word_t *words;
	long num_words;
} dict_t;

dict_t dict_create(word_t *words, long num_words) {
	dict_t d;
	d.words = words;
	d.num_words = num_words;
	
	return d;
}

typedef struct foot_t {
    char **spats; // "syllable patterns" :D
    long num_spats;
} foot_t;

char *clean_string(const char* data) {
	size_t len = strlen(data);
	char *clean = malloc(len); // this will waste a little bit of memory
	int i = 0, j = 0;
	while (i < len) {
		if (isalpha(data[i])) {
			clean[j] = tolower(data[i]);
			++j;
		}
		++i;
	}

	clean[j] = '\0';
	printf("clean_string: data = \"%s\", ret = \"%s\", j = %d\n", data, clean, j);
	return clean;
}

wchar_t *clean_wstring(const wchar_t* data) {
	size_t len = wcslen(data);
	wchar_t *clean = malloc(len*sizeof(wchar_t)); // this will waste a little bit of memory
	int i = 0, j = 0;
	while (i < len) {
		if (isalpha(data[i])) {
			clean[j] = tolower(data[i]);
			++j;
		}
		++i;
	}

	clean[j] = L'\0';
	printf("clean_string: data = \"%ls\", ret = \"%ls\", j = %d\n", data, clean, j);
	return clean;
}



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
							printf("warning: nextvowel check for %s failed at offset %lu!\n", current->pattern, offset);
							full_match = false;
						}
					}
				}

				if (full_match && strlen(longest->pattern) < plen) {
					printf("new longest matching pattern = %s (input %s, offset %ld)\n", current->pattern, vc, offset);
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

bool has_diphthong(const char* syllable) {
	const char **d = &DIPHTHONGS[0];
	while (*d) {
		if (strstr(syllable, *d) != NULL) { return true; }
		++d;
	}
	return false;
}

//fn has_double_vowel(syllable: &str) -> bool {
//    for n in DOUBLEVOWELS {
//        if syllable.contains(n) { return true; }
//    }
//
//    return false;
//}
//
bool has_double_vowel(const char* syllable) {
	const char **s = &DOUBLEVOWELS[0];
	while (*s) {
		if (strstr(syllable, *s) != NULL) { return true; }
		++s;
	}

	return false;
}


//static ALLOWED_CCOMBOS: &'static [&'static str] =
//&[ "kk", "ll", "mm", "nn", "pp", "rr", "ss", "tt",
//"sd", "lk", "lt", "rt", "tr", "st", "tk", "mp"
//
//];
//

static const char* ALLOWED_CCOMBOS[] = {
"kk", "ll", "mm", "nn", "pp", "rr", "ss", "tt",
"sd", "lk", "lt", "rt", "tr", "st", "tk", "mp", NULL
};

static const char *FORBIDDEN_CCOMBOS[] = {
"nm", "mn", "sv", "vs", "kt", 
"tk", "sr", "sn", "tv", "sm", 
"ms", "tm", "tl", "nl", "tp", 
"pt", "tn", "np", "sl", "th", 
"td", "dt", "tf", "ln", "mt", 
"kn", "kh", "lr", "kp", "nr",
"ml", "mk", "km", "nv", "sh",
"ls", "hn", "tj", "sj", "pk", 
"rl", "kr", "mj", "kl", "kj",
"nj", "kv", "hs", "hl", "nh",
"pm", "mr", "tg", "mh", "hp",
"kd", "dk", "dl", "ld", "mv", 
"vm", "pr", "hh", "pn", "tr",
"ts", "ks", "md", "pj", "jp",
"kg", "pv", "ph", NULL

};

//fn has_forbidden_ccombos(word: &str) -> bool {
//	for c in FORBIDDEN_CCOMBOS {
//		if word.contains(c) { return true; }
//	}
//
//	return false;
//}
//

bool has_forbidden_ccombos(const char* input) {
	const char** c = &FORBIDDEN_CCOMBOS[0];

	while (*c) {
		if (strstr(input, *c) != NULL) { return true; }
		++c;
	}

	return false;
}

static const char FORBIDDEN_ENDCONSONANTS[] = {
"pkrmhvsl"
};

//fn has_forbidden_endconsonant(word: &str) -> bool {
//	for c in FORBIDDEN_ENDCONSONANTS {
//		let last_char = word.chars().last().unwrap();
//		if &last_char == c { return true; }
//	}
//
//	return false;
//}	
//

bool has_forbidden_endconsonant(const char *input) {
	char l = input[strlen(input)-1];
	size_t i = 0;

	while (i < sizeof(FORBIDDEN_ENDCONSONANTS)) {
		if (l == FORBIDDEN_ENDCONSONANTS[i]) { return true; }
		++i;
	}

	return false;
}


bool str_contains(const char* in, const char* pattern) {
	return strstr(in, pattern) != NULL;
}


//fn syllabify(word: &mut word_t) {

void word_syllabify(word_t *word) {

    //let mut offset = 0;
	size_t offset = 0;

	char *vc_pattern = get_vc_pattern(word->chars);
	size_t vc_len = strlen(vc_pattern);

//    while offset < vclen {
	while (offset < vc_len) {
		//    	let longest = find_longest_vc_match(&vc_pattern, offset);
		const vcp_t *longest = find_longest_vc_match(vc_pattern, offset);
		const char *pat = longest->pattern;
		size_t plen = strlen(longest->pattern);

		if (plen < 1) {
			// didn't find a sensible vc match
			break;
		}

		else {
			//    let new_syl = get_substring(&word.chars, offset, plen);
			wchar_t *new_syl = get_subwstring(word->chars, offset, plen);

			if (offset > 0 && str_contains(pat, "VV") && (!has_diphthong(new_syl)) && (!has_double_vowel(new_syl))) {
				// need to split this syllable into two
				size_t vv_offset = strstr(pat, "VV") - pat;
				printf("pattern: %s, vv_offset = %lu\n", pat, vv_offset);

				wchar_t *p1 = get_subwstring(word->chars, offset, vv_offset + 1);
				wchar_t *p2 = get_subwstring(word->chars, offset+vv_offset+1, plen - strlen(p1));

				word_push_syllable(word, p1);
				word_push_syllable(word, p2);


			} else {
				//			word.syllables.push(syl_t{chars: new_syl, length_class: longest.L});
				word_push_syllable(word, new_syl);
			}
		}

		offset = offset + plen;
	}

	free(vc_pattern);

}

static long get_filesize(FILE *fp) {
	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);
	rewind(fp);

	return size;
}

static long word_count(const wchar_t* buf) {

	wchar_t *bufdup = wcsdup(buf);
	wchar_t *endptr;

	printf("%p\n", bufdup);

	long num_words = 0;

	wchar_t *token = wcstok(bufdup, L" ", &endptr);

	while (token) {
		++num_words;
		token = wcstok(NULL, L" ", &endptr);
		printf("%ls\n", token);
	}

	free(bufdup);
	
	return num_words;
}

static char *read_file_to_buffer(FILE *fp, long *filesize_out) {

	long size = get_filesize(fp);
	char *buf = malloc(size*sizeof(char));
	fread(buf, 1, size, fp); 

	if (filesize_out) { *filesize_out = size; }
	
	return buf;
}

word_t *construct_word_list(const wchar_t* buf, long num_words) {

	word_t *words = malloc(num_words * sizeof(word_t));

	wchar_t *bufdup = wcsdup(buf);
	wchar_t *endptr;
	wchar_t *token = wcstok(bufdup, L" ", &endptr);
	long i = 0;

	while (token) {
		wchar_t *clean = clean_string(token);
		words[i] = word_create(clean);
		free(clean);
		++i;
		token = wcstok(NULL, L" ", &endptr);
	}
	
	return words;

}

wchar_t *convert_to_multibyte(const char* arg, long buffer_size) {

	wchar_t *mb = malloc(buffer_size * sizeof(wchar_t)); // will waste some memory though

	mbstate_t state;
	memset(&state, 0, sizeof(state));

	printf("Using locale %s.\n", setlocale( LC_CTYPE, "" ));

	size_t result;
	result = mbsrtowcs(mb, &arg, buffer_size, &state);

	if (result == (size_t)-1) {
	       	fputs("convert_to_multibyte: encoding error :(", stderr);
		return NULL;
	}

	return mb;
}



dict_t read_file_to_words(const char* filename) {

	dict_t d;
	memset(&d, 0, sizeof(d));

	FILE *fp = fopen(filename, "r");

	if (!fp) {
		fprintf(stderr, "error: Couldn't open file %s\n", filename);
		return d;
	}

	long filesize;

	char *buf = read_file_to_buffer(fp, &filesize);
	wchar_t *buf_mb = convert_to_multibyte(buf, filesize);

	free(buf);
	fclose(fp);

	long wc = word_count(buf_mb);
	printf("number of words: %ld\n", wc);
//	word_t *words = construct_word_list(buf, wc);

//	d = dict_create(words, wc);

	return d;
	
}

sylvec_t compile_list_of_syllables(dict_t *dict) {
	sylvec_t s;
	memset(&s, 0, sizeof(s));

	for (long i = 0; i < dict->num_words; ++i) {
		sylvec_push_slice(&s, &dict->words[i].syllables);
	}

	return s;
}

long get_random(long min, long max) {
	return rand() % max + min;
}

double get_randomf() {
	return (double)rand() / (double)RAND_MAX;
}


word_t *get_random_word(dict_t *dict) {
	return &dict->words[get_random(0, dict->num_words)];
}

static int str_hasanyof(const char* str, const char* chars) {
	const char *c = &chars[0];

	while (*c) {
		if (strchr(str, *c)) { return 1; }
	       	++c;
	}

	return 0;
}

int get_vowel_harmony_state(const char* syllable) {
	int state = 0;

	if (str_hasanyof(syllable, "aou")) state |= 0x1;
	if (str_hasanyof(syllable, "äöy")) state |= 0x2;

	return state;
}

int get_num_trailing_vowels(const char *word) {
	int num = 0;
	size_t len = strlen(word);
	while (vc_map(word[len-num-1]) == 'V' && num < len) ++num;

	return num;

}

int get_num_beginning_vowels(const char *word) {
	int num = 0;
	size_t len = strlen(word);
	while (vc_map(word[num]) == 'V' && num < len) ++num;

	return num;
}

char get_first_consonant(const char *str) {
	size_t len = strlen(str);
	int i = 0;
	while (vc_map(str[i]) != 'C' && i < len) ++i;
	if (i == len) return '\0';
	else return str[i];
}

static const char *FORBIDDEN_VOWELENDINGS[] = {
"ai", "ei", "ou", "ae", "au", "iu", "oe", "ue", "äy", "ii", "yy", "äi", "eu", NULL
};

bool ends_in_wrong_vowelcombo(const char *str) {
	const char* vcp = get_vc_pattern_grep(str);

	if (strstr("VV$", vcp)) {
		char *substr = get_substring(str, strlen(str) - 2, 2);
		const char **f = &FORBIDDEN_VOWELENDINGS[0];

		while (f) {
			if (strncmp(*f, substr, 2) == 0) { return true; }
			++f;
		}

		free(substr);
	}

	return false;
}

syl_t *get_random_syllable_from_word(word_t *w, bool ignore_last) {
	long r = get_random(0, ignore_last ? w->syllables.length - 1 : w->syllables.length);
	printf("get_random_syllable_from_word: word = %s, returning syllable %ld\n", w->chars, r);
	return &w->syllables.syllables[r];	
}

char *get_random_syllable_any(dict_t *dict, bool ignore_last) {
	word_t *w = get_random_word(dict);
	while (w->syllables.length == 1) {
		w = get_random_word(dict);
	}

	syl_t *s = get_random_syllable_from_word(w, ignore_last);

	return strdup(s->chars);
}

//fn construct_random_word<'a>(word_list: &'a Vec<word_t>, rng: &mut StdRng, max_syllables: usize, rules_apply: bool) -> String {
char *construct_random_word(dict_t *dict, long max_syllables, bool rules_apply) {

	char new_word[256]; // should be enough :D
	memset(new_word, 0, sizeof(new_word));

	int num_syllables = 3; // get random blah balh	

	if (num_syllables == 1) {
		while (1) {
			word_t *w = get_random_word(dict);
			double r = get_randomf();
			if (w->syllables.length > 1 || w->length <= 4 || (r < 0.20 && w->length <= 5)) {
				return strdup(w->chars);
			}
		}
	}

//	let mut new_syllables: Vec<String> = Vec::new();
	sylvec_t new_syllables = sylvec_create();
//	
//	let mut vharm_state = 0;
	int vharm_state = 0;
//	let mut prev_first_c = '0';
	char prev_first_c = '\0';
//
//	for n in 0..num_syllables {
	for (int n = 0; n < num_syllables; ++n) {
//		
//		let ignore_last = n == 0;
		bool ignore_last = (n == 0);
//		let mut syl = get_random_syllable_any(&word_list, rng, ignore_last);
		char *syl = get_random_syllable_any(dict, ignore_last);

//		let mut syl_vharm: usize = 0;
		int syl_vharm = 0;
//		
//		if rules_apply {
		if (rules_apply) {
			while (1) {
				syl_vharm = get_vowel_harmony_state(syl);
				char first_c = get_first_consonant(syl);
				char *concatd = string_concat(new_word, syl);

				if (syl_vharm > 0 && vharm_state != 0 && syl_vharm != vharm_state) {
					goto new_syllable;
				}
				else if (n > 0 && strlen(syl) < 2) {
					goto new_syllable;
				}
				else if (sylvec_contains(&new_syllables, syl)) {
					goto new_syllable;
				}
				else if (has_forbidden_ccombos(concatd)) {
					goto new_syllable;
				}
				else if (get_num_trailing_vowels(new_word) + get_num_beginning_vowels(syl) > 2) {
					goto new_syllable;
				}
				else if ((n == num_syllables - 1) && (has_forbidden_endconsonant(syl) || ends_in_wrong_vowelcombo(syl))) {
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
					syl = get_random_syllable_any(dict, ignore_last);

				free(concatd);
			
			}

			if (vharm_state == 0) {
				// we're still in "undefined vocal harmony" => only either 'e's or 'i's have been encountered
				if (syl_vharm > 0) {
					vharm_state = syl_vharm;
				}
			}
		}

		sylvec_pushstr(&new_syllables, syl);
		strcat(new_word, syl);

	}

	if (strlen(new_word) < 2) {
		return construct_random_word(dict, max_syllables, rules_apply);
	} 
	else {
		return strdup(new_word);
	}

}

//fn construct_word_with_foot<'a>(syllables: &'a Vec<syl_t>, rng: &mut StdRng, foot: &str) -> String {
// 	let mut new_word = String::new();
//
//	let num_syllables = foot.chars().count();
//
////	if num_syllables == 1 {
////		loop {
////			let w = get_random_word(word_list, rng);
////			let r = rng.gen::<f64>();
////
////			if w.syllables.len() == 1 || 
////			   w.chars.chars().count() <= 4 || 
////			   (r < 0.20 && w.chars.chars().count() <= 5) {
////
////			       	return w.chars.clone(); 
////			}
////		}
////	}
//
//	let mut new_syllables: Vec<String> = Vec::new();
//	
//	let mut vharm_state = 0;
//	let mut prev_first_c = '0';
//
//    for (n, LC) in foot.chars().enumerate() {
//
//        let ignore_last = n == 0;
//        let mut syl = get_syllable_with_lclass(&syllables, rng, LC);
//        let mut syl_vharm: usize = 0;
//
//        loop { 
//
//            syl_vharm = get_vowel_harmony_state(&syl); 
//            let first_c = get_first_consonant(&syl);
//
////            println!("word (so far): {}, desired lclass: {} syl: {}", new_word, LC, syl);
//
//            if syl_vharm > 0 && vharm_state != 0 && syl_vharm != vharm_state {
//                syl = get_syllable_with_lclass(&syllables, rng, LC);
//            } 
//            else if n > 0 && syl.chars().count() < 2 {
//                syl = get_syllable_with_lclass(&syllables, rng, LC);
//            }
//            else if new_syllables.contains(&syl) {
//                syl = get_syllable_with_lclass(&syllables, rng, LC);
//            }
//            else if has_forbidden_ccombos(&(new_word.clone() + &syl)) {
//                syl = get_syllable_with_lclass(&syllables, rng, LC);
//            }
//            else if get_num_trailing_vowels(&new_word) + get_num_beginning_vowels(&syl) > 2 {
//                syl = get_syllable_with_lclass(&syllables, rng, LC);
//            }
//            else if (n == num_syllables-1) && (has_forbidden_endconsonant(&syl) || ends_in_wrong_vowelcombo(&syl)) {
//                syl = get_syllable_with_lclass(&syllables, rng, LC);
//            } 
//            else if first_c != '0' && first_c == prev_first_c {
//                syl = get_syllable_with_lclass(&syllables, rng, LC);
//            }
//
//            else { 
//                prev_first_c = first_c;
//                break;
//            }
//
//        }
//
//        if vharm_state == 0 {
//            // we're still in "undefined vocal harmony" => only either 'e's or 'i's have been encountered
//            if syl_vharm > 0 {
//                vharm_state = syl_vharm;
//            }
//        }
//
//        new_syllables.push(syl.clone());
//        new_word.push_str(&syl);
//
//    }
//
////    println!("{}", new_word);
//
//	return new_word;
//
//}   

//fn get_random_syllable_from_word(word: &word_t, rng: &mut StdRng, ignore_last: bool) -> String {
//
//	let mut sindex: usize = word.syllables.len();
//	if ignore_last {
//		sindex -= 1;
//	}
//
//	let syl = word.syllables[get_random(rng, 0, sindex)].chars.clone();
//	
//	return syl;
//}

//fn get_random_syllable_any(word_list: &Vec<word_t>, rng: &mut StdRng, ignore_last: bool) -> String {
//	let mut word = get_random_word(word_list, rng);
//	loop {
//		if word.syllables.len() == 1 {
//			word = get_random_word(word_list, rng);
//		} else {
//			break;
//		}
//	}
//	let syl = get_random_syllable_from_word(&word, rng, ignore_last);
//
//	return syl;
//}


//fn get_random_syllable<'a>(syllables: &'a Vec<syl_t>, rng: &mut StdRng) -> &'a syl_t {
//    let syl = &syllables[get_random(rng, 0, syllables.len() - 1)];
//    return syl;
//}

syl_t *get_random_syllable(sylvec_t *sv) {
	return &sv->syllables[get_random(0, sv->length - 1)];
}

//fn get_syllable_with_lclass(syllables: &Vec<syl_t>, rng: &mut StdRng, length_class: char) -> String {
//
//	let mut syl = get_random_syllable(&syllables, rng);
//    loop {
//        if syl.length_class != length_class {
//            syl = get_random_syllable(&syllables, rng);
//   //         println!("{}, {} != {}", syl.chars, length_class, syl.length_class);
//        }
//        else {
//    //        println!("OK! {}, {} == {}", syl.chars, length_class, syl.length_class);
//            break;
//        }
//    }
//
//	return syl.chars.clone(); 
//
//}

syl_t *get_syllable_with_lclass(sylvec_t *sv, char length_class) {
	return &sv->syllables[0]; // TODO
}


//fn generate_random_verse<'a>(word_list: &'a Vec<word_t>, rng: &mut StdRng, num_words: usize, last_verse: bool, state: &kstate_t, foot: &str) -> String {
char *generate_random_verse(dict_t *dict, long num_words, bool last_verse, kstate_t *state, foot_t *foot) {
//	let mut new_verse = String::new();
	char new_verse[2048];
	memset(new_verse, 0, sizeof(new_verse));

//	println!("num_words: {}", num_words);

//	for j in 0..num_words {
	for (int j = 0; j < num_words; ++j) {

//		let new_word = construct_random_word(&word_list, rng, 4, state.rules_apply);
		char *new_word = construct_random_word(dict, 4, state->rules_apply);
		//new_verse.push_str(&new_word);
		strcat(new_verse, new_word);
		free(new_word);

//		let r = rng.gen::<f64>();
		double r = get_randomf();

		if (last_verse && j == num_words - 1) {
			if (r < 0.08) {
				strcat(new_verse, "!");
			}
			else if (r < 0.15) {
				strcat(new_verse, "?");
			}
			else if (r < 0.25) {
				strcat(new_verse, ".");
			}

		} else {
			if (r < 0.02) {
				strcat(new_verse, ";");
			} 
			else if (r < 0.15) {
				strcat(new_verse, ",");
		
			} else if (r < 0.18) {
				strcat(new_verse, ":");
			}

		}

		if (j < num_words - 1) {
			strcat(new_verse, " ");
		}

	}

	return strdup(new_verse);
	
}

//fn generate_verse_with_foot<'a>(syllables: &'a Vec<syl_t>, rng: &mut StdRng, state: &kstate_t, foot: &str) -> String {
//    let mut new_verse = String::new();
//
//    for w in foot.split("-") {
//        let num_syllables = w.chars().count();
//        let w = construct_word_with_foot(syllables, rng, &w);
//        new_verse = new_verse + &w + " ";
//    }
//
//    return new_verse;
//}

//fn generate_random_stanza<'a>(word_list: &'a Vec<word_t>, rng: &mut StdRng, num_verses: usize, state: &kstate_t) -> String {
char *generate_random_stanza(dict_t *dict, long num_verses, kstate_t *state) {

	char new_stanza[4096];
	memset(new_stanza, 0, sizeof(new_stanza));
//	let mut i = 0;
//    let mut foot: foot_t = foot_t { spats: Vec::new() };

//    let syllables = compile_list_of_syllables(word_list);
//    	sylvec_t s = compile_list_of_syllables(dict);

//    foot.spats.push("211-21-2-221".to_string());
//   foot.spats.push("21-2-21-2-22".to_string());
//  foot.spats.push("2-111-12-121".to_string());
// foot.spats.push("122-112-2".to_string());
	
//    for f in foot.spats {
    for (int i = 0; i < num_verses; ++i) {
//		new_stanza.push('\n');
		strcat(new_stanza, "\n");
//		let new_verse = generate_verse_with_foot(&syllables, rng, state, &f);
		char *new_verse = generate_random_verse(dict, 4, false, state, NULL);
		strcat(new_stanza, new_verse);

		if (state->LaTeX_output) {
			strcat(new_stanza, " \\\\");
		}

		free(new_verse);
		i = i + 1;
	}

	if (state->LaTeX_output) { 
		strcat(new_stanza, "!\n\n");
	}

	return strdup(new_stanza);

}

//fn capitalize_first(word: &str) -> String {
//
//    let mut v: Vec<char> = word.chars().collect();
//    v[0] = v[0].to_uppercase().nth(0).unwrap();
//    let s2: String = v.into_iter().collect();
//    let s3 = &s2;
//
//    return s3.to_string();
//
//}

char *capitalize_first_nodup(char *str) {
	str[0] = toupper(str[0]);
	return str;
}

char *capitalize_first_dup(char *str) {
	char *dup = strdup(str);
	dup[0] = toupper(dup[0]);

	return dup;
}
//fn generate_poem(word_database: &Vec<word_t>, rng: &mut StdRng, state: &kstate_t) -> String {
char *generate_poem(dict_t *dict, kstate_t *state) {

//    let distr = generate_distribution_low(1, 3);
 //   let num_words_title = get_random_with_distribution(rng, &distr);
	int num_words_title = 4;
//    let max_syllables = 4;
	int max_syllables = 4;

//    let mut title = capitalize_first(&construct_random_word(word_database, rng, max_syllables, state.rules_apply));
	char *title = capitalize_first_nodup(construct_random_word(dict, max_syllables, state->rules_apply));

    for (int i = 1; i < num_words_title; ++i) {
	    // crap
	    char *new_title = string_concat_with_delim(title, construct_random_word(dict, max_syllables, state->rules_apply), " ");
	    free(title);
	    title = new_title;
    }

//    let mut poem = String::new();
    char poem[8096];
    memset(poem, 0, sizeof(poem));

    if (state->LaTeX_output) {
		strcat(poem, "\\poemtitle{");
		strcat(poem, title);
		strcat(poem, "}\n");
		strcat(poem, "\\settowidth{\\versewidth}{levaton, sitän kylpää ranjoskan asdf}\n");
	    	strcat(poem, "\\begin{verse}[\\versewidth]\n");
    } else {
	    strcat(poem, title);
    }

//    let num_stanzas = get_random(rng, 1, 2); 
    int num_stanzas = 3;
    
    for (int i = 0; i < num_stanzas; ++i) {

//	let distr = generate_distribution_mid(1, 6);
//    	let num_verses = get_random_with_distribution(rng, &distr);
	int num_verses = 2;

//	let new_stanza = generate_random_stanza(word_database, rng, num_verses, state);
	char *new_stanza = generate_random_stanza(dict, num_verses, state);

//	poem.push_str(&format!("{}\n", new_stanza));
	strcat(poem, new_stanza);
	free(new_stanza);
    }

    if (state->LaTeX_output) {
	    strcat(poem, "\\end{verse}\n");
	    strcat(poem, "\\newpage\n");
    }

    return strdup(poem);
}

static const char *LATEX_PREAMBLE = 
"\\documentclass[12pt, a4paper]{article}\n"
"\\usepackage{verse}\n"
"\\usepackage[utf8]{inputenc}\n"
"\\usepackage[T1]{fontenc} \n"
"\\usepackage{palatino} \n"
"\\usepackage[object=vectorian]{pgfornament} %%  http://altermundus.com/pages/tkz/ornament/index.html\n"
"\\setlength{\\parindent}{0pt} \n"
"\\renewcommand{\\poemtitlefont}{\normalfont\\bfseries\\large\\centering} \n"
"\\setlength{\\stanzaskip}{0.75\\baselineskip} \n"
"\newcommand{\\sectionlinetwo}[2]{%\n"
"\nointerlineskip \\vspace{.5\\baselineskip}\\hspace{\\fill}\n"
"{\\pgfornament[width=0.5\\linewidth, color = #1]{#2}}\n"
"\\hspace{\\fill}\n"
"\\par\nointerlineskip \\vspace{.5\\baselineskip}\n"
"}%\n"
"\\begin{document}\n";

void print_latex_preamble() {
	printf("%s", LATEX_PREAMBLE);
}

void print_latex_title_page(const char* poetname) {

	static const char *LATEX_TITLEPAGE_FMT = 
"\\begin{titlepage}\n"
"\\centering\n"
"{\\fontsize{45}{50}\\selectfont %s \\par}\n"
"\\vspace{4cm}\n"
"\\sectionlinetwo{black}{7}\n"
"\\vspace{5cm}\n"
"{\\fontsize{35}{60}\\selectfont \\itshape Runoja\\par}\n"
"\\end{titlepage}";

	printf(LATEX_TITLEPAGE_FMT, poetname);

}

//fn print_as_latex_document(poems: &Vec<String>, poetname: &str) {
void print_as_latex_document(const char* poem, const char *poetname) {
	print_latex_preamble();
	print_latex_title_page(poetname);
	printf("%s", poem);
	printf("\\end{document}");

}

//fn generate_random_poetname(word_list: &Vec<word_t>, rng: &mut StdRng) -> String {
char *generate_random_poetname(dict_t *dict) {

	char name[128];
	memset(name, 0, sizeof(name));

//	let first_name = capitalize_first(&construct_random_word(word_list, rng, 3, true));
	char *first_name = capitalize_first_nodup(construct_random_word(dict, 3, true));
	//let second_initial = capitalize_first(&construct_random_word(word_list, rng, 1, true)).chars().next().unwrap();
	char *second_name = capitalize_first_nodup(construct_random_word(dict, 2, true));
	second_name[1] = '\0';

//	let surname = capitalize_first(&construct_random_word(word_list, rng, 5, true));
	char *surname = capitalize_first_nodup(construct_random_word(dict, 5, true));

	strcat(name, first_name);
	strcat(name, " ");
	strcat(name, second_name);
	strcat(name, ". ");
	strcat(name, surname);

	free(first_name);
	free(second_name);
	free(surname);

	return strdup(name);

}

//fn main() {
int main(int argc, char *argv[]) {
	setlocale(LC_ALL, "fi_FI.UTF-8");

	dict_t dict = read_file_to_words("kalevala.txt");

	if (dict.num_words < 1) {
		fprintf(stderr, "kalemattu: fatal: couldn't open input file kalevala.txt, aborting!\n");
		return 1;
	}

	unsigned int seed = time(NULL);
	srand(seed);

	printf("(info: using %u as random seed)\n\n", seed);

	kstate_t state;
	state.numeric_seed = seed;
	state.LaTeX_output = 0;
	state.rules_apply = 1;

	char *poem = generate_poem(&dict, &state);
	printf("%s\n", poem);
	free(poem);

}
