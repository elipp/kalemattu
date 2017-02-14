#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

typedef struct strvec_t {
	char **strs;
	int length;
	int capacity;
} strvec_t;

typedef struct vcp_t {
	const char* pattern;
	char length_class;
} vcp_t;

const vcp_t *find_longest_vc_match(const char* vc, long offset);

strvec_t strvec_create() {
	strvec_t s;
	memset(&s, 0, sizeof(s));
	return s;
}

int strvec_push(strvec_t* vec, const char* str) {
	if (vec->length < 1) {
		vec->capacity = 2;
		vec->strs = malloc(vec->capacity*sizeof(char*));
		vec->length = 1;
	}
	else if (vec->length + 1 > vec->capacity) {
		vec->capacity *= 2;
		vec->strs = realloc(vec->strs, vec->capacity*sizeof(char*));
		vec->length += 1;
	}

	vec->strs[vec->length - 1] = strdup(str);

	return 1;
}

typedef struct kstate_t {
	unsigned int numeric_seed;
	int LaTeX_output;
	int rules_apply;
} kstate_t;

struct meter_t {

};

static const char *vowels = "aeiouyäöå";

char vc_map(char c) {
	if (isalpha(c)) {
		if (strchr(vowels, c)) {
			return 'V';
		}
		else return 'C';
	}
	else return '?';
}


// wikipedia:
// Suomessa on 10 perustavaa tavutyyppiä, jos konsonanttiyhtymillä alkavia tavuja
// ei lasketa mukaan. Tavun ytimessä on lyhyt vokaali (V) tai pitkä vokaali tai
// diftongi (VV). Ne voivat muodostaa tavun yksinkin, mutta niiden edessä voi
// olla yksi konsonantti (C) ja jäljessä yksi tai kaksi konsonanttia. Perustavat
// tavutyypit ovat siis CV (ta.lo), CVC (tas.ku), CVV (saa.ri), CVVC (viet.to),
// VC (es.te), V (o.sa), VV (au.to), CVCC (kilt.ti), VVC (aal.to) ja VCC (ark.ku).
// Kahdella konsonantilla alkavia tavutyyppejä ovat CCV (pro.sentti), CCVC (pris.ma),
// CCVV (kruu.nu), CCVVC (staat.tinen) ja CCVCC (prons.si). Harvinaisempia ovat
// kolmella konsonantilla alkavat CCCV (stra.tegia), CCCVC (stres.si) ja CCCVCC
// (sprint.teri).[84]

char *get_substring(const char* input, size_t offset, size_t n) {
	size_t len = strlen(input);
	char *r = malloc((len-offset) + n + 1);
	strncpy(r, input + offset, n);
	return r;
}

//fn get_vc_pattern(word: &str) -> String {
//	let mut p = String::new();
//	for c in word.chars() { p.push(vc_map(c)); }
//	return p;
//}

char *get_vc_pattern(const char* input) {
	size_t len = strlen(input);
	char *vc = malloc(len + 1);
	for (int i = 0; i < len; ++i) {
		vc[i] = vc_map(input[i]);
	}
	return vc;
}

//fn get_vc_pattern_grep(word: &str) -> String {
//	let mut p = String::new();

//	p.push('^');
//	p.push_str(&get_vc_pattern(word));
//	p.push('$');

//	return p;

//}

char *get_vc_pattern_grep(const char* input) {
	size_t len = strlen(input);
	char *vc = malloc(len+1 + 2);
	vc[0] = '^';
	strncpy(vc + 1, input, len);
	vc[len] = '$';
	vc[len+1] = '\0';

	return vc;
}


typedef struct syl_t {
	char *chars;
	long length;
	char length_class;
} syl_t;

syl_t syl_create(const char* syl, char length_class) {
	syl_t s;
	s.chars = strdup(syl);
	s.length = strlen(syl);
	s.length_class = length_class;

	return s;
}

typedef struct sylvec_t {
	syl_t *syllables;
	long length;
	size_t capacity;
} sylvec_t;

sylvec_t sylvec_create() {
	sylvec_t s;
	memset(&s, 0, sizeof(s));

	return s;
}

int sylvec_contains(sylvec_t *s, const char *str) {
	for (int i = 0; i < s->length; ++i) {
		if (strcmp(s->syllables[i].chars, str) == 0) return 1;
	}

	return 0;
}

char *string_concat(const char* str1, const char* str2) {
	size_t l1 = strlen(str1);
	size_t l2 = strlen(str2);

	char *buf = malloc(l1 + l2 + 1);

	strcat(buf, str1);
	strcat(buf, str2);

	return buf;

}

char *string_concat_with_delim(const char* str1, const char* str2, const char* delim_between) {
	size_t l1 = strlen(str1);
	size_t l2 = strlen(str2);
	size_t ld = strlen(delim_between);

	char *buf = malloc(l1 + l2 + ld + 1);

	strcat(buf, str1);
	strcat(buf, delim_between);
	strcat(buf, str2);

	return buf;
}


int sylvec_pushsyl(sylvec_t *s, const syl_t *syl) {
	if (s->length < 1) {
		s->capacity = 2;
		s->syllables = malloc(s->capacity*sizeof(syl_t));
		s->length = 1;
	}
	else if (s->length + 1 > s->capacity) {
		s->capacity *= 2;
		s->syllables = realloc(s->syllables, s->capacity*sizeof(syl_t));
		s->length += 1;
	}

	s->syllables[s->length - 1] = *syl;

	return 1;

}

int sylvec_pushstr(sylvec_t *s, const char *syl) {
	if (s->length < 1) {
		s->capacity = 2;
		s->syllables = malloc(s->capacity*sizeof(syl_t));
		s->length = 1;
	}
	else if (s->length + 1 > s->capacity) {
		s->capacity *= 2;
		s->syllables = realloc(s->syllables, s->capacity*sizeof(syl_t));
		s->length += 1;
	}

	char *vc = get_vc_pattern(syl);
	const vcp_t *L = find_longest_vc_match(vc, 0);
	free(vc);
	syl_t new_syl = syl_create(syl, L->length_class);
	s->syllables[s->length - 1] = new_syl;

	return 1;

}


int sylvec_push_slice(sylvec_t *s, const sylvec_t *in) {
	for (long i = 0; i < in->length; ++i) {
		sylvec_pushsyl(s, &in->syllables[i]);
	}

	return 1;
}

char *sylvec_get_word(sylvec_t *s) {
	long size = 0;
	for (int i = 0; i < s->length; ++i) {
		size += s->syllables[i].length;
	}

	char *buf = malloc(size + 1);
	
	long offset = 0;
	for (int i = 0; i < s->length; ++i) {
		strcpy(buf + offset, s->syllables[i].chars);
		offset += s->syllables[i].length;
	}

	buf[size] = '\0';

	return buf;
}

typedef struct word_t {
	char *chars;
	long length;
	sylvec_t syllables;
} word_t;

void word_syllabify(word_t *w);

word_t word_create(const char* chars) {
	word_t w;
	w.chars = strdup(chars);
	w.length = strlen(chars);
	word_syllabify(&w);

	return w;
}

int word_push_syllable(word_t *w, const char *s) {
	sylvec_pushstr(&w->syllables, s);
	return 1;
}

typedef struct dict_t {
	word_t *words;
	long num_words;
} dict_t;

dict_t dict_create(word_t *words, long num_words) {
	dict_t d;
	d.words = words;
	d.num_words = num_words;
	
	return d;
}

typedef struct foot_t {
    char **spats; // "syllable patterns" :D
    long num_spats;
} foot_t;

char *clean_string(const char* data) {
	char *dup = strdup(data);
	int i = 0, j = 0;
	while (i < strlen(dup)) {
		if (isalpha(dup[i])) {
			dup[j] = tolower(dup[i]);
			++j;
		}
		++i;
	}

	return dup;
}


static const vcp_t VC_PATTERNS[] = {
    {"V", '1'},
    {"VC",'1'},
    {"VV", '2' },
    {"VVC", '2' },
    {"VCC", '1' },
    {"CV", '1' },
    {"CVC", '1' },
    {"CVV", '2' },
    {"CVVC", '2' },
    {"CVCC", '1' },
    {"CCV", '1' },
    {"CCVC", '1' },
    {"CCVV", '2' },
    {"CCVVC", '2' },
    {"CCVCC", '1' },
    {"CCCVC", '1' },
    {"CCCVCC", '1' },
    { NULL, '0'}
};

const vcp_t *find_longest_vc_match(const char* vc, long offset) {
	static const vcp_t error_pat = {"", '0'};

	const vcp_t *longest = &error_pat;
	long vclen = strlen(vc);
	const vcp_t *current = &VC_PATTERNS[0];
	while (current->pattern != NULL) {
	
//		let mut full_match = true;
		bool full_match = true;
		
		//let plen = pat.chars().count();
		size_t plen = strlen(current->pattern);

		if ((vclen - offset) >= plen) {
//			for c in pat.chars() {
//				if get_nth(vc, i) != c  {
//					full_match = false;
//					break;
//				}
//				i = i+1;
//			}
			if (strncmp(current->pattern, vc, plen) == 0) {
				full_match = false;
			}

			if (full_match) {
				if ((vclen - offset) > plen) {
//					if (get_nth(pat, plen - 1) == 'C') {
//						// then the next letter can't be a vowel
//						if get_nth(vc, plen + offset) == 'V'{
//							//println!("nextvowel check for {} at offset {} failed!", p, offset);
//							full_match = false;
//						}
//					}
					if (current->pattern[plen-1] == 'C') {
						if (vc[plen+offset] == 'V') {
							printf("warning: nextvowel check for %s failed at offset %lu!\n", current->pattern, offset);
							full_match = false;
						}
					}
				}

//				if full_match && longest.P.chars().count() < plen {
//					longest = p;
//				//    println!("new longest matching pattern: for {} -> {}", &vc[offset..], p);
//				}

				if (full_match && strlen(longest->pattern) < plen) {
					longest = current;
				}
			}
		}

		++current;
	}

//	println!("syllable: {}, pattern: {}, length class: {}", vc, longest.P, longest.L);

	return longest;

}

//fn get_substring(word: &str, offset: usize, n: usize) -> String {
 //   let mut s = String::new();
  //  let mut i = 0;
   // for c in word.chars().skip(offset) {
    //    if i >= n { break; }
     //   s.push(c);
     //   i = i+1;
    //}

//    return s;
//}

static const char* DIPHTHONGS[] = {
"yi", "öi", "äi", "ui", "oi",
"ai", "äy", "au", "yö", "öy",
"uo", "ou", "ie", "ei", "eu",
"iu", "ey", "iy", NULL
};

static const char* DOUBLEVOWELS[] = {
"aa", "ee", "ii", "oo", "uu", "yy", "ää", "öö", NULL
};

static const char* NON_DIPHTHONGS[] = {
"ae", "ao", "ea", "eo", "ia",
"io", "iä", "oa", "oe", "ua",
"ue", "ye", "yä", "äe", "äö",
"öä", "eä", "iö", "eö", "öe",
"äa", "aä", "oö", "öo", "yu",
"uy", "ya", "yu", "äu", "uä",
"uö", "öu", "öa", "aö", NULL
};

//fn has_diphthong(syllable: &str) -> bool {
//        for n in DIPHTHONGS {
//                if syllable.contains(n) { return true; }
//        }
//        return false;
//}
//
bool has_diphthong(const char* syllable) {
	const char **d = &DIPHTHONGS[0];
	while (*d) {
		if (strstr(syllable, *d) != NULL) { return true; }
		++d;
	}
	return false;
}

//fn has_double_vowel(syllable: &str) -> bool {
//    for n in DOUBLEVOWELS {
//        if syllable.contains(n) { return true; }
//    }
//
//    return false;
//}
//
bool has_double_vowel(const char* syllable) {
	const char **s = &DOUBLEVOWELS[0];
	while (*s) {
		if (strstr(syllable, *s) != NULL) { return true; }
		++s;
	}

	return false;
}


//static ALLOWED_CCOMBOS: &'static [&'static str] =
//&[ "kk", "ll", "mm", "nn", "pp", "rr", "ss", "tt",
//"sd", "lk", "lt", "rt", "tr", "st", "tk", "mp"
//
//];
//

static const char* ALLOWED_CCOMBOS[] = {
"kk", "ll", "mm", "nn", "pp", "rr", "ss", "tt",
"sd", "lk", "lt", "rt", "tr", "st", "tk", "mp", NULL
};

static const char *FORBIDDEN_CCOMBOS[] = {
"nm", "mn", "sv", "vs", "kt", 
"tk", "sr", "sn", "tv", "sm", 
"ms", "tm", "tl", "nl", "tp", 
"pt", "tn", "np", "sl", "th", 
"td", "dt", "tf", "ln", "mt", 
"kn", "kh", "lr", "kp", "nr",
"ml", "mk", "km", "nv", "sh",
"ls", "hn", "tj", "sj", "pk", 
"rl", "kr", "mj", "kl", "kj",
"nj", "kv", "hs", "hl", "nh",
"pm", "mr", "tg", "mh", "hp",
"kd", "dk", "dl", "ld", "mv", 
"vm", "pr", "hh", "pn", "tr",
"ts", "ks", "md", "pj", "jp",
"kg", "pv", "ph", NULL

};

//fn has_forbidden_ccombos(word: &str) -> bool {
//	for c in FORBIDDEN_CCOMBOS {
//		if word.contains(c) { return true; }
//	}
//
//	return false;
//}
//

bool has_forbidden_ccombos(const char* input) {
	const char** c = &FORBIDDEN_CCOMBOS[0];

	while (*c) {
		if (strstr(input, *c) != NULL) { return true; }
		++c;
	}

	return false;
}

static const char FORBIDDEN_ENDCONSONANTS[] = {
"pkrmhvsl"
};

//fn has_forbidden_endconsonant(word: &str) -> bool {
//	for c in FORBIDDEN_ENDCONSONANTS {
//		let last_char = word.chars().last().unwrap();
//		if &last_char == c { return true; }
//	}
//
//	return false;
//}	
//

bool has_forbidden_endconsonant(const char *input) {
	char l = input[strlen(input)-1];
	size_t i = 0;

	while (i < sizeof(FORBIDDEN_ENDCONSONANTS)) {
		if (l == FORBIDDEN_ENDCONSONANTS[i]) { return true; }
		++i;
	}

	return false;
}


bool str_contains(const char* in, const char* pattern) {
	return strstr(in, pattern) != NULL;
}


//fn syllabify(word: &mut word_t) {

void word_syllabify(word_t *word) {

    //let mut offset = 0;
	size_t offset = 0;

//    let vc_pattern = get_vc_pattern(&word.chars); 
	char *vc_pattern = get_vc_pattern(word->chars);
//    let vclen = vc_pattern.chars().count();
	size_t vc_len = strlen(vc_pattern);

//    while offset < vclen {
	while (offset < vc_len) {
//    	let longest = find_longest_vc_match(&vc_pattern, offset);
		const vcp_t *longest = find_longest_vc_match(vc_pattern, offset);
		//let &pat = &longest.P;
		const char *pat = longest->pattern;
		//let plen = pat.chars().count();
		size_t plen = strlen(longest->pattern);

		if (plen < 1) {
	//          println!("(warning: syllabify() for word \"{}\" failed, no matches found)", word.chars);
	//	    word.syllables.push(word.chars.to_string());
		    break;
		}

		else {
			//    let new_syl = get_substring(&word.chars, offset, plen);
			char *new_syl = get_substring(word->chars, offset, plen);

			if (offset > 0 && str_contains(pat, "VV") && !has_diphthong(new_syl) && !has_double_vowel(new_syl)) {
				// need to split this syllable into two
				//			let vv_offset = pat.find("VV").unwrap();
				size_t vv_offset = strstr(pat, "VV") - pat;
				//			let new_syl_split1 = get_substring(&word.chars, offset, vv_offset+1);
				char *p1 = get_substring(word->chars, offset, vv_offset + 1);
				//			let new_syl_split2 = get_substring(&word.chars, offset+vv_offset+1, plen - new_syl_split1.chars().count());
				char *p2 = get_substring(word->chars, offset+vv_offset+1, plen - strlen(p1));


				//println!("split syllable {} into two: {}-{}", new_syl, &new_syl_split1, &new_syl_split2);
				word_push_syllable(word, p1);
				word_push_syllable(word, p2);


			} else {
				//			word.syllables.push(syl_t{chars: new_syl, length_class: longest.L});
				word_push_syllable(word, new_syl);
			}
		}
		
        offset = offset + plen;
    }

}

static long get_filesize(FILE *fp) {
	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);
	rewind(fp);

	return size;
}

static long word_count(const char* buf) {

	char *bufdup = strdup(buf);
	char *endptr;
	char *token = strtok_r(bufdup, " ", &endptr);
	long num_words = 0;

	while (token) {
		++num_words;
		token = strtok_r(NULL, " ", &endptr);
	}

	free(bufdup);
	
	return num_words;
}

static char *read_file_to_buffer(FILE *fp) {
	long filesize = get_filesize(fp);
	char *buf = malloc(filesize);
	fread(buf, 1, filesize, fp); 

	return buf;
}

word_t *construct_word_list(const char* buf, long num_words) {

	word_t *words = malloc(num_words * sizeof(word_t));

	char *bufdup = strdup(buf);
	char *endptr;
	char *token = strtok_r(bufdup, " ", &endptr);
	long i = 0;

	while (token) {
		token = strtok_r(NULL, " ", &endptr);
		words[i] = word_create(token);
		++i;
	}
	
	return words;

}

dict_t read_file_to_words(const char* filename) {
	dict_t d;
	memset(&d, 0, sizeof(d));

	FILE *fp = fopen(filename, "r");

	if (!fp) {
		fprintf(stderr, "error: Couldn't open file %s\n", filename);
	}
	char *buf = read_file_to_buffer(fp);
	fclose(fp);

	long wc = word_count(buf);
	word_t *words = construct_word_list(buf, wc);
	free(buf);

	d = dict_create(words, wc);

	return d;
	
}

//fn read_file_to_words(filename : &'static str) -> Vec<word_t> {
//
//    let mut f = match File::open(filename) {
//        Ok(file) => file,
//        Err(e) => {
//            // fallback in case of failure.
//            // you could log the error, panic, or do anything else.
//            // println("Opening file {} failed: {}", filename, e);
//
//            return Vec::new();
//        }
//    };
//
//    let mut s = String::new();
//    let asd = f.read_to_string(&mut s);
//
//    let mut words : Vec<word_t> = Vec::new();
//
//    for x in s.split_whitespace() {
//
//        let mut w = word_t { chars : clean_string(x), syllables : Vec::new() };
//
//        if w.chars != "" {
//            syllabify(&mut w);
//            words.push(w);
//        }
//
//    }
//    return words;
//}

//fn compile_list_of_syllables(words: &Vec<word_t>) -> Vec<syl_t> {
//    let mut syllables: Vec<syl_t> = Vec::new();
//
//    for w in words {
//        syllables.extend(w.syllables.clone());
//    }
//    return syllables;
//}

sylvec_t compile_list_of_syllables(dict_t *dict) {
	sylvec_t s;
	memset(&s, 0, sizeof(s));

	for (long i = 0; i < dict->num_words; ++i) {
		sylvec_push_slice(&s, &dict->words[i].syllables);
	}

	return s;
}

//fn get_random(rng: &mut StdRng, min: usize, max: usize) -> usize {
//	let k = ((max as f64)*rng.gen::<f64>() + (min as f64)) as usize;
//
//	return k;
//}
//

long get_random(long min, long max) {
	return rand() % max + min;
}

double get_randomf() {
	return (double)rand() / (double)RAND_MAX;
}


// hardcode these in C 


//fn get_random_with_distribution(rng: &mut StdRng, distr: &Vec<usize>) -> usize {
//
//	let r = rng.gen::<f64>();
//	let index = (r * (distr.len() as f64)) as usize;
//	let R = distr[index];
//
//	return R;
//}

//fn get_random_word<'a>(word_list: &'a Vec<word_t>, mut rng: &mut StdRng) -> &'a word_t {
//	let word = &word_list[get_random(rng, 0, word_list.len())];
//
//	return word;
//}
//

word_t *get_random_word(dict_t *dict) {
	return &dict->words[get_random(0, dict->num_words)];
}


//fn get_vowel_harmony_state(syllable: &str) -> usize {
//	let mut state: usize = 0;
//
//	if syllable.contains('a') || syllable.contains('o') || syllable.contains('u') {
//		state = state | 0x1;
//	} 
//
//	if syllable.contains('ä') || syllable.contains('ö') || syllable.contains('y') {
//		state = state | 0x2;
//	} 
//
////	println!("word: \"{}\", returning vharm: {}", syllable, state);
//	return state;
//}

static int str_hasanyof(const char* str, const char* chars) {
	const char *c = &chars[0];

	while (*c) {
		if (strchr(str, *c)) { return 1; }
	       	++c;
	}

	return 0;
}

int get_vowel_harmony_state(const char* syllable) {
	int state = 0;

	if (str_hasanyof(syllable, "aou")) state |= 0x1;
	if (str_hasanyof(syllable, "äöy")) state |= 0x2;

	return state;
}

//
//fn get_num_trailing_vowels(word: &str) -> usize {
//	let mut num = 0;
//	for c in word.chars().rev() {
//		if vc_map(c) == 'V' {
//			num = num+1;
//		}
//		else {
//			break;
//		}
//	}
//
//
//	return num;
//}

int get_num_trailing_vowels(const char *word) {
	int num = 0;
	size_t len = strlen(word);
	while (vc_map(word[len-num-1]) == 'V' && num < len) ++num;

	return num;

}

//fn get_num_beginning_vowels(word: &str) -> usize {
//	let mut num = 0;
//
//	for c in word.chars() {
//
//		if vc_map(c) == 'V' {
//			num = num+1;
//		}
//		else {
//			break;
//		}
//
//	}
//
//	return num;
//}

int get_num_beginning_vowels(const char *word) {
	int num = 0;
	size_t len = strlen(word);
	while (vc_map(word[num]) == 'V' && num < len) ++num;

	return num;
}

//fn get_first_consonant(syl: &str) -> char {
//
//	for c in syl.chars() {
//		if vc_map(c) == 'C' {
//			return c;
//		}
//	}
//
//	return '0';
//
//}

char get_first_consonant(const char *str) {
	size_t len = strlen(str);
	int i = 0;
	while (vc_map(str[i]) != 'C' && i < len) ++i;
	if (i == len) return '\0';
	else return str[i];
}

//static FORBIDDEN_VOWELENDINGS: &'static [&'static str] =
//&["ai", "ei", "ou", "ae", "au", "iu", "oe", "ue", "äy", "ii", "yy", "äi", "eu" ];

static const char *FORBIDDEN_VOWELENDINGS[] = {
"ai", "ei", "ou", "ae", "au", "iu", "oe", "ue", "äy", "ii", "yy", "äi", "eu", NULL
};

//fn ends_in_wrong_vowelcombo(word: &str) -> bool {
//	let vcp = get_vc_pattern_grep(&word);
//	if vcp.contains("VV$") {
//		let mut iter = word.chars().rev();
//		let last = iter.next().unwrap();
//		let second_last = iter.next().unwrap();
//
//		let mut lasttwo = String::new();
//		lasttwo.push(second_last);
//		lasttwo.push(last);
//
//		for ed in FORBIDDEN_VOWELENDINGS {
//			if ed == &lasttwo {
//				return true;
//			}
//
//		}
//	}
//
//	return false;
//}

bool ends_in_wrong_vowelcombo(const char *str) {
	const char* vcp = get_vc_pattern_grep(str);

	if (strstr("VV$", vcp)) {
		char *substr = get_substring(str, strlen(str) - 2, 2);
		const char **f = &FORBIDDEN_VOWELENDINGS[0];

		while (f) {
			if (strncmp(*f, substr, 2) == 0) { return true; }
			++f;
		}

		free(substr);
	}

	return false;
}

syl_t *get_random_syllable_from_word(word_t *w, bool ignore_last) {
	long r = get_random(0, ignore_last ? w->syllables.length - 1 : w->syllables.length);
	return &w->syllables.syllables[r];	
}

char *get_random_syllable_any(dict_t *dict, bool ignore_last) {
	word_t *w = get_random_word(dict);
	while (w->syllables.length == 1) {
		w = get_random_word(dict);
	}

	syl_t *s = get_random_syllable_from_word(w, ignore_last);

	return strdup(s->chars);
}

//fn construct_random_word<'a>(word_list: &'a Vec<word_t>, rng: &mut StdRng, max_syllables: usize, rules_apply: bool) -> String {
char *construct_random_word(dict_t *dict, long max_syllables, bool rules_apply) {

	char new_word[256]; // should be enough :D
	memset(new_word, 0, sizeof(new_word));

	int num_syllables = 3; // get random blah balh	

	if (num_syllables == 1) {
		while (1) {
			word_t *w = get_random_word(dict);
			double r = get_randomf();
			if (w->syllables.length > 1 || w->length <= 4 || (r < 0.20 && w->length <= 5)) {
				return strdup(w->chars);
			}
		}
	}

//	let mut new_syllables: Vec<String> = Vec::new();
	sylvec_t new_syllables = sylvec_create();
//	
//	let mut vharm_state = 0;
	int vharm_state = 0;
//	let mut prev_first_c = '0';
	char prev_first_c = '\0';
//
//	for n in 0..num_syllables {
	for (int n = 0; n < num_syllables; ++n) {
//		
//		let ignore_last = n == 0;
		bool ignore_last = (n == 0);
//		let mut syl = get_random_syllable_any(&word_list, rng, ignore_last);
		char *syl = get_random_syllable_any(dict, ignore_last);

//		let mut syl_vharm: usize = 0;
		int syl_vharm = 0;
//		
//		if rules_apply {
		if (rules_apply) {
			while (1) {
				syl_vharm = get_vowel_harmony_state(syl);
				char first_c = get_first_consonant(syl);
				char *concatd = string_concat(new_word, syl);

				if (syl_vharm > 0 && vharm_state != 0 && syl_vharm != vharm_state) {
					goto new_syllable;
				}
				else if (n > 0 && strlen(syl) < 2) {
					goto new_syllable;
				}
				else if (sylvec_contains(&new_syllables, syl)) {
					goto new_syllable;
				}
				else if (has_forbidden_ccombos(concatd)) {
					goto new_syllable;
				}
				else if (get_num_trailing_vowels(new_word) + get_num_beginning_vowels(syl) > 2) {
					goto new_syllable;
				}
				else if ((n == num_syllables - 1) && (has_forbidden_endconsonant(syl) || ends_in_wrong_vowelcombo(syl))) {
					goto new_syllable;
				}
				else if (first_c && first_c == prev_first_c) {
					goto new_syllable;
				}
				else { 
					free(concatd);
					prev_first_c = first_c;
					break;
				}

				new_syllable:
					syl = get_random_syllable_any(dict, ignore_last);

				free(concatd);
			
			}

			if (vharm_state == 0) {
				// we're still in "undefined vocal harmony" => only either 'e's or 'i's have been encountered
				if (syl_vharm > 0) {
					vharm_state = syl_vharm;
				}
			}
		}

		sylvec_pushstr(&new_syllables, syl);
		strcat(new_word, syl);

	}

	if (strlen(new_word) < 2) {
		return construct_random_word(dict, max_syllables, rules_apply);
	} 
	else {
		return strdup(new_word);
	}

}

//fn construct_word_with_foot<'a>(syllables: &'a Vec<syl_t>, rng: &mut StdRng, foot: &str) -> String {
// 	let mut new_word = String::new();
//
//	let num_syllables = foot.chars().count();
//
////	if num_syllables == 1 {
////		loop {
////			let w = get_random_word(word_list, rng);
////			let r = rng.gen::<f64>();
////
////			if w.syllables.len() == 1 || 
////			   w.chars.chars().count() <= 4 || 
////			   (r < 0.20 && w.chars.chars().count() <= 5) {
////
////			       	return w.chars.clone(); 
////			}
////		}
////	}
//
//	let mut new_syllables: Vec<String> = Vec::new();
//	
//	let mut vharm_state = 0;
//	let mut prev_first_c = '0';
//
//    for (n, LC) in foot.chars().enumerate() {
//
//        let ignore_last = n == 0;
//        let mut syl = get_syllable_with_lclass(&syllables, rng, LC);
//        let mut syl_vharm: usize = 0;
//
//        loop { 
//
//            syl_vharm = get_vowel_harmony_state(&syl); 
//            let first_c = get_first_consonant(&syl);
//
////            println!("word (so far): {}, desired lclass: {} syl: {}", new_word, LC, syl);
//
//            if syl_vharm > 0 && vharm_state != 0 && syl_vharm != vharm_state {
//                syl = get_syllable_with_lclass(&syllables, rng, LC);
//            } 
//            else if n > 0 && syl.chars().count() < 2 {
//                syl = get_syllable_with_lclass(&syllables, rng, LC);
//            }
//            else if new_syllables.contains(&syl) {
//                syl = get_syllable_with_lclass(&syllables, rng, LC);
//            }
//            else if has_forbidden_ccombos(&(new_word.clone() + &syl)) {
//                syl = get_syllable_with_lclass(&syllables, rng, LC);
//            }
//            else if get_num_trailing_vowels(&new_word) + get_num_beginning_vowels(&syl) > 2 {
//                syl = get_syllable_with_lclass(&syllables, rng, LC);
//            }
//            else if (n == num_syllables-1) && (has_forbidden_endconsonant(&syl) || ends_in_wrong_vowelcombo(&syl)) {
//                syl = get_syllable_with_lclass(&syllables, rng, LC);
//            } 
//            else if first_c != '0' && first_c == prev_first_c {
//                syl = get_syllable_with_lclass(&syllables, rng, LC);
//            }
//
//            else { 
//                prev_first_c = first_c;
//                break;
//            }
//
//        }
//
//        if vharm_state == 0 {
//            // we're still in "undefined vocal harmony" => only either 'e's or 'i's have been encountered
//            if syl_vharm > 0 {
//                vharm_state = syl_vharm;
//            }
//        }
//
//        new_syllables.push(syl.clone());
//        new_word.push_str(&syl);
//
//    }
//
////    println!("{}", new_word);
//
//	return new_word;
//
//}   

//fn get_random_syllable_from_word(word: &word_t, rng: &mut StdRng, ignore_last: bool) -> String {
//
//	let mut sindex: usize = word.syllables.len();
//	if ignore_last {
//		sindex -= 1;
//	}
//
//	let syl = word.syllables[get_random(rng, 0, sindex)].chars.clone();
//	
//	return syl;
//}

//fn get_random_syllable_any(word_list: &Vec<word_t>, rng: &mut StdRng, ignore_last: bool) -> String {
//	let mut word = get_random_word(word_list, rng);
//	loop {
//		if word.syllables.len() == 1 {
//			word = get_random_word(word_list, rng);
//		} else {
//			break;
//		}
//	}
//	let syl = get_random_syllable_from_word(&word, rng, ignore_last);
//
//	return syl;
//}


//fn get_random_syllable<'a>(syllables: &'a Vec<syl_t>, rng: &mut StdRng) -> &'a syl_t {
//    let syl = &syllables[get_random(rng, 0, syllables.len() - 1)];
//    return syl;
//}

syl_t *get_random_syllable(sylvec_t *sv) {
	return &sv->syllables[get_random(0, sv->length - 1)];
}

//fn get_syllable_with_lclass(syllables: &Vec<syl_t>, rng: &mut StdRng, length_class: char) -> String {
//
//	let mut syl = get_random_syllable(&syllables, rng);
//    loop {
//        if syl.length_class != length_class {
//            syl = get_random_syllable(&syllables, rng);
//   //         println!("{}, {} != {}", syl.chars, length_class, syl.length_class);
//        }
//        else {
//    //        println!("OK! {}, {} == {}", syl.chars, length_class, syl.length_class);
//            break;
//        }
//    }
//
//	return syl.chars.clone(); 
//
//}

syl_t *get_syllable_with_lclass(sylvec_t *sv, char length_class) {
	return &sv->syllables[0]; // TODO
}


//fn generate_random_verse<'a>(word_list: &'a Vec<word_t>, rng: &mut StdRng, num_words: usize, last_verse: bool, state: &kstate_t, foot: &str) -> String {
char *generate_random_verse(dict_t *dict, long num_words, bool last_verse, kstate_t *state, foot_t *foot) {
//	let mut new_verse = String::new();
	char new_verse[2048];
	memset(new_verse, 0, sizeof(new_verse));

//	println!("num_words: {}", num_words);

//	for j in 0..num_words {
	for (int j = 0; j < num_words; ++j) {

//		let new_word = construct_random_word(&word_list, rng, 4, state.rules_apply);
		char *new_word = construct_random_word(dict, 4, state->rules_apply);
		//new_verse.push_str(&new_word);
		strcat(new_verse, new_word);
		free(new_word);

//		let r = rng.gen::<f64>();
		double r = get_randomf();

		if (last_verse && j == num_words - 1) {
			if (r < 0.08) {
				strcat(new_verse, "!");
			}
			else if (r < 0.15) {
				strcat(new_verse, "?");
			}
			else if (r < 0.25) {
				strcat(new_verse, ".");
			}

		} else {
			if (r < 0.02) {
				strcat(new_verse, ";");
			} 
			else if (r < 0.15) {
				strcat(new_verse, ",");
		
			} else if (r < 0.18) {
				strcat(new_verse, ":");
			}

		}

		if (j < num_words - 1) {
			strcat(new_verse, " ");
		}

	}

	return strdup(new_verse);
	
}

//fn generate_verse_with_foot<'a>(syllables: &'a Vec<syl_t>, rng: &mut StdRng, state: &kstate_t, foot: &str) -> String {
//    let mut new_verse = String::new();
//
//    for w in foot.split("-") {
//        let num_syllables = w.chars().count();
//        let w = construct_word_with_foot(syllables, rng, &w);
//        new_verse = new_verse + &w + " ";
//    }
//
//    return new_verse;
//}

//fn generate_random_stanza<'a>(word_list: &'a Vec<word_t>, rng: &mut StdRng, num_verses: usize, state: &kstate_t) -> String {
char *generate_random_stanza(dict_t *dict, long num_verses, kstate_t *state) {

	char new_stanza[4096];
	memset(new_stanza, 0, sizeof(new_stanza));
//	let mut i = 0;
//    let mut foot: foot_t = foot_t { spats: Vec::new() };

//    let syllables = compile_list_of_syllables(word_list);
//    	sylvec_t s = compile_list_of_syllables(dict);

//    foot.spats.push("211-21-2-221".to_string());
//   foot.spats.push("21-2-21-2-22".to_string());
//  foot.spats.push("2-111-12-121".to_string());
// foot.spats.push("122-112-2".to_string());
	
//    for f in foot.spats {
    for (int i = 0; i < num_verses; ++i) {
//		new_stanza.push('\n');
		strcat(new_stanza, "\n");
//		let new_verse = generate_verse_with_foot(&syllables, rng, state, &f);
		char *new_verse = generate_random_verse(dict, 4, false, state, NULL);
		strcat(new_stanza, new_verse);

		if (state->LaTeX_output) {
			strcat(new_stanza, " \\\\");
		}

		free(new_verse);
		i = i + 1;
	}

	if (state->LaTeX_output) { 
		strcat(new_stanza, "!\n\n");
	}

	return strdup(new_stanza);

}

//fn capitalize_first(word: &str) -> String {
//
//    let mut v: Vec<char> = word.chars().collect();
//    v[0] = v[0].to_uppercase().nth(0).unwrap();
//    let s2: String = v.into_iter().collect();
//    let s3 = &s2;
//
//    return s3.to_string();
//
//}

char *capitalize_first_nodup(char *str) {
	str[0] = toupper(str[0]);
	return str;
}

char *capitalize_first_dup(char *str) {
	char *dup = strdup(str);
	dup[0] = toupper(dup[0]);

	return dup;
}
//fn generate_poem(word_database: &Vec<word_t>, rng: &mut StdRng, state: &kstate_t) -> String {
char *generate_poem(dict_t *dict, kstate_t *state) {

//    let distr = generate_distribution_low(1, 3);
 //   let num_words_title = get_random_with_distribution(rng, &distr);
	int num_words_title = 4;
//    let max_syllables = 4;
	int max_syllables = 4;

//    let mut title = capitalize_first(&construct_random_word(word_database, rng, max_syllables, state.rules_apply));
	char *title = capitalize_first_nodup(construct_random_word(dict, max_syllables, state->rules_apply));

    for (int i = 1; i < num_words_title; ++i) {
	    // crap
	    char *new_title = string_concat_with_delim(title, construct_random_word(dict, max_syllables, state->rules_apply), " ");
	    free(title);
	    title = new_title;
    }

//    let mut poem = String::new();
    char poem[8096];
    memset(poem, 0, sizeof(poem));

    if (state->LaTeX_output) {
		strcat(poem, "\\poemtitle{");
		strcat(poem, title);
		strcat(poem, "}\n");
		strcat(poem, "\\settowidth{\\versewidth}{levaton, sitän kylpää ranjoskan asdf}\n");
	    	strcat(poem, "\\begin{verse}[\\versewidth]\n");
    } else {
	    strcat(poem, title);
    }

//    let num_stanzas = get_random(rng, 1, 2); 
    int num_stanzas = 3;
    
    for (int i = 0; i < num_stanzas; ++i) {

//	let distr = generate_distribution_mid(1, 6);
//    	let num_verses = get_random_with_distribution(rng, &distr);
	int num_verses = 2;

//	let new_stanza = generate_random_stanza(word_database, rng, num_verses, state);
	char *new_stanza = generate_random_stanza(dict, num_verses, state);

//	poem.push_str(&format!("{}\n", new_stanza));
	strcat(poem, new_stanza);
	free(new_stanza);
    }

    if (state->LaTeX_output) {
	    strcat(poem, "\\end{verse}\n");
	    strcat(poem, "\\newpage\n");
    }

    return strdup(poem);
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

//fn print_as_latex_document(poems: &Vec<String>, poetname: &str) {
void print_as_latex_document(const char* poem, const char *poetname) {
	print_latex_preamble();
	print_latex_title_page(poetname);
	printf("%s", poem);
	printf("\\end{document}");

}

//fn generate_random_poetname(word_list: &Vec<word_t>, rng: &mut StdRng) -> String {
char *generate_random_poetname(dict_t *dict) {

	char name[128];
	memset(name, 0, sizeof(name));

//	let first_name = capitalize_first(&construct_random_word(word_list, rng, 3, true));
	char *first_name = capitalize_first_nodup(construct_random_word(dict, 3, true));
	//let second_initial = capitalize_first(&construct_random_word(word_list, rng, 1, true)).chars().next().unwrap();
	char *second_name = capitalize_first_nodup(construct_random_word(dict, 2, true));
	second_name[1] = '\0';

//	let surname = capitalize_first(&construct_random_word(word_list, rng, 5, true));
	char *surname = capitalize_first_nodup(construct_random_word(dict, 5, true));

	strcat(name, first_name);
	strcat(name, " ");
	strcat(name, second_name);
	strcat(name, ". ");
	strcat(name, surname);

	free(first_name);
	free(second_name);
	free(surname);

	return strdup(name);

}

//fn main() {
int main(int argc, char *argv[]) {

    //let mut source = read_file_to_words("kalevala.txt");
	dict_t dict = read_file_to_words("kalevala.txt");

	if (dict.num_words < 1) {
		fprintf(stderr, "kalemattu: fatal: couldn't open input file kalevala.txt, aborting!\n");
		return 1;
	}

	unsigned int seed = time(NULL);
	srand(seed);

//    writeln!(&mut stderr, "(info: using {} as random seed)\n\n", s).unwrap();
	printf("(info: using %u as random seed)\n\n", seed);

	kstate_t state;
	state.numeric_seed = seed;
	state.LaTeX_output = 0;
	state.rules_apply = 1;

//    let mut args: Vec<_> = env::args().collect();
//
//    let mut state_vars = kstate_t { numeric_seed: false, LaTeX_output: false, rules_apply: true };
//
//    for a in args.iter() {
//	if a == "--latex" {
//		writeln!(&mut stderr, "\n(info: option --latex provided, using LaTeX/verse suitable output!)").unwrap();
//		state_vars.LaTeX_output = true;
//	}
//	else if a == "--numeric" {
//		writeln!(&mut stderr, "\n(info: option --numeric provided, interpreting given seed as base-10 integer)").unwrap();
//		state_vars.numeric_seed = true;
//	}
//	else if a == "--chaos" {
//		writeln!(&mut stderr, "\n(info: option --chaos provided, disabling all filtering rules!)").unwrap();
//		state_vars.rules_apply = false;
//	}
//    }
//
//    // this should remove any arguments beginning with "--"
//    args.retain(|i|i.chars().take(2).collect::<Vec<char>>() != &['-', '-']);
//
//    let s = match args.len() {
//    	2 => { 
//		match state_vars.numeric_seed {
//			true => args[1].parse::<usize>().unwrap(),
//
//			false =>  {
//				writeln!(&mut stderr, "\n(info: option --numeric not provided, hashing string \"{}\" to be used as seed.)", args[1]).unwrap();
//				let mut hasher = SipHasher::new();
//				args[1].hash(&mut hasher);
//				hasher.finish() as usize
//			}
//		}
//	},
//
//	_ => time::precise_time_ns() as usize
//    };
//
//
//    let seed: &[_] = &[s,s,s,s];
//
//    let mut rng: StdRng = SeedableRng::from_seed(seed);


//    let mut poems: Vec<String> = Vec::new();

//   if state_vars.LaTeX_output {
//	let mut i = 0;
//	while i < 10 {
//		poems.push(generate_poem(&source, &mut rng, &state_vars));
//		i += 1;
//	}
//
//	print_as_latex_document(&poems, &generate_random_poetname(&source, &mut rng));
//
//   } else {

//println!("{}", generate_poem(&source, &mut rng, &state_vars));
	char *poem = generate_poem(&dict, &state);

	printf("%s\n", poem);

	free(poem);

}
