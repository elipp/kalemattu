extern crate rand;
extern crate time;

use std::fs::File;
use std::io::Read;
use std::io::Write;
use std::env;
use rand::{Rng, SeedableRng, StdRng};
use std::hash::{Hash, SipHasher, Hasher};

struct kstate_t {
	numeric_seed: bool,
	LaTeX_output: bool,
	rules_apply: bool
}

struct meter_t {

}


fn vc_map(c : char) -> char {

    match c.is_alphabetic() {
        true => {
            return match c {
                'a' => 'V',
                'e' => 'V',
                'i' => 'V',
                'o' => 'V',
                'u' => 'V',
                'y' => 'V',
                'ä' => 'V',
                'ö' => 'V',
                _ => 'C'
            };
        },

        false => { return '?'; }
    }

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


struct word_t {
    chars : String,
    syllables : Vec<String>
}

fn clean_string(data: &str) -> String {
	return data.to_lowercase().matches(char::is_alphabetic).collect();
}

fn get_nth(word: &str, n: usize) -> char {
	return word.chars().nth(n).unwrap();
}

static VC_PATTERNS: &'static [&'static str] = &[
    "V", "VC", "VV", "VVC", "VCC",
    "CV", "CVC", "CVV", "CVVC", "CVCC", "CCV", "CCVC", "CCVV", "CCCV", "CCVVC", "CCVCC", "CCCVC", "CCCVCC"
];

fn find_longest_vc_match(vc: &str, offset: usize) -> &'static str {
	let mut longest: &'static str = &"";

	for p in VC_PATTERNS {

		let mut full_match = true;
		let mut i = offset;

		if (vc.chars().count() - offset) >= p.chars().count() {
			for c in p.chars() {
				if get_nth(vc, i) != c  {
					full_match = false;
					break;
				}
				i = i+1;
			}

			if full_match {
				if (vc.chars().count() - offset) > p.chars().count() {
					if get_nth(p, p.chars().count() - 1) == 'C' {
						// then the next letter can't be a vowel
						if get_nth(vc, p.chars().count() + offset) == 'V'{
							//println!("nextvowel check for {} at offset {} failed!", p, offset);
							full_match = false;
						}
					}
				}

				if full_match && longest.chars().count() < p.chars().count() {
					longest = p;
                //    println!("new longest matching pattern: for {} -> {}", &vc[offset..], p);
				}
			}
		}
	}

	return longest;

}

fn get_substring(word: &str, offset: usize, n: usize) -> String {
    let mut s = String::new();
    let mut i = 0;
    for c in word.chars().skip(offset) {
        if i >= n { break; }
        s.push(c);
        i = i+1;
    }

    return s;
}

fn get_vc_pattern(word: &str) -> String {

	let mut p = String::new();
	for c in word.chars() { p.push(vc_map(c)); }
	return p;

}

fn get_vc_pattern_grep(word: &str) -> String {
	let mut p = String::new();

	p.push('^');
	p.push_str(&get_vc_pattern(word));
	p.push('$');

	return p;

}

static NON_DIPHTHONGS: &'static [&'static str] =
&["ae", "ao", "ea", "eo", "ia", 
"io", "iä", "oa", "oe", "ua",
"ue", "ye", "yä", "äe", "äö", 
"öä", "eä", "iö", "eö", "öe",
"äa", "aä", "oö", "öo", "yu", 
"uy", "ya", "yu", "äu", "uä", 
"uö", "öu", "öa", "aö" ];

fn has_diphthong(syllable: &str) -> bool {
	for n in NON_DIPHTHONGS {
		if syllable.contains(n) { return false; }
	}

	return true;

}

static FORBIDDEN_CCOMBOS: &'static [&'static str] =
&[ "nm", "mn", "sv", "vs", "kt", 
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
"kg" ];

fn has_forbidden_ccombos(word: &str) -> bool {
	for c in FORBIDDEN_CCOMBOS {
		if word.contains(c) { return true; }
	}

	return false;
}

static FORBIDDEN_ENDCONSONANTS : &'static [char] = 
&[
'p', 'k', 'r', 'm', 'h', 'v', 's', 'l'
];

fn has_forbidden_endconsonant(word: &str) -> bool {
	for c in FORBIDDEN_ENDCONSONANTS {
		let last_char = word.chars().last().unwrap();
		if &last_char == c { return true; }
	}

	return false;
}	

fn syllabify(word: &mut word_t) {

    let mut offset = 0;

    let vc_pattern = get_vc_pattern(&word.chars); 

    while offset < vc_pattern.chars().count() {
    	let longest = find_longest_vc_match(&vc_pattern, offset);

        if longest.chars().count() < 1 {
//            println!("(warning: syllabify() for word \"{}\" failed, no matches found)", word.chars);
	    word.syllables.push(word.chars.to_string());
            break;
        }

        else {
	    let new_syl = get_substring(&word.chars, offset, longest.chars().count());
	    if offset > 0 && longest.contains("VV") && !has_diphthong(&new_syl) {
			// need to split this syllable into two
			let vv_offset = longest.find("VV").unwrap();
			let new_syl_split1 = get_substring(&word.chars, offset, vv_offset+1);
			let new_syl_split2 = get_substring(&word.chars, offset+vv_offset+1, longest.chars().count() - new_syl_split1.chars().count());

			//println!("split syllable {} into two: {}-{}", new_syl, &new_syl_split1, &new_syl_split2);
			
			word.syllables.push(new_syl_split1);
			word.syllables.push(new_syl_split2);


	    } else {
		    word.syllables.push(new_syl);
	    }
        }
        
	offset = offset + longest.chars().count();
    }

}

fn read_file_to_words(filename : &'static str) -> Vec<word_t> {

    let mut f = match File::open(filename) {
        Ok(file) => file,
        Err(e) => {
            // fallback in case of failure.
            // you could log the error, panic, or do anything else.
            // println("Opening file {} failed: {}", filename, e);

            return Vec::new();
        }
    };

    let mut s = String::new();
    let asd = f.read_to_string(&mut s);

    let mut words : Vec<word_t> = Vec::new();

    for x in s.split_whitespace() {

        let mut w = word_t { chars : clean_string(x), syllables : Vec::new() };

        if w.chars != "" {
            syllabify(&mut w);
            words.push(w);
        }

    }
    return words;
}

fn get_random(rng: &mut StdRng, min: usize, max: usize) -> usize {
	let k = ((max as f64)*rng.gen::<f64>() + (min as f64)) as usize;

	return k;
}


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

				if syl_vharm > 0 && syl_vharm != vharm_state {
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



fn get_random_syllable_from_word(word: &word_t, rng: &mut StdRng, ignore_last: bool) -> String {

	let mut sindex: usize = word.syllables.len();
	if ignore_last {
		sindex -= 1;
	}

	let syl = &word.syllables[get_random(rng, 0, sindex)];
	
	return syl.to_string();
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


fn generate_random_verse<'a>(word_list: &'a Vec<word_t>, rng: &mut StdRng, num_words: usize, last_verse: bool, state: &kstate_t) -> String {
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

fn generate_random_stanza<'a>(word_list: &'a Vec<word_t>, rng: &mut StdRng, num_verses: usize, state: &kstate_t) -> String {

	let mut new_stanza = String::new();
	let mut i = 0;
	
	while i < num_verses {
		new_stanza.push('\n');
		let distr = generate_distribution_high(2, 4);
		let num_words = get_random_with_distribution(rng, &distr);
		let new_verse = generate_random_verse(word_list, rng, num_words, i == num_verses - 1, state);

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
	while (i < 10) {
		poems.push(generate_poem(&source, &mut rng, &state_vars));
		i += 1;
	}

	print_as_latex_document(&poems, &generate_random_poetname(&source, &mut rng));

   } else {
	println!("{}", generate_poem(&source, &mut rng, &state_vars));
   }

}
