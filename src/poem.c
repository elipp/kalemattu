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

wchar_t *construct_random_word(dict_t *dict, long max_syllables, bool rules_apply) {

	wchar_t new_word[256];
	new_word[0] = L'\0';

	int num_syllables = gauss_noise_with_limit(2, 1, 1, 4); 

	if (num_syllables == 1) {
		while (1) {
			word_t *w = get_random_word(dict);
			double r = get_randomf();
			if (w->syllables.length == 1 || w->length <= 4 || (r < 0.20 && w->length <= 5)) {
				return wcsdup(w->chars);
			}
		}
	} else {
		
	}

	if (wcslen(new_word) < 2) {
		return construct_random_word(dict, max_syllables, rules_apply);
	} 
	else {
		return wcsdup(new_word);
	}

}


syl_t *get_random_syllable(sylvec_t *sv) {
	return &sv->syllables[get_random(0, sv->length - 1)];
}

syl_t *get_syllable_with_lclass(sylvec_t *sv, char length_class) {
	return &sv->syllables[0]; // TODO
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

		} else if (r < 0.18) {
			wcscat(buffer, L":");
		}

	}

	if (!last_word) {
		wcscat(buffer, L" ");
	}

	return 1;
}

wchar_t *generate_random_verse(dict_t *dict, long num_words, bool last_verse, kstate_t *state, foot_t *foot) {
	wchar_t new_verse[2048];
	new_verse[0] = L'\0';

	for (int i = 0; i < num_words; ++i) {

		wchar_t *new_word = construct_random_word(dict, 4, state->rules_apply);
		wcscat(new_verse, new_word);
		free(new_word);

		bool last_word = i >= num_words - 1;
		add_punctuation(new_verse, last_verse, last_word);

	}

	return wcsdup(new_verse);
	
}

wchar_t *generate_random_stanza(dict_t *dict, long num_verses, kstate_t *state) {

	wchar_t new_stanza[4096];
	memset(new_stanza, 0, sizeof(new_stanza));
	for (int i = 0; i < num_verses; ++i) {
		wcscat(new_stanza, L"\n");
		wchar_t *new_verse = generate_random_verse(dict, 4, false, state, NULL);
		wcscat(new_stanza, new_verse);

		if (state->LaTeX_output) {
			wcscat(new_stanza, L" \\\\");
		}

		free(new_verse);
		i = i + 1;
	}

	if (state->LaTeX_output) { 
		wcscat(new_stanza, L"!\n\n");
	}

	return wcsdup(new_stanza);

}

wchar_t *generate_poem(dict_t *dict, kstate_t *state) {

	int num_words_title = 4;
	int max_syllables = 4;

	wchar_t *title = capitalize_first_nodup(construct_random_word(dict, max_syllables, state->rules_apply));

	for (int i = 1; i < num_words_title; ++i) {
		// crap
		wchar_t *new_title = wstring_concat_with_delim(title, construct_random_word(dict, max_syllables, state->rules_apply), L" ");
		free(title);
		title = new_title;
	}

	printf("title: %ls\n", title);

	wchar_t poem[8096];
	poem[0] = L'\0';

	if (state->LaTeX_output) {
		wcscat(poem, L"\\poemtitle{");
		wcscat(poem, title);
		wcscat(poem, L"}\n");
		wcscat(poem, L"\\settowidth{\\versewidth}{levaton, Lsitän kylpää ranjoskan asdf}\n");
		wcscat(poem, L"\\begin{verse}[\\versewidth]\n");
	} else {
		wcscat(poem, title);
		wcscat(poem, L"\n");
	}

	int num_stanzas = gauss_noise_with_limit(3, 1, 1, 4);

	for (int i = 0; i < num_stanzas; ++i) {

		int num_verses = gauss_noise_with_limit(4, 1, 1, 4);
		wchar_t *new_stanza = generate_random_stanza(dict, num_verses, state);

		wcscat(poem, new_stanza);
		free(new_stanza);
		wcscat(poem, L"\n");
	}

	if (state->LaTeX_output) {
		wcscat(poem, L"\\end{verse}\n");
		wcscat(poem, L"\\newpage\n");
	}

	return wcsdup(poem);
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

void print_as_latex_document(const char* poem, const char *poetname) {
	print_latex_preamble();
	print_latex_title_page(poetname);
	printf("%s", poem);
	printf("\\end{document}");

}

wchar_t *generate_random_poetname(dict_t *dict) {

	wchar_t name[128];
	name[0] = L'\0';

	wchar_t *first_name = capitalize_first_nodup(construct_random_word(dict, 3, true));
	wchar_t *second_name = capitalize_first_nodup(construct_random_word(dict, 2, true));
	second_name[1] = L'\0';

	wchar_t *surname = capitalize_first_nodup(construct_random_word(dict, 5, true));

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

