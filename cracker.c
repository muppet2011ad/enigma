#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "enigma.h"

#define IOC_ENGLISH_TEXT 1.73
#define ROTORS 5
#define BEST_ROTORS_COUNT 10

struct rotor_config_and_score_structure {
    int rotors[3];
    char positions[3];
    char ring_settings[3];
    char plugboard[27];
    char pairs[30];
    double score;
};

typedef struct rotor_config_and_score_structure rotor_config_and_score;

double ioc(char *text);
char *read_in_line(FILE *input_file);
void handle_new_score(rotor_config_and_score *best_list, int list_length, rotor_config_and_score new_score);

int main() {
    int num_rotors = 0;
    FILE* templates_file = fopen("rotors", "r");
    r_template *templates = load_templates_from_file(templates_file, &num_rotors);
    fclose(templates_file);
    FILE* source_file = fopen("example", "r");
    char *contents = read_in_line(source_file);
    fclose(source_file);

    //printf("%s\n", contents);
    
    //double *scores[5][5][26][26][26] = calloc(5*5*5*26*26*26, sizeof(double));
    rotor_config_and_score *scores = calloc(BEST_ROTORS_COUNT, sizeof(rotor_config_and_score));

    char r_id_lookup[8][5] = {"I", "II", "III", "IV", "V", "VI", "VII", "VIII"};
    char ring_settings[3] = "AAA";
    char plugboard[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char pairs[30] = "\0";
    for (int i = 0; i < num_rotors-1; i++) {
        for (int j = 0; j < num_rotors-1; j++) {
            if (i == j) { continue; }
            for (int k = 0; k < num_rotors-1; k++) {
                if (j == k || i == k) { continue; }
                printf("%5s %5s %5s\n", r_id_lookup[i], r_id_lookup[j], r_id_lookup[k]);
                double numerator = 0;
                for (int x = 0; x < 26; x++) {
                    for (int y = 0; y < 26; y++) {
                        for (int z = 0; z < 26; z++) {
                            if (i == j || j == k || i == k) {
                                //scores[i][j][k][x][y][z] = -1;
                            }
                            else {
                                r_template rotors[3] = {templates[i], templates[j], templates[k]};
                                char positions[3] = {x + 'A', y + 'A', z + 'A'};
                                enigma e = create_enigma(rotors, templates[num_rotors-1], positions, ring_settings, plugboard, pairs);
                                char *result = encode_message(contents, e);
                                double score = ioc(result);
                                //scores[i][j][k][x][y][z] = score;
                                numerator += score;
                                free(result);
                                rotor_config_and_score rs;
                                rs.rotors[0] = i;
                                rs.rotors[1] = j;
                                rs.rotors[2] = k;
                                strncpy(rs.ring_settings, "AAA", 3);
                                strncpy(rs.positions, positions, 3);
                                strcpy(rs.plugboard, plugboard);
                                strcpy(rs.pairs, pairs);
                                rs.score = score;
                                handle_new_score(scores, BEST_ROTORS_COUNT, rs);
                                destroy_engima(e);
                            }
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < BEST_ROTORS_COUNT; i++) {
        printf("%2d: %5s %5s %5s in position %c %c %c with score %f\n\n", i+1, r_id_lookup[scores[i].rotors[0]], r_id_lookup[scores[i].rotors[1]], r_id_lookup[scores[i].rotors[2]], scores[i].positions[0], scores[i].positions[1], scores[i].positions[2], scores[i].score);
    }

    free(contents);
    free(templates);
}

void handle_new_score(rotor_config_and_score *best_list, int list_length, rotor_config_and_score new_score) {
    for (int i = list_length; i >= 0; --i) {
        if (fabs(new_score.score - IOC_ENGLISH_TEXT) < fabs(best_list[i].score - IOC_ENGLISH_TEXT)) {
            if (i != list_length-1) {
                best_list[i+1] = best_list[i];
            }
            if (i == 0) {
                best_list[0] = new_score;
            }
            else if (!(fabs(new_score.score - IOC_ENGLISH_TEXT) < fabs(best_list[i-1].score - IOC_ENGLISH_TEXT))) {
                best_list[i] = new_score;
            }
        }
        else {
            break;
        }
    }
}

double ioc(char *text) {
    int letter_counts[26] = {0};
    int length = strlen(text);
    for (int i = 0; i < length; i++) {
        letter_counts[text[i] - 'A'] += 1;
    }
    double numerator = 0;
    for (int i = 0; i < 26; i++) {
        numerator += letter_counts[i]*(letter_counts[i]-1);
    }
    double ic = numerator / (length*(length-1)/26);
    return ic;
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