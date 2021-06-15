#include <stdlib.h>
#include "rotor.h"
#include <string.h>

int rotate_rotor(rotor r) {
    int special = 0; // Determines whether rotor 2 rotates if this is rotor 1 or we double step if this is rotor 2
    // Behaviour is implemented by whatever calls this
    if (!r->just_moved && r->slot == 0 && r->notch == r->step) {
        special = 1;
    }
    r->step = (r->step + 1) % 26;
    r->just_moved = 1;
    return special;
}

char r_sub (rotor r, char c, int reflected) { // Performs substitution in both directions
    return r->substitutions[((c - 'A' + r->step) % 26) + reflected*26];
}

void r_end_keypress(rotor r) { // Resets rotor movement
    r->just_moved = 0;
}

rotor create_rotor(char* subs, char notch, unsigned short start_pos, unsigned short slot) { // Creates a rotor
    rotor r = malloc(sizeof(struct rotor_structure)); // TODO: error checking on malloc
    r->notch = notch - 'A';
    r->step = start_pos;
    r->slot = slot;
    r->just_moved = 0;
    strncpy(r->substitutions, subs, 26); // Copies substitutions in
    for (int i = 0; i < 26; i++) { // Calculates substitutions in the reflected direction (quicker than working it out for every keypress)
        char reverse_input = r->substitutions[i];
        r->substitutions[26 + reverse_input - 'A'] = 'A' + i;
    }

    return r;
}