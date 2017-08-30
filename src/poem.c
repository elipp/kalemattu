#include <stdbool.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "poem.h"
#include "dict.h"
#include "distributions.h"
#include "stringutil.h"
#include "aesthetics.h"
#include "synth.h"

static const word_t *get_single_syllable_word() {

	const word_t *w = dict_get_random_word();
	double r = get_randomf();

	while (1) {
		if (w->syllables.length == 1 || w->length <= 4 || (r < 0.20 && w->length <= 5)) {
			break;
		}
		w = dict_get_random_word();
	}
	return w;

}

static int construct_random_word(long max_syllables, const kstate_t *state, wchar_t *buffer) {

	buffer[0] = L'\0';

	int num_syllables = gauss_noise_with_limit(2, 1, 1, 4); 

	if (num_syllables == 1) {
		wcscat(buffer, get_single_syllable_word()->chars);
		return 1;
	} 
	
	if (state->rules_apply) {
		make_valid_word(buffer, num_syllables, state->synth_enabled ? &synth_get_syllable : &dict_get_random_syllable_any);
	}
	else {
		make_any_word(buffer, num_syllables);
	}

	if (wcslen(buffer) < 2) {
		return construct_random_word(max_syllables, state, buffer);
	} 

	return 1;

}

static int add_punctuation(wchar_t *buffer, bool last_verse, bool last_word) {
	double r = get_randomf();

	if (last_verse && last_word) {
		if (r < 0.08) {
			wcscat(buffer, L"!");
		}
		else if (r < 0.15) {
			wcscat(buffer, L"?");
		}
		else if (r < 0.25) {
			wcscat(buffer, L".");
		}

	} else {
		if (r < 0.02) {
			wcscat(buffer, L";");
		} 
		else if (r < 0.15) {
			wcscat(buffer, L",");
		} 
		else if (r < 0.18) {
			wcscat(buffer, L":");
		}

	}

	if (!last_word) {
		wcscat(buffer, L" ");
	}

	return 1;
}

static wchar_t *generate_random_verse(long num_words, bool last_verse, const kstate_t *state, foot_t *foot) {
	wchar_t new_verse[2048];
	new_verse[0] = L'\0';

	for (int i = 0; i < num_words; ++i) {
		wchar_t wordbuf[256];
		wordbuf[0] = L'\0';

		construct_random_word(4, state, wordbuf);
		wcscat(new_verse, wordbuf);

		bool last_word = i == num_words - 1;
		add_punctuation(new_verse, last_verse, last_word);

	}

	return wcsdup(new_verse);
	
}

static wchar_t *generate_random_stanza(long num_verses, kstate_t *state) {

	wchar_t new_stanza[4096];
	memset(new_stanza, 0, sizeof(new_stanza));
	for (int i = 0; i < num_verses; ++i) {
		wcscat(new_stanza, L"\n");
		wchar_t *new_verse = generate_random_verse(4, i == num_verses - 1, state, NULL);
		wcscat(new_stanza, new_verse);

		if (state->LaTeX_output) {
			wcscat(new_stanza, L" \\\\");
		}

		free(new_verse);
	}

	if (state->LaTeX_output) { 
		wcscat(new_stanza, L"!\n\n");
	}

	return wcsdup(new_stanza);

}

poem_t generate_poem(kstate_t *state) {

	poem_t poem;
	memset(&poem, 0, sizeof(poem));

	int num_words_title = gauss_noise_with_limit(1, 1, 1, 4);
	int max_syllables = 4;

	wchar_t wordbuf[256];
	wordbuf[0] = L'\0';

	wchar_t *title = malloc(256*sizeof(wchar_t));
	title[0] = L'\0';

	for (int i = 0; i < num_words_title; ++i) {
		construct_random_word(max_syllables, state, wordbuf);
		wcscat(title, wordbuf);
		wcscat(title, L" ");
	}

	poem.title = capitalize_first_nodup(title);

	poem.num_stanzas = gauss_noise_with_limit(2, 0.60, 1, 3);
	poem.stanzas = malloc(poem.num_stanzas * sizeof(wchar_t*));

	for (int i = 0; i < poem.num_stanzas; ++i) {
		int num_verses = gauss_noise_with_limit(4, 1, 1, 4);
		poem.stanzas[i] = generate_random_stanza(num_verses, state);
	}

	return poem;
}


static void print_latex_preamble() {

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

	printf("%s", LATEX_PREAMBLE);
}

static void print_latex_title_page(const wchar_t* poetname) {

	static const char *LATEX_TITLEPAGE_FMT = 
"\\begin{titlepage}\n"
"\\centering\n"
"{\\fontsize{45}{50}\\selectfont %ls \\par}\n"
"\\vspace{4cm}\n"
"\\sectionlinetwo{black}{7}\n"
"\\vspace{5cm}\n"
"{\\fontsize{35}{60}\\selectfont \\itshape Runoja\\par}\n"
"\\end{titlepage}";

	printf(LATEX_TITLEPAGE_FMT, poetname);

}

void print_as_latex_document(const wchar_t* poem, const wchar_t *poetname) {
	print_latex_preamble();
	print_latex_title_page(poetname);
	printf("%ls", poem);
	printf("\\end{document}");

}

//static wchar_t *generate_random_poetname() {
//
//	wchar_t name[256];
//	name[0] = L'\0';
//
//	wchar_t wordbuf[256];
//	wordbuf[0] = L'\0';
//
//	// first name
//	construct_random_word(3, true, wordbuf);
//	wcscat(name, capitalize_first_nodup(wordbuf));
//	wcscat(name, L" ");
//
//	// second name
//	construct_random_word(2, true, wordbuf);
//	wordbuf[1] = L'.';
//	wordbuf[2] = L'\0';
//	wcscat(name, capitalize_first_nodup(wordbuf));
//	wcscat(name, L" ");
//
//	construct_random_word(5, true, wordbuf);
//	wcscat(name, capitalize_first_nodup(wordbuf));
//
//	return wcsdup(name);
//
//}

void poem_print(const poem_t *poem) {


	printf("%ls\n", poem->title);

	for (int i = 0; i < poem->num_stanzas; ++i) {
		printf("%ls\n", poem->stanzas[i]);
	}

	printf("\n");

}

char* poem_print_to_buffer(const poem_t *poem, int *len) {
#define BUFFER_SIZE_DEFAULT 8096
	int buf_size = BUFFER_SIZE_DEFAULT;
	char *buffer = malloc(buf_size); 

	int offset = sprintf(buffer, "%ls\n", poem->title);

	for (int i = 0; i < poem->num_stanzas; ++i) {
		if (offset + 512 > buf_size) {
			buf_size *= 2;
			buffer = realloc(buffer, buf_size);
		}
		offset += sprintf(buffer + offset, "%ls\n", poem->stanzas[i]);
	}

	offset += sprintf(buffer + offset, "\n");
	*len = offset;

	return buffer;

}

void poem_print_LaTeX(const poem_t *poem) {
static const char *pre = "\\poemtitle{%ls}\n\\settowidth{\\versewidth}{levaton, lsitan kylpaa ranjoskan asdf}\n\\begin{verse}[\\versewidth]\n";
static const char *post = "\\end{verse}\n\\newpage\n\n";
	printf(pre, poem->title);

	for (int i = 0; i < poem->num_stanzas; ++i) {
		printf("%ls\n", poem->stanzas[i]);
	}

	puts(post);
}

void poem_free(poem_t *poem) {
	free(poem->title);
	for (int i = 0; i < poem->num_stanzas; ++i) {
		free(poem->stanzas[i]);
	}
	free(poem->stanzas);
}
