#ifndef POEM_H
#define POEM_H

#include <wchar.h>

#include "types.h"

poem_t generate_poem(const kstate_t *state);
void print_as_latex_document(const poem_t* poem, const wchar_t *poetname);
void poem_print(const poem_t *poem, int stanza_format);
char *poem_print_to_fcgi_buffer(const poem_t *poem, int *len);
void poem_free(poem_t *poem);

enum {
	POEM_FORMAT_VANILLA,
	POEM_FORMAT_IRC,
	POEM_FORMAT_HTML,
	POEM_FORMAT_LATEX,
};

wchar_t *get_stanza(const stanza_t *s, int format);

#endif
