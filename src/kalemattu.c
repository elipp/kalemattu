#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

struct strvec_t {
	char **strs;
	int length;
	int capacity;
};

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
		vec->strs = realloc(vec->capacity*sizeof(char*));
		vec->length += 1;
	}

	strs[vec->length - 1] = strdup(str);
}

struct kstate_t {
	int numeric_seed;
	int LaTeX_output;
	int rules_apply;
};

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


struct syl_t {
	char *chars;
	size_t length;
	char length_class;
};

syl_t syl_create(const char* syl, char length_class) {
	syl_t s;
	s.chars = strdup(syl);
	s.length = strlen(syl);
	s.length_class = length_class;

	return s;
}

struct sylvec_t {
	syl_t *syllables;
	size_t length;
	size_t capacity;
};

int sylvec_push(sylvec_t *s, const syl_t *syl) {
	if (s->length < 1) {
		s->capacity = 2;
		s->strs = malloc(s->capacity*sizeof(syl_t));
		s->length = 1;
	}
	else if (s->length + 1 > s->capacity) {
		s->capacity *= 2;
		s->strs = realloc(s->capacity*sizeof(syl_t));
		s->length += 1;
	}

	s->syllables[s->length - 1] = *syl;

	return 1;

}

int sylvec_push_slice(sylvec_t *s, const sylvec_t *in) {
	for (long i = 0; i < in->length; ++i) {
		sylvec_push(s, &in->syllables[i]);
	}

	return 1;
}

struct word_t {
	char *chars;
	size_t length;
	sylvec_t syllables;
}

void word_syllabify(word_t *w);

word_t word_create(const char* chars) {
	word_t w;
	w.chars = strdup(chars);
	w.length = strlen(chars);
	word_syllabify(&w);

	return w;
}

int word_push_syllable(word_t *w, const syl_t *s) {
	sylvec_push(&w->syllables, s);
}

struct foot_t {
    char **spats; // "syllable patterns" :D
    size_t num_spats;
}

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

struct vcp_t {
	const char* pat;
	char length_class;
};


static const VC_PATTERNS[] = {
    vcp_t {"V", '1'},
    vcp_t {"VC",'1'},
    vcp_t {"VV", '2' },
    vcp_t {"VVC", '2' },
    vcp_t {"VCC", '1' },
    vcp_t {"CV", '1' },
    vcp_t {"CVC", '1' },
    vcp_t {"CVV", '2' },
    vcp_t {"CVVC", '2' },
    vcp_t {"CVCC", '1' },
    vcp_t {"CCV", '1' },
    vcp_t {"CCVC", '1' },
    vcp_t {"CCVV", '2' },
    vcp_t {"CCVVC", '2' },
    vcp_t {"CCVCC", '1' },
    vcp_t {"CCCVC", '1' },
    vcp_t {"CCCVCC", '1' }
    vcp_t {NULL, '0'},
};

vcp_t *find_longest_vc_match(const char* vc, size_t offset) {
	static const vcp_t error_pat = {"", '0'};

	vcp_t *longest = &error_pat;
	size_t vclen = strlen(vc);
	vcp_t *current = &VC_PATTERNS[0];
	while (current->pat != NULL) {
	
//		let mut full_match = true;
		bool full_match = true;
//		let mut i = offset;
		size_t i = offset;
//		let &pat = &p.P;
		
		//let plen = pat.chars().count();
		size_t plen = strlen(current->pat);

		if ((vclen - offset) >= plen) {
//			for c in pat.chars() {
//				if get_nth(vc, i) != c  {
//					full_match = false;
//					break;
//				}
//				i = i+1;
//			}
			if (strncmp(current->pat, vc, plen) == 0) {
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
					if (current->pat[plen-1] == 'C') {
						if (vc[plen+offset] == 'V') {
							printf("warning: nextvowel check for %s failed at offset %lu!\n", current->pat, offset);
							full_match = false;
						}
					}
				}

//				if full_match && longest.P.chars().count() < plen {
//					longest = p;
//				//    println!("new longest matching pattern: for {} -> {}", &vc[offset..], p);
//				}

				if (full_match && strlen(longest.pat) < plen) {
					longest = p;
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
}

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
	const char *c = &FORBIDDEN_ENDCONSONANTS[0];
	char l = input[strlen(input)-1];
	size_t i = 0;

	while (i < sizeof(FORBIDDEN_ENDCONSONANTS)) {
		if (l == FORBIDDEN_ENDCONSONANTS[i]) { return true; }
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
	while (offset < vclen) {
//    	let longest = find_longest_vc_match(&vc_pattern, offset);
		vcp_t *longest = find_longest_vc_match(vc_pattern, offset);
		//let &pat = &longest.P;
		char *pat = longest->pattern;
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
		
			    if (offset > 0 && str_contains(pat, "VV") && !has_diphthong(&new_syl) && !has_double_vowel(&new_syl)) {
			// need to split this syllable into two
//			let vv_offset = pat.find("VV").unwrap();
				    size_t vv_offset = strstr(pat, "VV") - pat;
//			let new_syl_split1 = get_substring(&word.chars, offset, vv_offset+1);
				    char *p1 = get_substring(word->chars, offset, vv_offset + 1);
//			let new_syl_split2 = get_substring(&word.chars, offset+vv_offset+1, plen - new_syl_split1.chars().count());
				    char *p2 = get_substring(word->chars, offset+vv_offset+1, plen - strlen(new_syl_split1));

			//println!("split syllable {} into two: {}-{}", new_syl, &new_syl_split1, &new_syl_split2);

	//		let vc1 = get_vc_pattern(&new_syl_split1);
			char *vc1 = get_vc_pattern(p1);
			//let L1 = find_longest_vc_match(&vc1, 0);
			vcp_t *l1 = find_longest_vc_match(vc1, 0);

//			let vc2 = get_vc_pattern(&new_syl_split2);
			char *vc2 = get_vc_pattern(p2);
//			let L2 = find_longest_vc_match(&vc1, 0); // lol this was a bug actually
			vcp_t *l2 = find_longest_vc_match(vc2, 0);

			//word.syllables.push(syl_t{chars: new_syl_split1, length_class: L1.L});
			word_push_syllable(&word, syl_create(vc1, l1->length_class));
			//word.syllables.push(syl_t{chars: new_syl_split2, length_class: L2.L});
			word_push_syllable(&word, syl_create(vc2, l2->length_class));




		    } else {
//			word.syllables.push(syl_t{chars: new_syl, length_class: longest.L});
			word_push_syllable(&word, syl_create(new_syl, longest->length_class));
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

	char *endptr;
	char *token = strtok_r(buf, " ", &endptr);
	long num_words = 0;

	while (token) {
		++num_words;
		token = strtok_r(NULL, " ", &endptr);
	}
	
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

	char *endptr;
	char *token = strtok_r(buf, " ", &endptr);
	long num_words = 0;

	while (token) {
		token = strtok_r(NULL, " ", &endptr);
		words[num_words] = word_create(token);
		++num_words;
	}
	
	return words;

}

word_t *read_file_to_words(const char* filename) {
	FILE *fp = fopen(filename, "r");
	
	if (!fp) {
		fprintf(stderr, "error: Couldn't open file %s\n", filename);
		return NULL;
	}
	char *buf = read_file_to_buffer(fp);
	fclose(fp);

	word_t *words = construct_word_list(buf, word_count(buf));
	free(buf);

	return words;
	
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

sylvec_t compile_list_of_syllables(word_t *words, long num_words) {
	sylvec_t s;
	memset(&s, 0, sizeof(s));

	for (long i = 0; i < num_words; ++i) {
		sylvec_push_slice(&s, words[i]->syllables);
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


// hardcode these in C 
fn generate_distribution_mid(min: usize, max: usize) -> Vec<usize> {
	let middle: usize = ((max-min)/2) + 1;
	let mut distr: Vec<usize> = Vec::new();

	for i in min..(max+1) {
		if i <= middle {
			for j in 0..i {
				distr.push(i);
			}
		} else {
			for j in i..(max+1) {
				distr.push(i);
			}
		}
	}

	return distr;

}

fn generate_distribution_low(min: usize, max: usize) -> Vec<usize> {

	let mut distr: Vec<usize> = Vec::new();

	for i in min..(max+1) {
		for j in i..(max+1) {
			distr.push(i);
		}
	}

	return distr;

}

fn generate_distribution_high(min: usize, max: usize) -> Vec<usize> {

	let mut distr: Vec<usize> = Vec::new();

	for i in min..(max+1) {
		for j in i..(max+1) {
			distr.push((max+2) - i);
		}
	}

	return distr;

}


fn get_random_with_distribution(rng: &mut StdRng, distr: &Vec<usize>) -> usize {

	let r = rng.gen::<f64>();
	let index = (r * (distr.len() as f64)) as usize;
	let R = distr[index];

	return R;
}


fn get_random_word<'a>(word_list: &'a Vec<word_t>, mut rng: &mut StdRng) -> &'a word_t {
	let word = &word_list[get_random(rng, 0, word_list.len())];

	return word;
}

fn get_vowel_harmony_state(syllable: &str) -> usize {
	let mut state: usize = 0;

	if syllable.contains('a') || syllable.contains('o') || syllable.contains('u') {
		state = state | 0x1;
	} 

	if syllable.contains('ä') || syllable.contains('ö') || syllable.contains('y') {
		state = state | 0x2;
	} 

//	println!("word: \"{}\", returning vharm: {}", syllable, state);
	return state;
}

fn get_num_trailing_vowels(word: &str) -> usize {
	let mut num = 0;
	for c in word.chars().rev() {
		if vc_map(c) == 'V' {
			num = num+1;
		}
		else {
			break;
		}
	}


	return num;
}

fn get_num_beginning_vowels(word: &str) -> usize {
	let mut num = 0;

	for c in word.chars() {

		if vc_map(c) == 'V' {
			num = num+1;
		}
		else {
			break;
		}

	}

	return num;
}

fn get_first_consonant(syl: &str) -> char {

	for c in syl.chars() {
		if vc_map(c) == 'C' {
			return c;
		}
	}

	return '0';

}

static FORBIDDEN_VOWELENDINGS: &'static [&'static str] =
&["ai", "ei", "ou", "ae", "au", "iu", "oe", "ue", "äy", "ii", "yy", "äi", "eu" ];

fn ends_in_wrong_vowelcombo(word: &str) -> bool {
	let vcp = get_vc_pattern_grep(&word);
	if vcp.contains("VV$") {
		let mut iter = word.chars().rev();
		let last = iter.next().unwrap();
		let second_last = iter.next().unwrap();

		let mut lasttwo = String::new();
		lasttwo.push(second_last);
		lasttwo.push(last);

		for ed in FORBIDDEN_VOWELENDINGS {
			if ed == &lasttwo {
				return true;
			}

		}
	}

	return false;
}

fn construct_random_word<'a>(word_list: &'a Vec<word_t>, rng: &mut StdRng, max_syllables: usize, rules_apply: bool) -> String {

	let mut new_word = String::new();

	let distr = generate_distribution_mid(1, max_syllables);
	let num_syllables = get_random_with_distribution(rng, &distr);

	if num_syllables == 1 {
		loop {
			let w = get_random_word(word_list, rng);
			let r = rng.gen::<f64>();

			if w.syllables.len() == 1 || 
			   w.chars.chars().count() <= 4 || 
			   (r < 0.20 && w.chars.chars().count() <= 5) {

			       	return w.chars.clone(); 
			}
		}
	}

	let mut new_syllables: Vec<String> = Vec::new();
	
	let mut vharm_state = 0;
	let mut prev_first_c = '0';

	for n in 0..num_syllables {
		
		let ignore_last = n == 0;
		let mut syl = get_random_syllable_any(&word_list, rng, ignore_last);
		let mut syl_vharm: usize = 0;
		
		if rules_apply {
			loop { 
				syl_vharm = get_vowel_harmony_state(&syl); 
				let first_c = get_first_consonant(&syl);

				if syl_vharm > 0 && vharm_state != 0 && syl_vharm != vharm_state {
					syl = get_random_syllable_any(&word_list, rng, ignore_last);
				} 
				else if n > 0 && syl.chars().count() < 2 {
					syl = get_random_syllable_any(&word_list, rng, ignore_last);
				}
				else if new_syllables.contains(&syl) {
					syl = get_random_syllable_any(&word_list, rng, ignore_last);
				}
				else if has_forbidden_ccombos(&(new_word.clone() + &syl)) {
					syl = get_random_syllable_any(&word_list, rng, ignore_last);
				}
				else if get_num_trailing_vowels(&new_word) + get_num_beginning_vowels(&syl) > 2 {
					syl = get_random_syllable_any(&word_list, rng, ignore_last);
				}
				else if (n == num_syllables - 1) && (has_forbidden_endconsonant(&syl) || ends_in_wrong_vowelcombo(&syl)) {
					syl = get_random_syllable_any(&word_list, rng, ignore_last);
				} 
				else if first_c != '0' && first_c == prev_first_c {
					syl = get_random_syllable_any(&word_list, rng, ignore_last);
				}

				else { 
					prev_first_c = first_c;
					break;
				}

			}

			if vharm_state == 0 {
				// we're still in "undefined vocal harmony" => only either 'e's or 'i's have been encountered
				if syl_vharm > 0 {
					vharm_state = syl_vharm;
				}
			}
		}

		new_syllables.push(syl.clone());
		new_word.push_str(&syl);

	}


	if new_word.chars().count() < 2 {
		return construct_random_word(word_list, rng, max_syllables, rules_apply);
	} 
	else {
		return new_word;
	}

}

fn construct_word_with_foot<'a>(syllables: &'a Vec<syl_t>, rng: &mut StdRng, foot: &str) -> String {
 	let mut new_word = String::new();

	let num_syllables = foot.chars().count();

//	if num_syllables == 1 {
//		loop {
//			let w = get_random_word(word_list, rng);
//			let r = rng.gen::<f64>();
//
//			if w.syllables.len() == 1 || 
//			   w.chars.chars().count() <= 4 || 
//			   (r < 0.20 && w.chars.chars().count() <= 5) {
//
//			       	return w.chars.clone(); 
//			}
//		}
//	}

	let mut new_syllables: Vec<String> = Vec::new();
	
	let mut vharm_state = 0;
	let mut prev_first_c = '0';

    for (n, LC) in foot.chars().enumerate() {

        let ignore_last = n == 0;
        let mut syl = get_syllable_with_lclass(&syllables, rng, LC);
        let mut syl_vharm: usize = 0;

        loop { 

            syl_vharm = get_vowel_harmony_state(&syl); 
            let first_c = get_first_consonant(&syl);

//            println!("word (so far): {}, desired lclass: {} syl: {}", new_word, LC, syl);

            if syl_vharm > 0 && vharm_state != 0 && syl_vharm != vharm_state {
                syl = get_syllable_with_lclass(&syllables, rng, LC);
            } 
            else if n > 0 && syl.chars().count() < 2 {
                syl = get_syllable_with_lclass(&syllables, rng, LC);
            }
            else if new_syllables.contains(&syl) {
                syl = get_syllable_with_lclass(&syllables, rng, LC);
            }
            else if has_forbidden_ccombos(&(new_word.clone() + &syl)) {
                syl = get_syllable_with_lclass(&syllables, rng, LC);
            }
            else if get_num_trailing_vowels(&new_word) + get_num_beginning_vowels(&syl) > 2 {
                syl = get_syllable_with_lclass(&syllables, rng, LC);
            }
            else if (n == num_syllables-1) && (has_forbidden_endconsonant(&syl) || ends_in_wrong_vowelcombo(&syl)) {
                syl = get_syllable_with_lclass(&syllables, rng, LC);
            } 
            else if first_c != '0' && first_c == prev_first_c {
                syl = get_syllable_with_lclass(&syllables, rng, LC);
            }

            else { 
                prev_first_c = first_c;
                break;
            }

        }

        if vharm_state == 0 {
            // we're still in "undefined vocal harmony" => only either 'e's or 'i's have been encountered
            if syl_vharm > 0 {
                vharm_state = syl_vharm;
            }
        }

        new_syllables.push(syl.clone());
        new_word.push_str(&syl);

    }

//    println!("{}", new_word);

	return new_word;

}   

fn get_random_syllable_from_word(word: &word_t, rng: &mut StdRng, ignore_last: bool) -> String {

	let mut sindex: usize = word.syllables.len();
	if ignore_last {
		sindex -= 1;
	}

	let syl = word.syllables[get_random(rng, 0, sindex)].chars.clone();
	
	return syl;
}

fn get_random_syllable_any(word_list: &Vec<word_t>, rng: &mut StdRng, ignore_last: bool) -> String {
	let mut word = get_random_word(word_list, rng);
	loop {
		if word.syllables.len() == 1 {
			word = get_random_word(word_list, rng);
		} else {
			break;
		}
	}
	let syl = get_random_syllable_from_word(&word, rng, ignore_last);

	return syl;
}


fn get_random_syllable<'a>(syllables: &'a Vec<syl_t>, rng: &mut StdRng) -> &'a syl_t {
    let syl = &syllables[get_random(rng, 0, syllables.len() - 1)];
    return syl;
}

fn get_syllable_with_lclass(syllables: &Vec<syl_t>, rng: &mut StdRng, length_class: char) -> String {

	let mut syl = get_random_syllable(&syllables, rng);
    loop {
        if syl.length_class != length_class {
            syl = get_random_syllable(&syllables, rng);
   //         println!("{}, {} != {}", syl.chars, length_class, syl.length_class);
        }
        else {
    //        println!("OK! {}, {} == {}", syl.chars, length_class, syl.length_class);
            break;
        }
    }

	return syl.chars.clone(); 

}


fn generate_random_verse<'a>(word_list: &'a Vec<word_t>, rng: &mut StdRng, num_words: usize, last_verse: bool, state: &kstate_t, foot: &str) -> String {
	let mut new_verse = String::new();

//	println!("num_words: {}", num_words);

	for j in 0..num_words {

		let new_word = construct_random_word(&word_list, rng, 4, state.rules_apply);
		new_verse.push_str(&new_word);

		let r = rng.gen::<f64>();

		if last_verse && j == num_words - 1 {
			
			if r < 0.08 {
				new_verse.push('!');
			}
			else if r < 0.15 {
				new_verse.push('?');
			}
			else if r < 0.25 {
				new_verse.push('.');
			}

		} else {
			if r < 0.02 {
				new_verse.push(';');
			} 
				
			else if r < 0.15 {
				new_verse.push(',');
		
			} else if r < 0.18 {
				new_verse.push(':')
			}

		}

		if j != num_words - 1 {
			new_verse.push(' ');
		}

	}

	return new_verse;
	
}

fn generate_verse_with_foot<'a>(syllables: &'a Vec<syl_t>, rng: &mut StdRng, state: &kstate_t, foot: &str) -> String {
    let mut new_verse = String::new();

    for w in foot.split("-") {
        let num_syllables = w.chars().count();
        let w = construct_word_with_foot(syllables, rng, &w);
        new_verse = new_verse + &w + " ";
    }

    return new_verse;
}

fn generate_random_stanza<'a>(word_list: &'a Vec<word_t>, rng: &mut StdRng, num_verses: usize, state: &kstate_t) -> String {

	let mut new_stanza = String::new();
	let mut i = 0;

    let mut foot: foot_t = foot_t { spats: Vec::new() };

    let syllables = compile_list_of_syllables(word_list);

    foot.spats.push("211-21-2-221".to_string());
    foot.spats.push("21-2-21-2-22".to_string());
    foot.spats.push("2-111-12-121".to_string());
    foot.spats.push("122-112-2".to_string());
	
    for f in foot.spats {
		new_stanza.push('\n');
		let new_verse = generate_verse_with_foot(&syllables, rng, state, &f);

		new_stanza.push_str(&new_verse);

		if state.LaTeX_output {
			new_stanza.push_str(" \\\\");
		}

		i = i + 1;
	}

	if state.LaTeX_output { 
		new_stanza.push_str("!\n\n");
	}

	return new_stanza;

}

fn capitalize_first(word: &str) -> String {

    let mut v: Vec<char> = word.chars().collect();
    v[0] = v[0].to_uppercase().nth(0).unwrap();
    let s2: String = v.into_iter().collect();
    let s3 = &s2;

    return s3.to_string();

}


fn generate_poem(word_database: &Vec<word_t>, rng: &mut StdRng, state: &kstate_t) -> String {

    let distr = generate_distribution_low(1, 3);

    let num_words_title = get_random_with_distribution(rng, &distr);

    let max_syllables = 4;

    let mut title = capitalize_first(&construct_random_word(word_database, rng, max_syllables, state.rules_apply));

    for i in 1..num_words_title {
	    title = title + " " + &construct_random_word(word_database, rng, max_syllables, state.rules_apply);
    }

    let mut poem = String::new();

    if state.LaTeX_output {
	    poem.push_str(&format!("\\poemtitle{{{}}}\n", title));
	    poem.push_str(&format!("\\settowidth{{\\versewidth}}{{{}}}\n", "levaton, sitän kylpää ranjoskan asdf"));
	    poem.push_str("\\begin{verse}[\\versewidth]\n");
    } else {
	    poem.push_str(&format!("{}\n", &title));
    }

    let num_stanzas = get_random(rng, 1, 2); 
    let mut i = 0;
    
    while i < num_stanzas {

	let distr = generate_distribution_mid(1, 6);
    	let num_verses = get_random_with_distribution(rng, &distr);
	let new_stanza = generate_random_stanza(word_database, rng, num_verses, state);

	poem.push_str(&format!("{}\n", new_stanza));

	i = i + 1;

    }

    if state.LaTeX_output {
	    poem.push_str("\\end{verse}\n");
	    poem.push_str("\\newpage\n");
    }

    return poem;
}

static LATEX_PREAMBLE: &'static str = 

r#"\documentclass[12pt, a4paper]{article} 

\usepackage{verse}

\usepackage[utf8]{inputenc}
\usepackage[T1]{fontenc} 
\usepackage{palatino} 

\usepackage[object=vectorian]{pgfornament} %%  http://altermundus.com/pages/tkz/ornament/index.html

\setlength{\parindent}{0pt} 
\renewcommand{\poemtitlefont}{\normalfont\bfseries\large\centering} 

\setlength{\stanzaskip}{0.75\baselineskip} 

\newcommand{\sectionlinetwo}[2]{%
\nointerlineskip \vspace{.5\baselineskip}\hspace{\fill}
{\pgfornament[width=0.5\linewidth, color = #1]{#2}}
\hspace{\fill}
\par\nointerlineskip \vspace{.5\baselineskip}
}%
\begin{document}
"#;

fn latex_print_preamble() {
	println!("{}", LATEX_PREAMBLE);
}

fn latex_print_title_page(poet: &str) {
	println!(

"\\begin{{titlepage}}
\\centering
{{\\fontsize{{45}}{{50}}\\selectfont {} \\par}}
\\vspace{{4cm}}
\\sectionlinetwo{{black}}{{7}}
\\vspace{{5cm}}
{{\\fontsize{{35}}{{60}}\\selectfont \\itshape Runoja\\par}}
\\end{{titlepage}}", 

	poet);

}

fn print_as_latex_document(poems: &Vec<String>, poetname: &str) {
	latex_print_preamble();
	latex_print_title_page(poetname);

	for p in poems {
		println!("{}", p);
	}

	println!("\\end{{document}}");

}

fn generate_random_poetname(word_list: &Vec<word_t>, rng: &mut StdRng) -> String {

	let first_name = capitalize_first(&construct_random_word(word_list, rng, 3, true));
	let second_initial = capitalize_first(&construct_random_word(word_list, rng, 1, true)).chars().next().unwrap();
	let surname = capitalize_first(&construct_random_word(word_list, rng, 5, true));

	return format!("{} {}. {}", first_name, second_initial, surname);

}


fn main() {

//	srand(time(NULL));

    let mut stderr = std::io::stderr();

    let mut source = read_file_to_words("kalevala.txt");
    if source.len() < 1 {
        source = read_file_to_words("/home/elias/kalemattu/kalevala.txt");
    }
    if source.len() < 1 {
        writeln!(&mut stderr, "kalemattu: fatal: couldn't open input file kalevala.txt, aborting!");
        return;
    }


    let mut args: Vec<_> = env::args().collect();

    let mut state_vars = kstate_t { numeric_seed: false, LaTeX_output: false, rules_apply: true };

    for a in args.iter() {
	if a == "--latex" {
		writeln!(&mut stderr, "\n(info: option --latex provided, using LaTeX/verse suitable output!)").unwrap();
		state_vars.LaTeX_output = true;
	}
	else if a == "--numeric" {
		writeln!(&mut stderr, "\n(info: option --numeric provided, interpreting given seed as base-10 integer)").unwrap();
		state_vars.numeric_seed = true;
	}
	else if a == "--chaos" {
		writeln!(&mut stderr, "\n(info: option --chaos provided, disabling all filtering rules!)").unwrap();
		state_vars.rules_apply = false;
	}
    }

    // this should remove any arguments beginning with "--"
    args.retain(|i|i.chars().take(2).collect::<Vec<char>>() != &['-', '-']);

    let s = match args.len() {
    	2 => { 
		match state_vars.numeric_seed {
			true => args[1].parse::<usize>().unwrap(),

			false =>  {
				writeln!(&mut stderr, "\n(info: option --numeric not provided, hashing string \"{}\" to be used as seed.)", args[1]).unwrap();
				let mut hasher = SipHasher::new();
				args[1].hash(&mut hasher);
				hasher.finish() as usize
			}
		}
	},

	_ => time::precise_time_ns() as usize
    };


    let seed: &[_] = &[s,s,s,s];

    let mut rng: StdRng = SeedableRng::from_seed(seed);
    writeln!(&mut stderr, "(info: using {} as random seed)\n\n", s).unwrap();

    let mut poems: Vec<String> = Vec::new();

   if state_vars.LaTeX_output {
	let mut i = 0;
	while i < 10 {
		poems.push(generate_poem(&source, &mut rng, &state_vars));
		i += 1;
	}

	print_as_latex_document(&poems, &generate_random_poetname(&source, &mut rng));

   } else {
	println!("{}", generate_poem(&source, &mut rng, &state_vars));
   }

}
