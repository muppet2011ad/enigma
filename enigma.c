#include <stdlib.h>
#include <stdio.h>
#include "readlines.h"
#include "rotor.h"

struct enigma_structure {
    rotor rotors[3];
    rotor reflector;
    char plugboard[26];
};

typedef struct enigma_structure *enigma;

void keypress_rotate(rotor rotors[3]);
char *encode_message(char *message, rotor rotors[3], rotor reflector, char plugboard[26]);
char p_sub(char c, char p[26]);
void input(char *string,int length);
enigma create_engima(FILE *config_file, r_template templates[], int num_templates);
void destroy_engima(enigma e);

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

    enigma e = create_engima(fopen("config", "r"), templates, num_lines);

    printf("Enter message to be encoded: ");
    char buffer[BUFFER_SIZE];
    input(buffer, BUFFER_SIZE);

    char *result = encode_message(buffer, e->rotors, e->reflector, e->plugboard);

    printf("Result: %s\n", result);
}

enigma create_engima(FILE *config_file, r_template templates[], int num_templates) {
    enigma e = malloc(sizeof(struct enigma_structure)); // Allocate memory for enigma
    int num_config_lines = 0;
    int config_lines_arr_size = LINES_ARR_LEN;
    char **config_lines = calloc(LINES_ARR_LEN, sizeof(char*));
    read_lines(config_file, &config_lines, &num_config_lines, &config_lines_arr_size); // Read in config file
    e->rotors[0] = create_rotor(templates[config_lines[0][0] - '1'], config_lines[1][0], config_lines[2][0]);
    e->rotors[1] = create_rotor(templates[config_lines[0][2] - '1'], config_lines[1][2], config_lines[2][2]);
    e->rotors[2] = create_rotor(templates[config_lines[0][4] - '1'], config_lines[1][4], config_lines[2][4]);
    e->reflector = create_rotor(templates[num_templates-1], 'A', 'A');
    strncpy(e->plugboard, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26);
    return e;
}

void destroy_engima(enigma e) {
    for (int i = 0; i < 3; i++) {
        free(e->rotors[i]);
    }
    free(e->reflector);
    free(e);
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

void input(char *string,int length) {
    fgets(string,length,stdin);
    int i = 0;
    while(*string != '\n') {
        i++;
        if (i == length) {
            *string = '\n';
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF);
        }
        else {
            string++;
        }
    }
    *string = '\0';
}