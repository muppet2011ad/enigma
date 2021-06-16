#include <stdlib.h>
#include <stdio.h>
#include "readlines.h"
#include "rotor.h"

void keypress_rotate(rotor rotors[3]);
char *encode_message(char *message, rotor rotors[3], rotor reflector, char plugboard[26]);
char p_sub(char c, char p[26]);

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

    rotor r0 = create_rotor(templates[0], 'A', 'B');
    rotor r1 = create_rotor(templates[1], 'A', 'B');
    rotor r2 = create_rotor(templates[2], 'A', 'B');
    rotor rr = create_rotor(templates[num_lines-1], 'A', 'A');

    rotor rotors[] = {r0, r1, r2};

    char *result = encode_message("AAAAA", rotors, rr, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

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

char p_sub(char c, char p[26]) {
    return p[c - 'A'];
}

char *encode_message(char *message, rotor rotors[3], rotor reflector, char plugboard[26]) {
    int msg_len = strlen(message);
    char *encoded = malloc(msg_len+1);
    for (int i = 0; i < msg_len; i++) {
        keypress_rotate(rotors);
        char c = p_sub(message[i], plugboard);
        // Absolutely cursed line of code here but it essentially manages the whole journey from rotor 0 to the reflector and back
        c = r_sub(rotors[0], r_sub(rotors[1], r_sub(rotors[2], r_sub(reflector, r_sub(rotors[2], r_sub(rotors[1], r_sub(rotors[0], c, 0), 0), 0), 1), 1), 1), 1);
        encoded[i] = p_sub(c, plugboard);
    }
    encoded[msg_len] = '\0';
    return encoded;
}