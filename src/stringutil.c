#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <wchar.h>
#include <wctype.h>
#include <string.h>
#include <stdbool.h>

#include "stringutil.h"

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
//	printf("get_subwstring: input: %ls, offset: %lu, n: %lu -> r = %ls\n", input, offset, n, r);

	return r;
}

char *string_concat(const char* str1, const char* str2) {
	size_t l1 = strlen(str1);
	size_t l2 = strlen(str2);

	char *buf = malloc(l1 + l2 + 1);

	strcat(buf, str1);
	strcat(buf, str2);

	return buf;

}

wchar_t *wstring_concat(const wchar_t* str1, const wchar_t* str2) {

	size_t l1 = wcslen(str1);
	size_t l2 = wcslen(str2);

	size_t combined = l1+l2;
	size_t bufsize_bytes = (combined+1)*sizeof(wchar_t);

	wchar_t *buf = malloc(bufsize_bytes);
	buf[0] = L'\0';

	wcscat(buf, str1);
	wcscat(buf, str2);
	buf[combined] = L'\0';

	return buf;

}


char *string_concat_with_delim(const char* str1, const char* str2, const char* delim_between) {
	// an unnecessary allocation is made but w/e
	char *first = string_concat(str1, delim_between);
	char *final = string_concat(first, str2);
	free(first);

	return final;
}

wchar_t *wstring_concat_with_delim(const wchar_t* str1, const wchar_t* str2, const wchar_t* delim_between) {
	// an unnecessary allocation is made but w/e
	wchar_t *first = wstring_concat(str1, delim_between);
	wchar_t *final = wstring_concat(first, str2);
	free(first);

	return final;
}

char *clean_string(const char* data) {
	size_t len = strlen(data);
	char *clean = malloc(len+1); // this will waste a little bit of memory
	int i = 0, j = 0;
	while (i < len) {
		if (isalpha(data[i])) {
			clean[j] = tolower(data[i]);
			++j;
		}
		++i;
	}

	clean[j] = '\0';
//	printf("clean_string: data = \"%s\", ret = \"%s\", j = %d\n", data, clean, j);
	return clean;
}

wchar_t *clean_wstring(const wchar_t* data) {

	size_t len = wcslen(data);
	wchar_t *clean = malloc((len+1)*sizeof(wchar_t)); // this will waste a little bit of memory
	int i = 0, j = 0;
	while (i < len) {
		if (iswalpha(data[i])) {
			clean[j] = towlower(data[i]);
			++j;
		}
		++i;
	}

	if (j == 0) { free(clean); return NULL; }

	clean[j] = L'\0';
	return clean;
}

bool wstr_contains(const wchar_t* in, const wchar_t* pattern) {
	return wcsstr(in, pattern) != NULL;
}

bool str_contains(const char* in, const char* pattern) {
	return strstr(in, pattern) != NULL;
}


wchar_t *convert_to_wchar(const char* arg, long num_chars) {

	wchar_t *wc = malloc(num_chars * sizeof(wchar_t)); // will waste some memory though

	mbstate_t state;
	memset(&state, 0, sizeof(state));

//	printf("Using locale %s.\n", setlocale( LC_CTYPE, "" ));

	size_t result;
	result = mbsrtowcs(wc, &arg, num_chars, &state);

	if (result == (size_t)-1) {
	       	fputs("convert_to_wchar: encoding error X(", stderr);
		return NULL;
	}

	return wc;
}

char *convert_to_multibyte(const wchar_t* arg, long num_chars) {

	size_t buffer_size = num_chars * sizeof(wchar_t);
	char *mb = malloc(buffer_size); // will waste some memory though

	mbstate_t state;
	memset(&state, 0, sizeof(state));

	size_t result;
	result = wcsrtombs(mb, &arg, buffer_size, &state);
	if (result == (size_t)-1) {
	       	fputs("convert_to_multibyte: encoding error X(", stderr);
		free(mb);
		return NULL;
	}

	mb[buffer_size-1] = '\0';

	return mb;
}

int str_hasanyof(const wchar_t* str, const wchar_t* chars) {
	const wchar_t *c = &chars[0];

	while (*c != L'\0') {
		if (wcschr(str, *c)) { return 1; }
	       	++c;
	}

	return 0;
}


wchar_t *capitalize_first_nodup(wchar_t *str) {
	str[0] = toupper(str[0]);
	return str;
}

wchar_t *capitalize_first_dup(wchar_t *str) {
	wchar_t *dup = wcsdup(str);
	dup[0] = toupper(dup[0]);

	return dup;
}

char* const* tokenize(const char* input, const char *delims, long *num_tokens_out) {
	char *dup = strdup(input);
	char *endptr;

	char *token = strtok_r(dup, delims, &endptr);
	long num_tokens = 0;
	size_t size = 16; // just guessing
	char **ret = malloc(size*sizeof(char*));

	while (token) {
		if (num_tokens >= size) {
			size *= 2;
			ret = realloc(ret, size*sizeof(char*));
		}
		ret[num_tokens] = strdup(token);
		++num_tokens;
		token = strtok_r(NULL, delims, &endptr);
	}

	if (num_tokens < size) {
		// shrink
		ret = realloc(ret, num_tokens*sizeof(char*));
	}

	*num_tokens_out = num_tokens;
	free(dup);

	return ret;
}
