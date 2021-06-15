#ifndef rotor_h

#define rotor_h

#include <stdlib.h>

struct rotor_structure {
    char substitutions[52]; // Stores normal substitutions in positions 0-25 and reflections in 26-51
    unsigned short notch : 5; // Notch position
    unsigned short step : 5; // Current rotation step
    unsigned short slot : 2; // Which slot the rotor is in
    unsigned short just_moved : 1; // Determines whether the rotor has moved this keypress (relevant for double-stepping)
};

typedef struct rotor_structure *rotor;

rotor create_rotor(char* subs, char notch, unsigned short start_pos, unsigned short slot);
void r_end_keypress(rotor r);
char r_sub (rotor r, char c, int reflected);
int rotate_rotor(rotor r);

#endif