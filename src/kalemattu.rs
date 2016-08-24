extern crate rand;
extern crate time;

use std::fs::File;
use std::io::Read;
use std::io::Write;
use std::env;
use rand::{Rng, SeedableRng, StdRng};
use std::hash::{Hash, SipHasher, Hasher};

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
    vc_pattern : String,
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

fn syllabify(word: &mut word_t) {

    let mut offset = 0;

    while offset < word.vc_pattern.chars().count() {
    	let mut longest = find_longest_vc_match(&word.vc_pattern, offset);

        if longest.chars().count() < 1 {
//            println!("(warning: syllabify() for word \"{}\" failed, no matches found)", word.chars);
	    word.syllables.push(word.chars.to_string());
            break;
        }

        else {
            word.syllables.push(get_substring(&word.chars, offset, longest.chars().count()));
            offset = offset + longest.chars().count();
            //println!("word: {} offset = {}, vc pattern length = {}", word.chars, offset, word.vc_pattern.chars().count());
        }
    }

//println!("Word: \"{}\", # syllables: {}", word.chars, word.syllables.len());
//    for s in word.syllables.iter() {
//        print!("{} - ", s);
//    }
//    println!("\n");
//
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

        let mut w = word_t { chars : clean_string(x), vc_pattern : String::new(), syllables : Vec::new() };

        if w.chars != "" {
            for c in w.chars.chars() {
                w.vc_pattern.push(vc_map(c));
            }

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
	let windex = word_list.len();
	let word = &word_list[get_random(rng, 0, windex)];

	return word;
}

fn get_vowel_harmony_state(syllable: &str) -> i32 {
	if syllable.contains('ä') || syllable.contains('ö') || syllable.contains('y') {
		return 1;
	} else if syllable.contains('a') || syllable.contains('o') || syllable.contains('u') {
		return 0;
	} else {
		return -1;
	}
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

fn get_num_beginning_vowels(syl: &str) -> usize {
	let mut num = 0;
	for c in syl.chars() {
		if vc_map(c) == 'V' {
			num = num+1;
		}
		else {
			break;
		}

	}

	return num;
}

fn construct_random_word<'a>(word_list: &'a Vec<word_t>, rng: &mut StdRng, max_syllables: usize) -> String {

	let mut n = 0;
	let mut new_word = String::new();

	let distr = generate_distribution_mid(1, max_syllables);
	let num_syllables = get_random_with_distribution(rng, &distr);

	let mut vharm_state = -1;

	while n < num_syllables {
		let mut syl = get_random_syllable_any(&word_list, rng);

		let this_vharm_state = get_vowel_harmony_state(&syl); 

		if vharm_state == -1 {
		// we're still in "undefined vocal harmony" == only either 'e's or 'i's have been encountered
			if this_vharm_state != -1 {
				vharm_state = this_vharm_state;
			}
		}

		else  {
			
			while get_vowel_harmony_state(&syl) != vharm_state {
				syl = get_random_syllable_any(&word_list, rng);
			}

			let mut vowels_beg = get_num_beginning_vowels(&syl);

		}

		if n == num_syllables - 1 {
			// finnish words never end in 'p', 'k' 'm' 'h' or 'r'
			let mut last_char = syl.chars().last().unwrap();

			while last_char == 'p' || last_char == 'k' || last_char == 'r' || last_char == 'm' || last_char == 'h' {
				syl = get_random_syllable_any(&word_list, rng);
				last_char = syl.chars().last().unwrap();
			}

			while get_vowel_harmony_state(&syl) != vharm_state {
				syl = get_random_syllable_any(&word_list, rng);
			}



		}

		new_word.push_str(&syl);

		n = n + 1;
	}

	if new_word.chars().count() < 2 {
		return construct_random_word(word_list, rng, max_syllables);
	} else {
		return new_word;
	}

}



fn get_random_syllable_from_word(word: &word_t, rng: &mut StdRng) -> String {

	let sindex = (word.syllables.len() as i32) as usize;
	let syl = &word.syllables[get_random(rng, 0, sindex)];
	
	return syl.to_string();
}

fn get_random_syllable_any(word_list: &Vec<word_t>, rng: &mut StdRng) -> String {
	let word = get_random_word(word_list, rng);
	let syl = get_random_syllable_from_word(&word, rng);

	return syl;
}


fn generate_random_verse<'a>(word_list: &'a Vec<word_t>, rng: &mut StdRng, num_words: usize) -> String {
	let mut j = 0;
	let mut new_verse = String::new();

//	println!("num_words: {}", num_words);

	while j < num_words {

		let new_word = construct_random_word(&word_list, rng, 4);
		new_verse.push_str(&new_word);

		let r = rng.gen::<f64>();

		if r < 0.15 {
			if j == num_words - 1 {
				new_verse.push('!');
			}
		}

		else if j < num_words - 1 {
			if r < 0.17 {
				new_verse.push_str(";");
			} 
			
			else if r < 0.30 {
				new_verse.push_str(",");
			}

		}

		if j != num_words - 1 {
			new_verse.push(' ');
		}


		j = j + 1;
	}

	return new_verse;
	
}

fn generate_random_stanza<'a>(word_list: &'a Vec<word_t>, rng: &mut StdRng, num_verses: usize, LaTeX_output: bool) -> String {

	let mut new_stanza = String::new();
	let mut i = 0;
	
	while i < num_verses {
		new_stanza.push('\n');
		let distr = generate_distribution_high(2, 4);
		let num_words = get_random_with_distribution(rng, &distr);
		let new_verse = generate_random_verse(word_list, rng, num_words);

		new_stanza.push_str(&new_verse);

		if i == num_verses - 1 {
			let r = rng.gen::<f64>();
			if r < 0.15 {
				let last_char = new_stanza.chars().last().unwrap();
				if last_char == '.' {
					new_stanza.pop();
				}
				new_stanza.push('?');
			}
			else if r < 0.25 {
				new_stanza.push('.');
			}
		}

		if LaTeX_output {
			new_stanza.push_str(" \\\\");
		}

		i = i + 1;
	}

	if LaTeX_output { 
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


fn generate_poem(word_database: &Vec<word_t>, rng: &mut StdRng, LaTeX: bool) -> String {

    let distr = generate_distribution_low(1, 3);

    let num_words_title = get_random_with_distribution(rng, &distr);
    let mut max_syllables = 4;
    let mut title = capitalize_first(&construct_random_word(word_database, rng, max_syllables));

    for i in 1..num_words_title {
	    title = title + " " + &construct_random_word(word_database, rng, max_syllables);
    }

    let mut poem = String::new();

    if LaTeX {
	    poem.push_str(&format!("\\poemtitle{{{}}}\n", title));
	    poem.push_str(&format!("\\settowidth{{\\versewidth}}{{{}}}\n", "levaton, sitän kylpää ranjoskan"));
	    poem.push_str("\\begin{verse}[\\versewidth]\n");
    } else {
	    poem.push_str(&format!("{}\n", &title));
    }

    let num_stanzas = get_random(rng, 1, 5); 
    let mut i = 0;
    
    while i < num_stanzas {

	let distr = generate_distribution_mid(1, 8);
    	let num_verses = get_random_with_distribution(rng, &distr);
	let new_stanza = generate_random_stanza(word_database, rng, num_verses, LaTeX);

	poem.push_str(&format!("{}\n", new_stanza));

	i = i + 1;

    }

    if LaTeX {
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
\\vspace{{5cm}}
\\sectionlinetwo{{darkgray}}{{44}}
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


fn main() {

    let source = read_file_to_words("kalevala.txt");

    let mut stderr = std::io::stderr();

    let mut args: Vec<_> = env::args().collect();

    let mut numeric_seed: bool = false;
    let mut LaTeX_output: bool = false;

    for a in args.iter() {
	if a == "--latex" {
		writeln!(&mut stderr, "\n(info: option --latex provided, using LaTeX/verse suitable output!)").unwrap();
		LaTeX_output = true;
	}
	else if a == "--numeric" {
		writeln!(&mut stderr, "\n(info: option --numeric provided, interpreting given seed as base-10 integer)").unwrap();
		numeric_seed = true;
	}
    }

    // this should remove any arguments beginning with "--"
    args.retain(|i|i.chars().take(2).collect::<Vec<char>>() != &['-', '-']);

    let s = match args.len() {
    	2 => { 
		match numeric_seed {
			true => args[1].parse::<usize>().unwrap(),

			false =>  {
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

   if LaTeX_output {
	for i in 0..10 {
		poems.push(generate_poem(&source, &mut rng, LaTeX_output));
	}

	let mut name = capitalize_first(&construct_random_word(&source, &mut rng, 3));
	name.push(' ');
	let second_initial = capitalize_first(&construct_random_word(&source, &mut rng, 1)).chars().next().unwrap();
	name.push(second_initial);
	name.push_str(". ");
	let surname = capitalize_first(&construct_random_word(&source, &mut rng, 3));
	name.push_str(&surname);

	print_as_latex_document(&poems, &name);

   } else {
	println!("{}", generate_poem(&source, &mut rng, LaTeX_output));
   }

}
