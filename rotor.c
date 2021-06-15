#include <stdlib.h>
#include "rotor.h"
#include <string.h>

int r_rotate(rotor r) {
    int special = 0; // Determines whether rotor 2 rotates if this is rotor 1 or we double step if this is rotor 2
    // Behaviour is implemented by whatever calls this
    if (r->notch == r->step) {
        special = 1;
    }
    r->step = (r->step + 1) % 26;
    return special;
}

char r_sub (rotor r, char c, int reflected) { // Performs substitution in both directions
    return r->substitutions[((c - 'A' + r->step) % 26) + reflected*26];
}

rotor create_rotor(r_template template, char start_pos) { // Creates a rotor
    rotor r = malloc(sizeof(struct rotor_structure)); // TODO: error checking on malloc
    r->notch = template.notch - 'A';
    r->step = start_pos - 'A';
    strncpy(r->substitutions, template.substitutions, 26); // Copies substitutions in
    for (int i = 0; i < 26; i++) { // Calculates substitutions in the reflected direction (quicker than working it out for every keypress)
        char reverse_input = r->substitutions[i];
        r->substitutions[26 + reverse_input - 'A'] = 'A' + i;
    }

    return r;
}

char r_get_position(rotor r) {
    return r->step + 'A';
}