#include <stdlib.h>
#include <stdio.h>
#include "readlines.h"
#include "rotor.h"

void keypress_rotate(rotor rotors[3]);
char *encode_message(char *message, rotor rotors[3], rotor reflector);

int main() {
    int num_lines = 0;
    int lines_arr_size = LINES_ARR_LEN;
    char **lines = calloc(LINES_ARR_LEN, sizeof(char*));
    FILE* file = fopen("rotors", "r");
    read_lines(file, &lines, &num_lines, &lines_arr_size);
    fclose(file);

    r_template templates[num_lines];

    for (int i = 0; i < num_lines; i++) {
        char *ptr = strtok(lines[i], " ");
        strncpy(templates[i].substitutions, ptr, 26);
        ptr = strtok(NULL, " ");
        if (i != num_lines-1) {
            templates[i].notch = *ptr;
        }
        else {
            templates[i].notch = 27;
        }
        free(lines[i]);
    }

    free(lines);

    rotor r0 = create_rotor(templates[0], 'A');
    rotor r1 = create_rotor(templates[1], 'A');
    rotor r2 = create_rotor(templates[2], 'A');
    rotor rr = create_rotor(templates[num_lines-1], 'A');

    rotor rotors[] = {r0, r1, r2};

    char *result = encode_message("PROGRAMMINGPUZZLES", rotors, rr);

    printf("%s\n", result);
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

char *encode_message(char *message, rotor rotors[3], rotor reflector) {
    int msg_len = strlen(message);
    char *encoded = malloc(msg_len+1);
    for (int i = 0; i < msg_len; i++) {
        keypress_rotate(rotors);
        // Absolutely cursed line of code here but it essentially manages the whole journey from rotor 0 to the reflector and back
        encoded[i] = r_sub(rotors[0], r_sub(rotors[1], r_sub(rotors[2], r_sub(reflector, r_sub(rotors[2], r_sub(rotors[1], r_sub(rotors[0], message[i], 0), 0), 0), 1), 1), 1), 1);
    }
    encoded[msg_len] = '\0';
    return encoded;
}