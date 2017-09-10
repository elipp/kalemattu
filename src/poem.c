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

	word_t w = synth_get_word(num_syllables);
	wcscpy(buffer, w.chars);
	word_destroy(&w);

	return 1;

	if (num_syllables == 1) {
		wcscat(buffer, get_single_syllable_word()->chars);
		return 1;
	}

	SYLLABLE_SOURCE_FUNC SYLSOURCE = state->synth_enabled ? &synth_get_syllable : &dict_get_random_syllable_any;

	const char *sylp = NULL;
	if (state->synth_enabled) {
		sylp = synth_get_sylp(num_syllables);
		printf("sylp: %s, num_syllables: %d\n", sylp, num_syllables);
	}
	
	if (state->rules_apply) {
		make_valid_word(buffer, num_syllables, SYLSOURCE, sylp);
	}
	else {
		make_any_word(buffer, num_syllables, SYLSOURCE);
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

static verse_t generate_random_verse(long num_words, bool last_verse, const kstate_t *state, foot_t *foot) {

	verse_t verse;

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

	verse.verse = wcsdup(new_verse);
	verse.length = wcslen(new_verse);

	return verse;
	
}

static void stanza_free(stanza_t *s) {
	for (int i = 0; i < s->num_verses; ++i) {
		free(s->verses[i].verse);
	}

	free(s->verses);
}

static stanza_t generate_random_stanza(long num_verses, const kstate_t *state) {

	stanza_t s;
	s.verses = malloc(num_verses*sizeof(verse_t));
	s.num_verses = num_verses;

	for (int i = 0; i < num_verses; ++i) {
		s.verses[i] = generate_random_verse(4, i == num_verses - 1, state, NULL);
	}

	return s;

}

poem_t generate_poem(const kstate_t *state) {

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
	poem.stanzas = malloc(poem.num_stanzas * sizeof(stanza_t));

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

static void poem_print_LaTeX(const poem_t *poem) {
static const char *pre = "\\poemtitle{%ls}\n\\settowidth{\\versewidth}{levaton, lsitan kylpaa ranjoskan asdf}\n\\begin{verse}[\\versewidth]\n";
static const char *post = "\\end{verse}\n\\newpage\n\n";
	printf(pre, poem->title);

	for (int i = 0; i < poem->num_stanzas; ++i) {
		wchar_t *stanza = get_stanza(&poem->stanzas[i], POEM_FORMAT_LATEX);
		printf("%ls\n", stanza);
		free(stanza);
	}

	puts(post);
}

void print_as_latex_document(const poem_t* poem, const wchar_t *poetname) {
	print_latex_preamble();
	print_latex_title_page(poetname);
	poem_print_LaTeX(poem);
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

void poem_print(const poem_t *poem, int format) {

	switch (format) {
		case POEM_FORMAT_VANILLA:
		case POEM_FORMAT_IRC:
			printf("%ls\n\n", poem->title);
			break;
		case POEM_FORMAT_HTML:
			printf("<h2>%ls</h2>\r\n", poem->title);
			break;
		case POEM_FORMAT_LATEX:
			poem_print_LaTeX(poem);
			return;
		default:
			printf("poem_print: invalid format %d\n", format);
			return;

	}

	for (int i = 0; i < poem->num_stanzas; ++i) {
		wchar_t *stanza = get_stanza(&poem->stanzas[i], format);
		printf("%ls\n", stanza);
		free(stanza);
	}

	printf("\n");

}

char* poem_print_to_fcgi_buffer(const poem_t *poem, int *len) {
#define BUFFER_SIZE_DEFAULT 8096
	int buf_size = BUFFER_SIZE_DEFAULT;
	char *buffer = malloc(buf_size); 
	buffer[0] = '\0';

	int offset = sprintf(buffer, "<h3>%ls</h3>\n", poem->title);

	for (int i = 0; i < poem->num_stanzas; ++i) {
		if (offset + 512 > buf_size) {
			buf_size *= 2;
			buffer = realloc(buffer, buf_size);
		}
		wchar_t *stanza = get_stanza(&poem->stanzas[i], POEM_FORMAT_HTML);
		offset += sprintf(buffer + offset, "%ls", stanza);
		free(stanza);
	}

	offset += sprintf(buffer + offset, "\r\n");
	*len = offset;

	return buffer;

}



void poem_free(poem_t *poem) {
	free(poem->title);
	for (int i = 0; i < poem->num_stanzas; ++i) {
		stanza_free(&poem->stanzas[i]);
	}

	free(poem->stanzas);
}

static long get_stanza_length(const stanza_t *s) {
	long len = 0;
	for (int i = 0; i < s->num_verses; ++i) {
		len += s->verses[i].length;
	}

	return len;
}

wchar_t *get_stanza(const stanza_t *s, int format) {
	long total_len = get_stanza_length(s);
	long bufsize = total_len + 16*s->num_verses;
	wchar_t *stanza = malloc(bufsize*sizeof(wchar_t)); // the 8*s->num_verses part is padding for \r\n<br></p> etc :D
	stanza[0] = L'\0';

	wchar_t *fmt = NULL;

	switch(format) {
		case POEM_FORMAT_VANILLA:
			fmt = L"\n";
			break;
		case POEM_FORMAT_IRC:
			fmt = L"\n";
			break;
		case POEM_FORMAT_LATEX:
			fmt = L" \\\\ \n";
			break;
		case POEM_FORMAT_HTML:
			fmt = L"<br>\r\n";
			break;
		default:
			fmt = L"\n";
			break;
	}

	if (format == POEM_FORMAT_HTML) {
		wcscat(stanza, L"<p> ");
	}

	for (int i = 0; i < s->num_verses; ++i) {
		wcscat(stanza, s->verses[i].verse);
		wcscat(stanza, fmt);
	}

	if (format == POEM_FORMAT_HTML) {
		wcscat(stanza, L" </p>");
	}



	return stanza;

}
