#ifndef rotor_h

#define rotor_h

#include <stdlib.h>

struct rotor_structure {
    char substitutions[52]; // Stores normal substitutions in positions 0-25 and reflections in 26-51
    unsigned short notch : 5; // Notch position
    unsigned short step : 5; // Current rotation step
};

typedef struct rotor_structure *rotor;

rotor create_rotor(char* subs, char notch, char start_pos);
char r_sub (rotor r, char c, int reflected);
int r_rotate(rotor r);
char r_get_position(rotor r);

#endif