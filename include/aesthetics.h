#ifndef AESTHETICS_H
#define AESTHETICS_H

#include <wchar.h>

#include "types.h"

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


bool is_vowel(wchar_t c);

char vc_map(wchar_t c);
const vcp_t *find_longest_vc_match(const char* vc, long offset);

char *get_vc_pattern(const wchar_t* input);
char *get_vc_pattern_grep(const wchar_t* input);

const vcp_t *find_longest_vc_match(const char* vc, long offset);

wchar_t get_first_consonant(const wchar_t *str);

int make_valid_word(wchar_t *buffer, long num_syllables, SYLLABLE_SOURCE_FUNC SYLLABLE_SOURCE, const char *sylp);
int make_any_word(wchar_t *buffer, long num_syllables, SYLLABLE_SOURCE_FUNC SYLLABLE_SOURCE);

bool has_diphthong(const wchar_t* syllable); 
bool has_double_vowel(const wchar_t* syllable);

filter_state_t filter_state_new(int num_syllables);
void filter_state_free(filter_state_t *fs);

bool is_forbidden_endconsonant(wchar_t c);
bool has_forbidden_endconsonant(const wchar_t *word);
int get_vowel_harmony_state(const wchar_t* word);

#endif
