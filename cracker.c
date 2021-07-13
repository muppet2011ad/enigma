#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "enigma.h"
#include "data_structures.h"

#define mod(x, n) ((x % n + n) %n)

#define IOC_ENGLISH_TEXT 1.73
#define ROTORS 5
#define BEST_ROTORS_COUNT 10
#define MAX_HASHMAP_LOAD_FACTOR 0.7
#define NUM_PLUGS 5

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
    FILE *bigrams_file = fopen("data/bigrams-cphile", "r");
    hashmap bigrams = load_ngram_scores(bigrams_file);
    fclose(bigrams_file);

    FILE *trigrams_file = fopen("data/trigrams-cphile", "r");
    hashmap trigrams = load_ngram_scores(trigrams_file);
    fclose(trigrams_file);

    FILE *quadgrams_file = fopen("data/quadgrams-cphile", "r");
    hashmap quadgrams = load_ngram_scores(quadgrams_file);
    fclose(quadgrams_file);

    printf("Loaded %d bigrams from file.\nLoaded %d trigrams from file.\nLoaded %d quadgrams from file.\n\n", bigrams->nmeb, trigrams->nmeb, quadgrams->nmeb);

    int num_rotors = 0;
    FILE* templates_file = fopen("data/rotors", "r");
    r_template *templates = load_templates_from_file(templates_file, &num_rotors);
    fclose(templates_file);
    FILE* source_file = fopen("data/secret", "r");
    char *contents = read_in_line(source_file);
    fclose(source_file);

    //printf("%s\n", contents);
    
    //double *scores[5][5][26][26][26] = calloc(5*5*5*26*26*26, sizeof(double));
    rotor_config_and_score *rp_scores = calloc(BEST_ROTORS_COUNT, sizeof(rotor_config_and_score));
    for (int i = 0; i < BEST_ROTORS_COUNT; i++) {
        rp_scores[i].score = -1e8;
    }

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
                            r_template rotors[3] = {templates[i], templates[j], templates[k]};
                            char positions[3] = {x + 'A', y + 'A', z + 'A'};
                            //char ring_settings[3] = {mod('A' - x, 26) + 'A', mod('A' - y, 26) + 'A', mod('A' - z, 26) + 'A'};
                            enigma e = create_enigma(rotors, templates[num_rotors-1], positions, ring_settings, plugboard, pairs);
                            char *result = encode_message(contents, e);
                            double score = 1/fabs(ioc(result) - IOC_ENGLISH_TEXT);
                            //double score = ngram_score(result, bigrams, 2);
                            free(result);
                            rotor_config_and_score rs;
                            rs.rotors[0] = i;
                            rs.rotors[1] = j;
                            rs.rotors[2] = k;
                            strncpy(rs.ring_settings, ring_settings, 3);
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

    for (int i = 0; i < BEST_ROTORS_COUNT; i++) {
        printf("%2d: %5s %5s %5s in position %c %c %c with score %f\n\n", i+1, r_id_lookup[rp_scores[i].rotors[0]], r_id_lookup[rp_scores[i].rotors[1]], r_id_lookup[rp_scores[i].rotors[2]], rp_scores[i].positions[0], rp_scores[i].positions[1], rp_scores[i].positions[2], rp_scores[i].score);
    }

    rotor_config_and_score *rs_scores = calloc(BEST_ROTORS_COUNT, sizeof(rotor_config_and_score));
    for (int i = 0; i < BEST_ROTORS_COUNT; i++) {
        rs_scores[i].score = -1e8;
    }

    //This was my original approach of cracking all ring settings at once
    for (int i = 0; i < BEST_ROTORS_COUNT; i++) {
        rotor_config_and_score best = {0};
        best.score = -1e8;
        r_template rotors[3] = {templates[rp_scores[i].rotors[0]], templates[rp_scores[i].rotors[1]], templates[rp_scores[i].rotors[2]]};
        for (int x = 0; x < 26; x++) {
            for (int y = 0; y < 26; y++) {
                for (int z = 0; z < 26; z++) {
                    char ring_settings[3] = {x + 'A', y + 'A', z + 'A'};
                    enigma e = create_enigma(rotors, templates[num_rotors-1], rp_scores[i].positions, ring_settings, plugboard, pairs);
                    char *result = encode_message(contents, e);
                    double score = ngram_score(result, bigrams, 2);
                    free(result);
                    if (score > best.score) {
                        memcpy(&best.rotors, &rp_scores[i].rotors, 3*sizeof(int));
                        strncpy(best.ring_settings, ring_settings, 3);
                        strncpy(best.positions, rp_scores[i].positions, 3);
                        strcpy(best.plugboard, plugboard);
                        strcpy(best.pairs, pairs);
                        best.score = score;
                    }
                    destroy_engima(e);
                }
            }
        }
        handle_new_score(rs_scores, BEST_ROTORS_COUNT, best);
    }
    free(rp_scores);

    // Will now implement computerphile approach of doing one at a time

    // for (int i = 0; i < BEST_ROTORS_COUNT; i++) {
    //     rotor_config_and_score best = {0};
    //     best.score = -1e8;
    //     r_template rotors[3] = {templates[rp_scores[i].rotors[0]], templates[rp_scores[i].rotors[1]], templates[rp_scores[i].rotors[2]]};
    //     char partial_config[3] = {'A', 'A', 'A'};
    //     double final_score;
    //     for (int j = 0; j < 3; j++) {
    //         double best_score = -1e8;
    //         char best_ring_setting = 'A';
    //         for (int x = 0; x < 26; x++) {
    //             char ring_settings[3];
    //             memcpy(ring_settings, partial_config, 3);
    //             ring_settings[j] = x + 'A';
    //             enigma e = create_enigma(rotors, templates[num_rotors-1], rp_scores[i].positions, ring_settings, plugboard, pairs);
    //             char *result = encode_message(contents, e);
    //             double score = ngram_score(result, bigrams, 2);
    //             //double score = 1/fabs(ioc(result) - IOC_ENGLISH_TEXT);
    //             free(result);
    //             if (score > best_score) {
    //                 best_score = score;
    //                 best_ring_setting = ring_settings[j];
    //             }
    //             destroy_engima(e);
    //         }
    //         partial_config[j] = best_ring_setting;
    //         if (j == 2) {
    //             final_score = best_score;
    //         }
    //     }
    //     memcpy(&best.rotors, &rp_scores[i].rotors, 3*sizeof(int));
    //     strncpy(best.ring_settings, partial_config, 3);
    //     strncpy(best.positions, rp_scores[i].positions, 3);
    //     strcpy(best.plugboard, plugboard);
    //     strcpy(best.pairs, pairs);
    //     best.score = final_score;
    //     handle_new_score(rs_scores, BEST_ROTORS_COUNT, best);
    // }

    printf("Best: %5s %5s %5s in position %c %c %c and ring setting %c %c %c with score %f\n\n",  r_id_lookup[rs_scores[0].rotors[0]], r_id_lookup[rs_scores[0].rotors[1]], r_id_lookup[rs_scores[0].rotors[2]],
                                                                                                rs_scores[0].positions[0], rs_scores[0].positions[1], rs_scores[0].positions[2],
                                                                                                rs_scores[0].ring_settings[0], rs_scores[0].ring_settings[1], rs_scores[0].ring_settings[2],
                                                                                                rs_scores[0].score);

    rotor_config_and_score config = rs_scores[0];
    free(rs_scores);

    r_template rotors[3] = {templates[config.rotors[0]], templates[config.rotors[1]], templates[config.rotors[2]]};

    // Add this step to try and deal with positions and ring settings being incorrectly set
    rotor_config_and_score final_config = {0};
    final_config.score = -1e8;
    memcpy(final_config.rotors, config.rotors, 3*sizeof(int));
    memcpy(final_config.plugboard, config.plugboard, 27);
    memcpy(final_config.pairs, config.pairs, 30);

    for (int i = 0; i < 26; i++) {
        for (int j = -1; j < 2; j+=2) {
            char positions[3];
            char ring_settings[3];
            memcpy(positions, config.positions, 3);
            memcpy(ring_settings, config.ring_settings, 3);
            for (int x = 0; x < 3; x++) {
                positions[x] = mod(positions[x] + i, 26);
                ring_settings[x] = mod(ring_settings[x] + (j*i), 26);
            }
            enigma e = create_enigma(rotors, templates[num_rotors-1], positions, ring_settings, plugboard, pairs);
            char *result = encode_message(contents, e);
            destroy_engima(e);
            double score = ngram_score(result, trigrams, 3);
            if (score > final_config.score) {
                final_config.score = score;
                memcpy(final_config.positions, positions, 3);
                memcpy(final_config.ring_settings, ring_settings, 3);
            }
        }
    }

    // Now it's time to try and conquer the plugboard with the power of quadgrams

    //memcpy(final_config.positions, "ZDV", 3);
    //memcpy(final_config.ring_settings, "XDA", 3);

    
    for (int i = 0; i < NUM_PLUGS; i++) {
        char best_pair[3] = "";
        double best_score = -1e8;
        char temp_plugboard[27];
        for (int x = 0; x < 26; x++) {
            char a = x + 'A';
            if (!strchr(pairs, a)) {
                for (int y = x+1; y < 26; y++) {
                    char b = y + 'A';
                    if (!strchr(pairs, b)) {
                        strncpy(temp_plugboard, plugboard, 27);
                        temp_plugboard[x] = b;
                        temp_plugboard[y] = a;
                        enigma e = create_enigma(rotors, templates[num_rotors-1], final_config.positions, final_config.ring_settings, temp_plugboard, pairs);
                        char *result = encode_message(contents, e);
                        destroy_engima(e);
                        double score = ngram_score(result, quadgrams, 4);
                        //free(result);
                        if (score > best_score) {
                            best_score = score;
                            best_pair[0] = a;
                            best_pair[1] = b;
                            best_pair[2] = ' ';
                            //printf("New best pair %c%c with score %lf\nResult: %s\n\n", a, b, score, result);
                        }
                        free(result);
                    }
                }
            }
        }
        plugboard[best_pair[0] - 'A'] = best_pair[1];
        plugboard[best_pair[1] - 'A'] = best_pair[0];
        memcpy(&pairs[3*i], best_pair, 3);
    }

    #if NUM_PLUGS != 0
        pairs[3*(NUM_PLUGS)-1] = '\0';
    #endif

    // Now it's just time to output results.
    enigma e = create_enigma(rotors, templates[num_rotors-1], final_config.positions, final_config.ring_settings, plugboard, pairs);
    printf("Final solution:\n");
    display_config(e);
    char *result = encode_message(contents, e);

    printf("Final decryption: %s\n", result);

    destroy_engima(e);
    free(result);

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
        double this_ngram_score = 0;
        void *result = get_value_from_hashmap(ngrams, key, n);
        if (result) {
            //this_ngram_score = log(*(double*) result);
            //this_ngram_score = pow(*(double*) result, (double) n);
            this_ngram_score = *(double*) result;
        } 
        else {
            //this_ngram_score = log((double) 1/ngrams->nmeb);
            this_ngram_score = -10;
        }
        final_score += this_ngram_score;
    }
    return final_score;
}

hashmap load_ngram_scores(FILE *f) {
    char *input_buffer = calloc(64, sizeof(char));
    //linked_list ngrams_list = new_linked_list();
    //linked_list frequencies_list = new_linked_list();
    hashmap h = new_hashmap(500, MAX_HASHMAP_LOAD_FACTOR, string_equality, simple_hash);
    //unsigned long combined_frequency = 0;
    while (!feof(f)) {
        char* result = fgets(input_buffer, 64, f);
        if (!result) {
            continue;
        }
        if (input_buffer[0] == '\n') {
            break;
        }
        char *new_ngram = calloc(8, sizeof(char));
        //int *new_frequency = malloc(sizeof(int));
        double *new_score = malloc(sizeof(double));
        sscanf(input_buffer, "%7s %lf", new_ngram, new_score);
        //combined_frequency += *new_frequency;
        //add_to_linked_list(ngrams_list, new_ngram);
        //add_to_linked_list(frequencies_list, new_frequency);
        int succ = add_to_hashmap(h, new_ngram, strlen(new_ngram), new_score);
        if (!succ) {
            fprintf(stderr, "Could not add key %s with value %f to hashmap.\n\n", new_ngram, *new_score);
            free(new_ngram);
            free(new_score);
        }
    }
    free(input_buffer);
    // char **ngrams = (char**) linked_list_to_array(ngrams_list);
    // int **frequencies = (int**) linked_list_to_array(frequencies_list);
    // for (int i = 0; i < ngrams_list->length; i++) {
    //     char *key = malloc(8);
    //     strncpy(key, ngrams[i], 8);
    //     double *val = malloc(sizeof(double));
    //     *val = (double)*frequencies[i]/combined_frequency;
    //     int succ = add_to_hashmap(h, key, strlen(key), val);
    //     if (!succ) {
    //         fprintf(stderr, "Could not add key %s with value %f to hashmap.\n\n", key, *val);
    //         free(key);
    //         free(val);
    //     }
    // }
    // free(ngrams);
    // free(frequencies);
    // destroy_linked_list(ngrams_list);
    // destroy_linked_list(frequencies_list);
    return h;
}