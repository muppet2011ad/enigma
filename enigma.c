#include <stdlib.h>
#include <stdio.h>
#include "readlines.h"
#include "rotor.h"

void keypress_rotate(rotor rotors[3]);

int main() {
    int num_lines = 0;
    int lines_arr_size = LINES_ARR_LEN;
    char **lines = calloc(LINES_ARR_LEN, sizeof(char*));
    FILE* file = fopen("rotors", "r");
    read_lines(file, &lines, &num_lines, &lines_arr_size);
    fclose(file);

    for (int i = 0; i < num_lines; i++) {
        printf("%s", lines[i]);
    }

    printf("\n\n");

    rotor r0 = create_rotor(lines[0], 'Q', 'P');
    rotor r1 = create_rotor(lines[2], 'V', 'U');
    rotor r2 = create_rotor(lines[4], 'Z', 'H');

    rotor rotors[] = {r0, r1, r2};

    for (int i = 0; i < 4; i++) {
        printf("%c, %c, %c\n", r_get_position(r0), r_get_position(r1), r_get_position(r2));
        keypress_rotate(rotors);
    }

}

void keypress_rotate(rotor rotors[3]) {
    short notch_0_hit = r_rotate(rotors[0]);
    if (notch_0_hit) {
        r_rotate(rotors[1]);
    }
    else if (rotors[1]->step == rotors[1]->notch) {
        r_rotate(rotors[1]);
        r_rotate(rotors[2]);
    }
}