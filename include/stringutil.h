#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <wchar.h>
#include <stdbool.h>

char *get_substring(const char* input, size_t offset, size_t n);
wchar_t *get_subwstring(const wchar_t *input, size_t offset, size_t n);

char *string_concat(const char* str1, const char* str2);
wchar_t *wstring_concat(const wchar_t* str1, const wchar_t* str2);

char *string_concat_with_delim(const char* str1, const char* str2, const char* delim_between); 
wchar_t *wstring_concat_with_delim(const wchar_t* str1, const wchar_t* str2, const wchar_t* delim_between);

char *clean_string(const char* data);
wchar_t *clean_wstring(const wchar_t* data);

bool wstr_contains(const wchar_t* in, const wchar_t* pattern);

bool str_contains(const char* in, const char* pattern);

wchar_t *convert_to_wchar(const char* arg, long num_chars);
char *convert_to_multibyte(const wchar_t* arg, long num_chars);

int str_hasanyof(const wchar_t* str, const wchar_t* chars);

wchar_t *capitalize_first_nodup(wchar_t *str);
wchar_t *capitalize_first_dup(wchar_t *str);

char* const* tokenize(const char* input, const char *delims, long *num_tokens_out);

#endif
