#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "enigma.h"
#include "data_structures.h"

#define IOC_ENGLISH_TEXT 1.73
#define ROTORS 5
#define BEST_ROTORS_COUNT 20

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
hashmap load_ngram_scores(FILE *f);
double ngram_score(char *string, hashmap ngrams, short n);

int string_equality(void *a, void *b) {
    return strcmp((char*) ((kv_pair) a)->key, (char*) ((kv_pair) b)->key) == 0;
}

int simple_hash(void *x, int len) {
    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum ^= (int) pow((double) ((char*) x)[i], (double) i+1);
        sum += (((char*) x)[i] * (i+1));
    }
    return sum;
}

void print_hashmap(hashmap h) {
    kv_pair *pairs = hashmap_to_array(h);
    for (int i = 0; i < h->nmeb; i++) {
        printf("%4s -> %f\n", (char*) pairs[i]->key, *(double*) pairs[i]->val);
    }
    printf("\n");
    free(pairs);
}

int main() {

    // Hashmap debugging zone

    // printf("Hello World\n");
    // hashmap h = new_hashmap(6, 0.75, string_equality, simple_hash);
    // char (*keys)[5];
    // keys = calloc(5, 5*sizeof(char));
    // int (*vals);
    // int arr[5] = {1, 2, 3, 4 ,5};
    // vals = calloc(5, sizeof(int));

    // memcpy(*keys, "ABCD\0EFGH\0IJKL\0MNOP\0QRST\0", 25);
    // memcpy(vals, arr, 5*sizeof(int));

    // for (int i = 0; i < 5; i++) {
    //     add_to_hashmap(h, keys[i], 4, &vals[i]);
    // }

    // int val = *(int*) get_value_from_hashmap(h, "IJKL\0", 4);

    // print_hashmap(h);

    // remove_from_hashmap(h, "IJKL\0", 4);
    // print_hashmap(h);

    // destroy_hashmap(h, 0);

    // free(keys);
    // free(vals);

    // exit(0);

    // End zone

    int num_rotors = 0;
    FILE* templates_file = fopen("rotors", "r");
    r_template *templates = load_templates_from_file(templates_file, &num_rotors);
    fclose(templates_file);
    FILE* source_file = fopen("example", "r");
    char *contents = read_in_line(source_file);
    fclose(source_file);

    //printf("%s\n", contents);
    
    //double *scores[5][5][26][26][26] = calloc(5*5*5*26*26*26, sizeof(double));
    rotor_config_and_score *rp_scores = calloc(BEST_ROTORS_COUNT, sizeof(rotor_config_and_score));

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
                                double score = 1/fabs(ioc(result) - IOC_ENGLISH_TEXT);
                                //scores[i][j][k][x][y][z] = score;
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
                                handle_new_score(rp_scores, BEST_ROTORS_COUNT, rs);
                                destroy_engima(e);
                            }
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < BEST_ROTORS_COUNT; i++) {
        printf("%2d: %5s %5s %5s in position %c %c %c with score %f\n\n", i+1, r_id_lookup[rp_scores[i].rotors[0]], r_id_lookup[rp_scores[i].rotors[1]], r_id_lookup[rp_scores[i].rotors[2]], rp_scores[i].positions[0], rp_scores[i].positions[1], rp_scores[i].positions[2], rp_scores[i].score);
    }

    rotor_config_and_score *rs_scores = calloc(BEST_ROTORS_COUNT, sizeof(rotor_config_and_score));

    FILE *bigrams_file = fopen("bigrams", "r");
    hashmap bigrams = load_ngram_scores(bigrams_file);
    fclose(bigrams_file);

    FILE *trigrams_file = fopen("trigrams", "r");
    hashmap trigrams = load_ngram_scores(trigrams_file);
    fclose(trigrams_file);

    printf("Loaded %d bigrams from file.\nLoaded %d trigrams from file.\n\n", bigrams->nmeb, trigrams->nmeb);

    // This was my original approach of cracking all ring settings at once
    // for (int i = 0; i < BEST_ROTORS_COUNT; i++) {
    //     rotor_config_and_score best = {0};
    //     r_template rotors[3] = {templates[rp_scores[i].rotors[0]], templates[rp_scores[i].rotors[1]], templates[rp_scores[i].rotors[2]]};
    //     for (int x = 0; x < 26; x++) {
    //         for (int y = 0; y < 26; y++) {
    //             for (int z = 0; z < 26; z++) {
    //                 char ring_settings[3] = {x + 'A', y + 'A', z + 'A'};
    //                 enigma e = create_enigma(rotors, templates[num_rotors-1], rp_scores[i].positions, ring_settings, plugboard, pairs);
    //                 char *result = encode_message(contents, e);
    //                 double score = ngram_score(result, bigrams, 2);
    //                 free(result);
    //                 if (score > best.score) {
    //                     memcpy(&best.rotors, &rp_scores[i].rotors, 3*sizeof(int));
    //                     strncpy(best.ring_settings, ring_settings, 3);
    //                     strncpy(best.positions, rp_scores[i].positions, 3);
    //                     strcpy(best.plugboard, plugboard);
    //                     strcpy(best.pairs, pairs);
    //                     best.score = score;
    //                 }
    //                 destroy_engima(e);
    //             }
    //         }
    //     }
    //     handle_new_score(rs_scores, BEST_ROTORS_COUNT, best);
    // }
    // free(rp_scores);

    // Will now implement computerphile approach of doing one at a time

    for (int i = 0; i < BEST_ROTORS_COUNT; i++) {
        rotor_config_and_score best = {0};
        r_template rotors[3] = {templates[rp_scores[i].rotors[0]], templates[rp_scores[i].rotors[1]], templates[rp_scores[i].rotors[2]]};
        char partial_config[3] = {'A', 'A', 'A'};
        double final_score;
        for (int j = 0; j < 3; j++) {
            double best_score = 0;
            char best_ring_setting = 'A';
            for (int x = 0; x < 26; x++) {
                char ring_settings[3];
                memcpy(ring_settings, partial_config, 3);
                ring_settings[j] = x + 'A';
                enigma e = create_enigma(rotors, templates[num_rotors-1], rp_scores[i].positions, ring_settings, plugboard, pairs);
                char *result = encode_message(contents, e);
                double score = ngram_score(result, trigrams, 3);
                free(result);
                if (score > best_score) {
                    best_score = score;
                    best_ring_setting = x + 'A';
                }
                destroy_engima(e);
            }
            partial_config[j] = best_ring_setting;
            if (j == 2) {
                final_score = best_score;
            }
        }
        memcpy(&best.rotors, &rp_scores[i].rotors, 3*sizeof(int));
        strncpy(best.ring_settings, ring_settings, 3);
        strncpy(best.positions, rp_scores[i].positions, 3);
        strcpy(best.plugboard, plugboard);
        strcpy(best.pairs, pairs);
        best.score = final_score;
        handle_new_score(rs_scores, BEST_ROTORS_COUNT, best);
    }

    printf("Best: %5s %5s %5s in position %c %c %c and ring setting %c %c %c with score %f\n",  r_id_lookup[rs_scores[0].rotors[0]], r_id_lookup[rs_scores[0].rotors[1]], r_id_lookup[rs_scores[0].rotors[2]],
                                                                                                rs_scores[0].positions[0], rs_scores[0].positions[1], rs_scores[0].positions[2],
                                                                                                rs_scores[0].ring_settings[0], rs_scores[0].ring_settings[1], rs_scores[0].ring_settings[2],
                                                                                                rs_scores[0].score);

    free(rs_scores);
    free(contents);
    free(templates);
}

void handle_new_score(rotor_config_and_score *best_list, int list_length, rotor_config_and_score new_score) {
    for (int i = list_length-1; i >= 0; i--) {
        if (new_score.score > best_list[i].score) {
            if (i != list_length-1) {
                best_list[i+1] = best_list[i];
            }
            if (i == 0) {
                best_list[0] = new_score;
            }
            else if (new_score.score <= best_list[i-1].score) {
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

double ngram_score(char *string, hashmap ngrams, short n) {
    size_t length = strlen(string);
    double final_score = 0;
    for (int i = 0; i < length - (n-1); i++) {
        char key[n+1];
        memcpy(key, &(string[i]), n);
        key[n] = '\0';
        double this_ngram_score;
        void *result = get_value_from_hashmap(ngrams, key, n);
        if (result) {
            this_ngram_score = pow(*(double*) result, (double) 2);
        } 
        else {
            this_ngram_score = 0;
        }
        final_score += this_ngram_score;
    }
    return final_score;
}

hashmap load_ngram_scores(FILE *f) {
    char *input_buffer = calloc(64, sizeof(char));
    linked_list ngrams_list = new_linked_list();
    linked_list frequencies_list = new_linked_list();
    hashmap h = new_hashmap(500, 0.75, string_equality, simple_hash);
    unsigned long combined_frequency = 0;
    while (!feof(f)) {
        char* result = fgets(input_buffer, 64, f);
        if (!result) {
            continue;
        }
        if (input_buffer[0] == '\n') {
            break;
        }
        char *new_ngram = calloc(8, sizeof(char));
        int *new_frequency = malloc(sizeof(int));
        sscanf(input_buffer, "%7s %d", new_ngram, new_frequency);
        combined_frequency += *new_frequency;
        add_to_linked_list(ngrams_list, new_ngram);
        add_to_linked_list(frequencies_list, new_frequency);
    }
    free(input_buffer);
    char **ngrams = (char**) linked_list_to_array(ngrams_list);
    int **frequencies = (int**) linked_list_to_array(frequencies_list);
    for (int i = 0; i < ngrams_list->length; i++) {
        char *key = malloc(8);
        strncpy(key, ngrams[i], 8);
        double *val = malloc(sizeof(double));
        *val = (double)*frequencies[i]/combined_frequency;
        int succ = add_to_hashmap(h, key, strlen(key), val);
        if (!succ) {
            fprintf(stderr, "Could not add key %s with value %f to hashmap.\n\n", key, *val);
            free(key);
            free(val);
        }
    }
    free(ngrams);
    free(frequencies);
    destroy_linked_list(ngrams_list);
    destroy_linked_list(frequencies_list);
    return h;
}