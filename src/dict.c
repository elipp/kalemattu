#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "dict.h"
#include "stringutil.h" 
#include "aesthetics.h"
#include "distributions.h"

static dict_t dictionary;

static long get_filesize(FILE *fp) {
	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);
	rewind(fp);

	return size;
}

static long word_count(const wchar_t* buf) {

	wchar_t *bufdup = wcsdup(buf);
	wchar_t *endptr;

	long num_words = 0;

	wchar_t *token = wcstok(bufdup, L" ", &endptr);

	while (token) {
		++num_words;
		token = wcstok(NULL, L" ", &endptr);
	}

	free(bufdup);
	
	return num_words;
}

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

syl_t syl_create(const wchar_t* syl, int length_class) {
	syl_t s;

	s.chars = wcsdup(syl);
	s.length = wcslen(syl);
	s.vcp.length_class = length_class;
	s.vcp.pattern = get_vc_pattern(syl);

	return s;
}

void syl_free(syl_t *s) {
	free(s->chars);
	free(s->vcp.pattern);
}

sylvec_t sylvec_create() {
	sylvec_t s;
	memset(&s, 0, sizeof(s));

	return s;
}

int sylvec_contains(sylvec_t *s, const wchar_t *str) {
	for (int i = 0; i < s->length; ++i) {
		if (wcscmp(s->syllables[i].chars, str) == 0) {
			return 1;
		}
	}

	return 0;
}

int sylvec_pushsyl(sylvec_t *s, const syl_t *syl) {

//	printf("sylvec_pushsyl: capacity = %lu, length = %lu, pushing %ls\n", s->capacity, s->length, syl->chars);

	if (s->length < 1) {
		s->capacity = 2;
		s->syllables = malloc(s->capacity*sizeof(syl_t));
	}
	else if (s->length >= s->capacity) {
		s->capacity *= 2;
		s->syllables = realloc(s->syllables, s->capacity*sizeof(syl_t));
	}

	s->length += 1;
	s->syllables[s->length - 1] = *syl;

	return 1;

}

int sylvec_pushstr(sylvec_t *s, const wchar_t *syl) {

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

void sylvec_destroy(sylvec_t *s) {
	for (int i = 0; i < s->length; ++i) {
		syl_free(&s->syllables[i]);
	}
	free(s->syllables);
}

static int word_push_syllable(word_t *w, const wchar_t *s) {
	sylvec_pushstr(&w->syllables, s);
	return 1;
}

typedef struct adjc_t {
	wchar_t consonants[8];
} adjc_t;

static void print_all_ccombos(const wchar_t *w) {
	
	wchar_t temp[16];

	if (!w || wcslen(w) < 1) return;

	int wordlen = wcslen(w);
	int i = 0, j = 0;

	while (i < wordlen) {
		j = 0;

		while (!is_vowel(w[i+j])) {
			temp[j] = w[i+j];
			++j;
		}

		if (j > 1) {
			temp[j] = L'\0';
			printf("%ls\n", temp);
		}

		i += j + 1;
	}
}

static void print_all_vcombos(const wchar_t *w) {
	
	wchar_t temp[16];

	if (!w || wcslen(w) < 1) return;

	int wordlen = wcslen(w);
	int i = 0, j = 0;

	while (i < wordlen) {
		j = 0;

		while (is_vowel(w[i+j])) {
			temp[j] = w[i+j];
			++j;
		}

		if (j > 1) {
			temp[j] = L'\0';
			printf("%ls\n", temp);
		}

		i += j + 1;
	}
}



static wchar_t *get_adjacent_consonants(const wchar_t *w, wchar_t c) {
	const wchar_t *t = wcschr(w, c);
	if (!t) return NULL;

	if (t > w) {
		const wchar_t *beg = t-1;
		const wchar_t *end = t+1;
		while (!is_vowel(*beg) && beg > w) {
			--beg;
		}
		while (!is_vowel(*end) && (end-w) < wcslen(w)) {
			++end;
		}

		wchar_t *r = malloc((end-beg+2) * sizeof(wchar_t));
		const wchar_t *iter = beg+1;
		int i = 0;
		for (; i < end-beg-1; ++i) {
			r[i] = *iter;
			++iter;
		}

		r[i] = L'\0';
		printf("%ls\n", r);
		return r;

	} else {
		return NULL;
	}
}

static char *get_sylpattern(const word_t *word) {
	char buffer[16];	
	buffer[0] = '\0';

	const sylvec_t *sv = &word->syllables;

	for (int i = 0; i < sv->length; ++i) {
		char n[8];
		sprintf(n, "%d", sv->syllables[i].vcp.length_class);
		strcat(buffer, n);
	}

	return strdup(buffer);
}

static int word_syllabify(word_t *word) {

	size_t offset = 0;

	char *vc_pattern = get_vc_pattern(word->chars);
	size_t vc_len = strlen(vc_pattern);

//	print_all_vcombos(word->chars);

	while (offset < vc_len) {
		const vcp_t *longest = find_longest_vc_match(vc_pattern, offset);
		const char *pat = longest->pattern;
		size_t plen = strlen(longest->pattern);

		if (plen < 1) {
			// didn't find a sensible vc match
			printf("warning: couldn't find a vcp match for word %ls\n", word->chars);
			word_push_syllable(word, word->chars);
			free(vc_pattern);
			return 0;
		}

		else {
			wchar_t *new_syl = get_subwstring(word->chars, offset, plen);

			if (offset > 0 && str_contains(pat, "VV") && (!has_diphthong(new_syl)) && (!has_double_vowel(new_syl))) {
				size_t vv_offset = strstr(pat, "VV") - pat;
//				printf("pattern: %s, vv_offset = %lu\n", pat, vv_offset);

				wchar_t *p1 = get_subwstring(word->chars, offset, vv_offset + 1);
				wchar_t *p2 = get_subwstring(word->chars, offset+vv_offset+1, plen - wcslen(p1));

				word_push_syllable(word, p1);
				word_push_syllable(word, p2);

				free(p1);
				free(p2);


			} else {
				word_push_syllable(word, new_syl);
			}

			free(new_syl);
		}

		offset = offset + plen;
	}


	free(vc_pattern);
	return 1;


}

word_t word_create(const wchar_t* chars) {
	word_t w;
	w.chars = wcsdup(chars);
	w.length = wcslen(chars);
	w.chars[w.length] = L'\0';

	w.syllables = sylvec_create();
	word_syllabify(&w);

	return w;
}

void word_destroy(word_t *w) {
	free(w->chars);
	sylvec_destroy(&w->syllables);
}

static dict_t dict_create(word_t *words, long num_words) {
	dict_t d;
	d.words = words;
	d.num_words = num_words;
	
	return d;
}

syl_t *sylvec_get_random(sylvec_t *sv) {
	return &sv->syllables[get_random(0, sv->length - 1)];
}

syl_t *sylvec_get_random_with_lclass(sylvec_t *sv, char length_class) {
	return &sv->syllables[0]; // TODO
}


//static sylvec_t compile_list_of_syllables(dict_t *dict) {
//	sylvec_t s;
//	memset(&s, 0, sizeof(s));
//
//	for (long i = 0; i < dict->num_words; ++i) {
//		sylvec_push_slice(&s, &dict->words[i].syllables);
//	}
//
//	return s;
//}
//
static word_t *construct_word_list(const wchar_t* buf, long num_words_in, long *num_words_out) {

	word_t *words = malloc(num_words_in * sizeof(word_t));

	wchar_t *bufdup = wcsdup(buf);
	wchar_t *endptr;
	wchar_t *token = wcstok(bufdup, L" ", &endptr);
	long i = 0;

	while (token) {
		wchar_t *clean = clean_wstring(token);
		if (clean) {
			words[i] = word_create(clean);
			free(clean);
			++i;
		}
		token = wcstok(NULL, L" ", &endptr);
	}

	free(bufdup);
	words = realloc(words, i*sizeof(word_t));



	*num_words_out = i;
	return words;

}

static char *read_file_to_buffer(FILE *fp, long *filesize_out) {

	long size = get_filesize(fp);
	char *buf = malloc(size + 1);
	fread(buf, 1, size, fp); 
	buf[size] = '\0';

	if (filesize_out) { *filesize_out = size; }
	
	return buf;
}

int read_file_to_words(const char* filename) {

	FILE *fp = fopen(filename, "r");

	if (!fp) {
		fprintf(stderr, "error: Couldn't open file %s\n", filename);
		return 0;
	}

	long filesize;

	char *buf = read_file_to_buffer(fp, &filesize);
	wchar_t *wbuf = convert_to_wchar(buf, filesize);

	purge_stringbuffer_inplace(wbuf);

	free(buf);
	fclose(fp);

	long wc = word_count(wbuf);
	long wc_actual = 0;
	printf("number of words: %ld\n", wc);
	word_t *words = construct_word_list(wbuf, wc, &wc_actual);

	dictionary = dict_create(words, wc_actual);

	free(wbuf);

//	for (int i = 0; i < d.num_words; ++i) {
//		word_t *w = &d.words[i];
//		printf("%ls, length = %lu, num_syllables = %lu\n", w->chars, w->length, w->syllables.length);
//	}

	return 1;
}

const word_t *dict_get_random_word() {
	return &dictionary.words[get_random(0, dictionary.num_words)];
}

static syl_t syl_duplicate(const syl_t *s) {
	syl_t r;
	r.chars = wcsdup(s->chars);
	r.length = s->length;
	r.vcp.pattern = strdup(s->vcp.pattern);
	r.vcp.length_class = s->vcp.length_class;

	return r;
}

static syl_t get_random_syllable_from_word(const word_t *w, bool ignore_last) {
	if (w->syllables.length == 1) { return w->syllables.syllables[0]; }

	long max;

	if (ignore_last) max = w->syllables.length - 1;
	else max = w->syllables.length;
	
	if (max == 0) {
		printf("MAX is 0 %ls\n", w->chars);
	}

	long r = get_random(0, max);

	return syl_duplicate(&w->syllables.syllables[r]);
}

syl_t dict_get_random_syllable_any(sylsrc_args_t *arg) {

	const word_t *w = dict_get_random_word();
	while (w->syllables.length == 1) {
		w = dict_get_random_word();
	}

	syl_t s;


	if (arg) s = get_random_syllable_from_word(w, arg->ignore_last);
	else s = get_random_syllable_from_word(w, false);
	
	return s;
}

const dict_t *get_dictionary() { return &dictionary; }


