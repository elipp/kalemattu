#ifndef POEM_H
#define POEM_H

#include <wchar.h>

#include "types.h"

poem_t generate_poem(kstate_t *state);
void print_as_latex_document(const wchar_t* poem, const wchar_t *poetname);
void poem_print(const poem_t *poem);
void poem_print_LaTeX(const poem_t *poem);
char *poem_print_to_buffer(const poem_t *poem, int *len);
void poem_free(poem_t *poem);

#endif
