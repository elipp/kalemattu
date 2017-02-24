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

static syl_t syl_create(const wchar_t* syl, char length_class) {
	syl_t s;

	s.chars = wcsdup(syl);
	s.length = wcslen(syl);
	s.length_class = length_class;

	return s;
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
		free(s->syllables[i].chars);
	}
	free(s->syllables);
}

static int word_push_syllable(word_t *w, const wchar_t *s) {
	sylvec_pushstr(&w->syllables, s);
	return 1;
}

static void word_syllabify(word_t *word) {

	size_t offset = 0;

	char *vc_pattern = get_vc_pattern(word->chars);
	size_t vc_len = strlen(vc_pattern);

	while (offset < vc_len) {
		const vcp_t *longest = find_longest_vc_match(vc_pattern, offset);
		const char *pat = longest->pattern;
		size_t plen = strlen(longest->pattern);

		if (plen < 1) {
			// didn't find a sensible vc match
			break;
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

}

static word_t word_create(const wchar_t* chars) {
	word_t w;
	w.chars = wcsdup(chars);
	w.length = wcslen(chars);
	w.chars[w.length] = L'\0';

	w.syllables = sylvec_create();
	word_syllabify(&w);

	return w;
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

static syl_t *get_random_syllable_from_word(const word_t *w, bool ignore_last) {
//	if (w->syllables.length == 0) { printf("FUCCCKKK\n"); return NULL; }
	if (w->syllables.length == 1) { return &w->syllables.syllables[0]; }

	long max;
	if (ignore_last) max = w->syllables.length - 1;
	else max = w->syllables.length;

	long r = get_random(0, max);

//	printf("get_random_syllable_from_word: word = %ls, returning syllable %ld\n", w->chars, r);
	return &w->syllables.syllables[r];	
}

const syl_t *dict_get_random_syllable_any(bool ignore_last) {
	const word_t *w = dict_get_random_word();
	while (w->syllables.length == 1) {
		w = dict_get_random_word();
	}

	syl_t *s = get_random_syllable_from_word(w, ignore_last);

	return s;
}

const dict_t *get_dictionary() { return &dictionary; }


