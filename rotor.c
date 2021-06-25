#include <stdlib.h>
#include "rotor.h"
#include <string.h>

#define mod(x, n) ((x % n + n) %n)

int r_rotate(rotor r) {
    int special = 0; // Determines whether rotor 2 rotates if this is rotor 1 or we double step if this is rotor 2
    // Behaviour is implemented by whatever calls this
    if (r->notch == r->step) {
        special = 1;
    }
    r->step = (r->step + 1) % 26;
    r->offset = (r->offset + 1) % 26;
    return special;
}

char r_sub (rotor r, char c, int reflected) { // Performs substitution in both directions
    if (reflected){
        return r_sub_reflect(r, c);
    }
    else {
        return r_sub_no_reflect(r, c);
    }
}

char r_sub_no_reflect(rotor r, char c) {
    return mod(r->substitutions[(c + r->offset) % 26] - r->offset, 26);
}

char r_sub_reflect(rotor r, char c) {
    return mod(r->substitutions[26 + ((c + r->offset) % 26)] - (r->offset), 26);
}

rotor create_rotor(r_template template, char start_pos, char ring_setting) { // Creates a rotor
    rotor r = malloc(sizeof(struct rotor_structure)); // TODO: error checking on malloc
    r->notch = template.notch - 'A';
    r->step = start_pos - 'A';
    r->ring_setting = ring_setting - 'A';
    r->offset = mod(r->step - r->ring_setting, 26);
    r->id = template.id;
    for (int i = 0; i < 26; i++) {
        r->substitutions[i] = template.substitutions[i] - 'A';
        r->substitutions[r->substitutions[i] + 26] = i;
    }
    return r;
}

void destroy_rotor(rotor r) {
    free(r);
}

char r_get_position(rotor r) {
    return r->step + 'A';
}

char r_get_ring_setting(rotor r) {
    return r->ring_setting + 'A';
}