#ifndef rotor_h

#define rotor_h

#include <stdlib.h>

struct rotor_structure {
    char substitutions[52]; // Stores normal substitutions in positions 0-25 and reflections in 26-51
    unsigned short notch : 5; // Notch position
    unsigned short step : 5; // Current rotation step
};

typedef struct rotor_structure *rotor;

struct rotor_template {
    char substitutions[26];
    char notch;
};

typedef struct rotor_template r_template;

rotor create_rotor(r_template template, char start_pos);
char r_sub (rotor r, char c, int reflected);
int r_rotate(rotor r);
char r_get_position(rotor r);

#endif