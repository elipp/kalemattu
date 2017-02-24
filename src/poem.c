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

static wchar_t *construct_random_word(long max_syllables, bool rules_apply) {

	wchar_t new_word[256];
	new_word[0] = L'\0';

	int num_syllables = gauss_noise_with_limit(2, 1, 1, 4); 

	if (num_syllables == 1) {
		return wcsdup(get_single_syllable_word()->chars);
	} 
	
	if (rules_apply) {
		make_valid_word(new_word, num_syllables);
	}
	else {
		make_any_word(new_word, num_syllables);
	}

	if (wcslen(new_word) < 2) {
		return construct_random_word(max_syllables, rules_apply);
	} 
	else {
		return wcsdup(new_word);
	}

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

static wchar_t *generate_random_verse(long num_words, bool last_verse, kstate_t *state, foot_t *foot) {
	wchar_t new_verse[2048];
	new_verse[0] = L'\0';

	for (int i = 0; i < num_words; ++i) {

		wchar_t *new_word = construct_random_word(4, state->rules_apply);
		wcscat(new_verse, new_word);
		free(new_word);

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

	int num_words_title = 4;
	int max_syllables = 4;

	wchar_t *title = capitalize_first_nodup(construct_random_word(max_syllables, state->rules_apply));

	for (int i = 1; i < num_words_title; ++i) {
		// crap
		wchar_t *word = construct_random_word(max_syllables, state->rules_apply);
		wchar_t *new_title = wstring_concat_with_delim(title, word, L" ");
		free(word);
		free(title);
		title = new_title;
	}

	poem.title = title;

	poem.num_stanzas = gauss_noise_with_limit(2, 1, 1, 4);
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

static wchar_t *generate_random_poetname() {

	wchar_t name[128];
	name[0] = L'\0';

	wchar_t *first_name = capitalize_first_nodup(construct_random_word(3, true));
	wchar_t *second_name = capitalize_first_nodup(construct_random_word(2, true));
	second_name[1] = L'\0';

	wchar_t *surname = capitalize_first_nodup(construct_random_word(5, true));

	wcscat(name, first_name);
	wcscat(name, L" ");
	wcscat(name, second_name);
	wcscat(name, L". ");
	wcscat(name, surname);

	free(first_name);
	free(second_name);
	free(surname);

	return wcsdup(name);

}

void poem_print(const poem_t *poem) {

	printf("%ls\n", poem->title);

	for (int i = 0; i < poem->num_stanzas; ++i) {
		printf("%ls\n", poem->stanzas[i]);
	}

	puts("\n");

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
