#ifndef enigma_h

#define enigma_h

#include "rotor.h"
#include "readlines.h"
#include <stdio.h>
#include <ctype.h>

struct enigma_structure {
    rotor rotors[3];
    rotor reflector;
    char plugboard[27]; // Used to perform substitution
    char pairs[30]; // Not needed for enigma functionality, but useful to output plugboard config
};

typedef struct enigma_structure *enigma;

void keypress_rotate(rotor rotors[3]);
char *encode_message(char *message, enigma e);
char p_sub(char c, char p[26]);
void input(char *string,int length, FILE *src);
enigma create_enigma_from_file(FILE *config_file, r_template templates[], int num_templates);
void destroy_engima(enigma e);
void display_config(enigma e);
r_template *load_templates_from_file(FILE* f, int *num_rotors);
enigma create_enigma(r_template rotors[3], r_template reflector, char positions[3], char ring_settings[3], char plugboard[27], char pairs[30]);
char *read_in_line(FILE *input_file);

#endif