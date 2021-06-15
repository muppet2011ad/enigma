#include <stdlib.h>
#include "rotor.h"
#include <string.h>

int mod(int i, int j) {
    if (i < 0) {
        i += j;
    }
    return i % j;
}

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
    if (reflected){
        char result = 'a';
        c = (((c - 'A') + r->step) % 26) + 'A';
        int x = -1;
        while (result != c) {
            x++;
            result = r->substitutions[(x + r->step) % 26];
        }
        return x + 'A';
    }
    else {
        int raw = r->substitutions[(c - 'A' + r->step) % 26] - 'A';
        return 'A' + mod(raw - r->step, 26);
    }
}

rotor create_rotor(r_template template, char start_pos) { // Creates a rotor
    rotor r = malloc(sizeof(struct rotor_structure)); // TODO: error checking on malloc
    r->notch = template.notch - 'A';
    r->step = start_pos - 'A';
    strncpy(r->substitutions, template.substitutions, 26); // Copies substitutions in
    return r;
}

char r_get_position(rotor r) {
    return r->step + 'A';
}