#include <stdlib.h>
#include <stdio.h>
#include "enigma.h"

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

char *encode_message(char *message, enigma e) {
    int msg_len = strlen(message);
    char *encoded = malloc(msg_len+1);
    for (int i = 0; i < msg_len; i++) {
        keypress_rotate(e->rotors);
        char c = p_sub(message[i], e->plugboard);
        // Absolutely cursed line of code here but it essentially manages the whole journey from rotor 0 to the reflector and back
        char c00 = r_sub_no_reflect(e->rotors[0], c - 'A');
        char c01 = r_sub_no_reflect(e->rotors[1], c00);
        char c12 = r_sub_no_reflect(e->rotors[2], c01);
        char c2r = r_sub_no_reflect(e->reflector, c12);
        char cr2 = r_sub_reflect(e->rotors[2], c2r);
        char c21 = r_sub_reflect(e->rotors[1], cr2);
        char c10 = r_sub_reflect(e->rotors[0], c21);
        encoded[i] = p_sub(c10 + 'A', e->plugboard);
    }
    encoded[msg_len] = '\0';
    return encoded;
}

void input(char *string,int length, FILE* src) {
    fgets(string,length,src);
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

char *read_in_line(FILE *input_file) {
    int arr_size = LINES_ARR_LEN;
    int num_lines = 0;
    char **lines = calloc(LINES_ARR_LEN, sizeof(char*));
    read_lines(input_file, &lines, &num_lines, &arr_size);
    char *combined_string = malloc(BUFFER_SIZE*num_lines);
    int x = 0;
    for (int i = 0; i < num_lines; i++) {
        strcpy(&(combined_string[x]), lines[i]);
        x += strlen(lines[i]);
    }
    char *final_string = malloc(strlen(combined_string)+1);
    x = 0;
    for (int i = 0; i < strlen(combined_string); i++) {
        if (islower(combined_string[i])) {
            combined_string[i] = toupper(combined_string[i]);
        }
        if (!strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZ", combined_string[i])) {
            continue;
        }
        final_string[x] = combined_string[i];
        x++;
    }
    final_string[strlen(combined_string)] = '\0';
    destroy_lines(lines, num_lines);
    free(combined_string);
    return final_string;
}