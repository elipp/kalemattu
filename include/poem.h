#ifndef POEM_H
#define POEM_H

#include <wchar.h>

#include "types.h"

wchar_t *generate_poem(kstate_t *state);
void print_as_latex_document(const wchar_t* poem, const wchar_t *poetname);

#endif
