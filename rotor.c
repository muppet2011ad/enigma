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
    return special;
}

char r_sub (rotor r, char c, int reflected) { // Performs substitution in both directions
    int offset = r->step - r->ring_setting;
    if (reflected){
        c = mod((c - 'A') + (offset), 26);
        char raw = r->substitutions[26+c] - 'A';
        return 'A' + mod(raw - (offset), 26);
    }
    else {
        int raw = r->substitutions[mod(c - 'A' + (offset), 26)] - 'A';
        return 'A' + mod(raw - (offset), 26);
    }
}

rotor create_rotor(r_template template, char start_pos, char ring_setting) { // Creates a rotor
    rotor r = malloc(sizeof(struct rotor_structure)); // TODO: error checking on malloc
    r->notch = template.notch - 'A';
    r->step = start_pos - 'A';
    r->ring_setting = ring_setting - 'A';
    r->id = template.id;
    strncpy(r->substitutions, template.substitutions, 26); // Copies substitutions in
    for (int i = 0; i < 26; i++) {
        r->substitutions[r->substitutions[i] - 'A' + 26] = i+'A';
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