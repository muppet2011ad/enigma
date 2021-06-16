#ifndef rotor_h

#define rotor_h

#include <stdlib.h>

struct rotor_structure {
    char substitutions[26]; // Stores normal substitutions in positions 0-25 and reflections in 26-51
    unsigned short notch : 5; // Notch position
    unsigned short ring_setting : 5; // Ringstellung
    unsigned short step : 5; // Current rotation step
    unsigned short id : 3;
};

typedef struct rotor_structure *rotor;

struct rotor_template {
    char substitutions[26];
    char notch;
    unsigned short id : 3;
};

typedef struct rotor_template r_template;

rotor create_rotor(r_template template, char start_pos, char ring_setting);
void destroy_rotor(rotor r);
char r_sub (rotor r, char c, int reflected);
int r_rotate(rotor r);
char r_get_position(rotor r);
char r_get_ring_setting(rotor r);

#endif