#include <stdlib.h>
#include <stdio.h>
#include "readlines.h"
#include "rotor.h"

struct enigma_structure {
    rotor rotors[3];
    rotor reflector;
    char plugboard[27]; // Used to perform substitution
    char pairs[30]; // Not needed for enigma functionality, but useful to output plugboard config
};

typedef struct enigma_structure *enigma;

void keypress_rotate(rotor rotors[3]);
char *encode_message(char *message, rotor rotors[3], rotor reflector, char plugboard[26]);
char p_sub(char c, char p[26]);
void input(char *string,int length);
enigma create_enigma_from_file(FILE *config_file, r_template templates[], int num_templates);
void destroy_engima(enigma e);
void display_config(enigma e);
r_template *load_templates_from_file(FILE* f, int *num_rotors);
enigma create_enigma(r_template rotors[3], r_template reflector, char positions[3], char ring_settings[3], char plugboard[27], char pairs[30]);

int main() {
    FILE* file = fopen("rotors", "r");
    int num_rotors = 0;
    r_template *templates = load_templates_from_file(file, &num_rotors);
    fclose(file);

    FILE *config_file = fopen("config", "r");
    enigma e = create_enigma_from_file(config_file, templates, num_rotors);
    fclose(config_file);
    free(templates);

    display_config(e);

    printf("Enter message to be encoded: ");
    char buffer[BUFFER_SIZE];
    input(buffer, BUFFER_SIZE);

    char *result = encode_message(buffer, e->rotors, e->reflector, e->plugboard);

    printf("Result: %s\n", result);
    destroy_engima(e);
    free(result);
}

r_template *load_templates_from_file(FILE* f, int *num_rotors) {
    int arr_size = LINES_ARR_LEN;
    char **lines = calloc(LINES_ARR_LEN, sizeof(char*));
    read_lines(f, &lines, num_rotors, &arr_size);
    r_template *templates = malloc(*num_rotors*sizeof(r_template));
    for (int i = 0; i < *num_rotors; i++) {
        char *ptr = strtok(lines[i], " ");
        strncpy(templates[i].substitutions, ptr, 26);
        ptr = strtok(NULL, " ");
        if (i != *num_rotors-1) {
            templates[i].notch = *ptr;
            templates[i].id = i;
        }
        else {
            templates[i].notch = 27;
            templates[i].id = 0;
        }
        free(lines[i]);
    }

    free(lines);
    return templates;
}

enigma create_enigma(r_template rotors[3], r_template reflector, char positions[3], char ring_settings[3], char plugboard[27], char pairs[30]) {
    enigma e = malloc(sizeof(struct enigma_structure)); // Allocate memory for enigma
    e->rotors[0] = create_rotor(rotors[0], positions[0], ring_settings[0]);
    e->rotors[1] = create_rotor(rotors[1], positions[1], ring_settings[1]);
    e->rotors[2] = create_rotor(rotors[2], positions[2], ring_settings[2]);
    e->reflector = create_rotor(reflector, 'A', 'A');
    strcpy(e->plugboard, plugboard);
    strcpy(e->pairs, pairs);
    return e;
}

enigma create_enigma_from_file(FILE *config_file, r_template templates[], int num_templates) {
    int num_config_lines = 0;
    int config_lines_arr_size = LINES_ARR_LEN;
    char **config_lines = calloc(LINES_ARR_LEN, sizeof(char*));
    read_lines(config_file, &config_lines, &num_config_lines, &config_lines_arr_size); // Read in config file

    char plugboard[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ\0";
    char pairs[30] = "\0";
    // Loops through steckered pairs given in config
    if (config_lines[3] != NULL) {
        strncpy(pairs, config_lines[3], 30);
        pairs[29] = '\0';
        char *ptr = strtok(config_lines[3], " ");
        while (ptr != NULL) {
            plugboard[ptr[0] - 'A'] = ptr[1];
            plugboard[ptr[1] - 'A'] = ptr[0];
            ptr = strtok(NULL, " ");
        }
    }

    r_template rotors[3] = {templates[config_lines[0][0] - '1'], templates[config_lines[0][2] - '1'], templates[config_lines[0][4] - '1']};
    char positions[3] = {config_lines[1][0], config_lines[1][2], config_lines[1][4]};
    char ring_settings[3] = {config_lines[2][0], config_lines[2][2], config_lines[2][4]};

    enigma e = create_enigma(rotors, templates[num_templates-1], positions, ring_settings, plugboard, pairs);

    for (int i = 0; i < num_config_lines; i++) {
        free(config_lines[i]);
    }
    free(config_lines);
    return e;
}

void destroy_engima(enigma e) {
    for (int i = 0; i < 3; i++) {
        destroy_rotor(e->rotors[i]);
    }
    free(e->reflector);
    free(e);
}

void display_config(enigma e) {
    char r_id_lookup[8][5] = {"I", "II", "III", "IV", "V", "VI", "VII", "VIII"};
    printf("%12s | %4s | %4s | %4s\n-------------+------+------+-----\n", "Slot", "1", "2", "3");
    printf("%12s | %4s | %4s | %4s\n", "Rotor", r_id_lookup[e->rotors[0]->id], r_id_lookup[e->rotors[1]->id], r_id_lookup[e->rotors[2]->id]);
    printf("%12s | %4c | %4c | %4c\n", "Position", r_get_position(e->rotors[0]), r_get_position(e->rotors[1]), r_get_position(e->rotors[2]));
    printf("%12s | %4c | %4c | %4c\n", "Ringstellung", r_get_ring_setting(e->rotors[0]), r_get_ring_setting(e->rotors[1]), r_get_ring_setting(e->rotors[2]));
    printf("Plugboard: %s\n", e->pairs);
    printf("Note that under this numbering system, rotor slots are traversed in ascending order then descending order.\n\n");
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