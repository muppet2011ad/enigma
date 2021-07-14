#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include "enigma.h"
#include "data_structures.h"

#define mod(x, n) ((x % n + n) %n)

#define IOC_ENGLISH_TEXT 1.73
#define ROTORS 5
#define BEST_ROTORS_COUNT 10
#define MAX_HASHMAP_LOAD_FACTOR 0.7
#define NUM_PLUGS 5
#define MAX_THREADS 12

struct rotor_config_and_score_structure {
    int rotors[3];
    char positions[3];
    char ring_settings[3];
    char plugboard[27];
    char pairs[30];
    double score;
};

typedef struct rotor_config_and_score_structure rotor_config_and_score;

struct enigma_thread_job_args_structure {
    rotor_config_and_score config;
    r_template *templates;
    int num_rotors;
    char *text;
    hashmap bigrams;
    hashmap trigrams;
    hashmap quadgrams;
};

typedef struct enigma_thread_job_args_structure *enigma_thread_job_args;

double ioc(char *text);
char *read_in_line(FILE *input_file);
void handle_new_score(rotor_config_and_score *best_list, int list_length, rotor_config_and_score new_score);
hashmap load_ngram_scores(FILE *f);
double ngram_score(char *string, hashmap ngrams, short n);
void multithread_jobs(void *job(void*), void *args[], int num_jobs, void *results[], int max_threads);
void *evaluate_rotor_config(void *a);
void *evaluate_ring_settings(void *a);
void destroy_arr_of_ptrs(void **arr, int length);

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

    // First pick rotors and their positions
    int num_rotor_jobs = (ROTORS)*(ROTORS-1)*(ROTORS-2); // Calculate total number of jobs
    enigma_thread_job_args *rotor_args = calloc(num_rotor_jobs, sizeof(enigma_thread_job_args)); // Allocate memory to store pointers to argument structs

    // Loop creates and populates structs with arguments for each thread
    int x = 0;
    for (int i = 0; i < num_rotors-1; i++) {
        for (int j = 0; j < num_rotors-1; j++) {
            if (i == j) { continue; }
            for (int k = 0; k < num_rotors-1; k++) {
                if (j == k || i == k) { continue; }
                rotor_args[x] = calloc(1, sizeof(struct enigma_thread_job_args_structure));
                //printf("%5s %5s %5s\n", r_id_lookup[i], r_id_lookup[j], r_id_lookup[k]);
                int rotors[3] = {i, j, k};
                memcpy(rotor_args[x]->config.rotors, rotors, 3*sizeof(int));
                rotor_args[x]->num_rotors = num_rotors;
                rotor_args[x]->templates = templates;
                rotor_args[x]->text = contents;
                x++;
            }
        }
    }

    printf("Trialling rotor combinations...\n");

    rotor_config_and_score **results = calloc(num_rotor_jobs, sizeof(void*)); // Array to store results of calculations
    multithread_jobs(evaluate_rotor_config, (void*) rotor_args, num_rotor_jobs, (void*) results, MAX_THREADS); // Perform multithreading
    destroy_arr_of_ptrs((void**) rotor_args, num_rotor_jobs); // Clear argument data
    for (int i = 0; i < num_rotor_jobs; i++) {
        handle_new_score(rp_scores, BEST_ROTORS_COUNT, *(results[i])); // Sort results into top 10 list
    }
    destroy_arr_of_ptrs((void**) results, num_rotor_jobs); // Clear results data

    for (int i = 0; i < BEST_ROTORS_COUNT; i++) {
        printf("%2d: %5s %5s %5s in position %c %c %c with score %f\n\n", i+1, r_id_lookup[rp_scores[i].rotors[0]], r_id_lookup[rp_scores[i].rotors[1]], r_id_lookup[rp_scores[i].rotors[2]], rp_scores[i].positions[0], rp_scores[i].positions[1], rp_scores[i].positions[2], rp_scores[i].score);
    }

    rotor_config_and_score *rs_scores = calloc(BEST_ROTORS_COUNT, sizeof(rotor_config_and_score));
    for (int i = 0; i < BEST_ROTORS_COUNT; i++) {
        rs_scores[i].score = -1e8;
    }

    enigma_thread_job_args *ring_setting_args = calloc(BEST_ROTORS_COUNT, sizeof(enigma_thread_job_args));
    for (int i = 0; i < BEST_ROTORS_COUNT; i++) {
        ring_setting_args[i] = calloc(1, sizeof(struct enigma_thread_job_args_structure));
        memcpy(&(ring_setting_args[i]->config), &(rp_scores[i]), sizeof(rotor_config_and_score));
        ring_setting_args[i]->num_rotors = num_rotors;
        ring_setting_args[i]->templates = templates;
        ring_setting_args[i]->text = contents;
        ring_setting_args[i]->bigrams = bigrams;
        ring_setting_args[i]->trigrams = trigrams;
        ring_setting_args[i]->quadgrams = quadgrams;
    }

    printf("Optimising ring settings...\n");
    results = calloc(BEST_ROTORS_COUNT, sizeof(void*));
    multithread_jobs(evaluate_ring_settings, (void*) ring_setting_args, BEST_ROTORS_COUNT, (void*) results, MAX_THREADS);
    destroy_arr_of_ptrs((void**) ring_setting_args, BEST_ROTORS_COUNT);
    for (int i = 0; i < BEST_ROTORS_COUNT; i++) {
        handle_new_score(rs_scores, BEST_ROTORS_COUNT, *(results[i]));
    }
    destroy_arr_of_ptrs((void**) results, BEST_ROTORS_COUNT);

    free(rp_scores);

    printf("Best: %5s %5s %5s in position %c %c %c and ring setting %c %c %c with score %f\n\n",  r_id_lookup[rs_scores[0].rotors[0]], r_id_lookup[rs_scores[0].rotors[1]], r_id_lookup[rs_scores[0].rotors[2]],
                                                                                                rs_scores[0].positions[0], rs_scores[0].positions[1], rs_scores[0].positions[2],
                                                                                                rs_scores[0].ring_settings[0], rs_scores[0].ring_settings[1], rs_scores[0].ring_settings[2],
                                                                                                rs_scores[0].score);

    rotor_config_and_score config = rs_scores[0];
    free(rs_scores);

    r_template rotors[3] = {templates[config.rotors[0]], templates[config.rotors[1]], templates[config.rotors[2]]};

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
                        enigma e = create_enigma(rotors, templates[num_rotors-1], config.positions, config.ring_settings, temp_plugboard, pairs);
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

    printf("Plugboard solution: %s\n\nPerforming final tweak for rotor settings...\n\n", plugboard);

    // Add this step to try and deal with positions and ring settings being incorrectly set
    rotor_config_and_score final_config = {0};
    final_config.score = -1e8;
    memcpy(final_config.rotors, config.rotors, 3*sizeof(int));
    memcpy(final_config.plugboard, plugboard, 27);
    memcpy(final_config.pairs, pairs, 30);

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
            double score = ngram_score(result, quadgrams, 4);
            free(result);
            if (score > final_config.score) {
                final_config.score = score;
                memcpy(final_config.positions, positions, 3);
                memcpy(final_config.ring_settings, ring_settings, 3);
            }
        }
    }

    // Now it's just time to output results.
    enigma e = create_enigma(rotors, templates[num_rotors-1], final_config.positions, final_config.ring_settings, plugboard, pairs);
    printf("Final solution:\n");
    display_config(e);
    char *result = encode_message(contents, e);

    printf("Final decryption: %s\n", result);

    destroy_engima(e);
    free(result);

    destroy_hashmap(bigrams, 1);
    destroy_hashmap(trigrams, 1);
    destroy_hashmap(quadgrams, 1);

    free(contents);
    free(templates);
}

void *evaluate_rotor_config(void *a) {
    enigma_thread_job_args args = (enigma_thread_job_args) a;
    r_template rotors[3] = {args->templates[args->config.rotors[0]], args->templates[args->config.rotors[1]], args->templates[args->config.rotors[2]]};
    char r_id_lookup[8][5] = {"I", "II", "III", "IV", "V", "VI", "VII", "VIII"};
    char ring_settings[3] = "AAA";
    char plugboard[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char pairs[30] = "\0";
    rotor_config_and_score *rs = calloc(sizeof(rotor_config_and_score), 1);
    rs->score = -1e8;
    memcpy(rs->rotors, args->config.rotors, 3*sizeof(int));
    strncpy(rs->ring_settings, ring_settings, 3);
    strcpy(rs->plugboard, plugboard);
    strcpy(rs->pairs, pairs);
    printf("%5s %5s %5s\n", r_id_lookup[args->config.rotors[0]], r_id_lookup[args->config.rotors[1]], r_id_lookup[args->config.rotors[2]]);
    for (int x = 0; x < 26; x++) {
        for (int y = 0; y < 26; y++) {
            for (int z = 0; z < 26; z++) {
                char positions[3] = {x + 'A', y + 'A', z + 'A'};
                enigma e = create_enigma(rotors, args->templates[args->num_rotors-1], positions, ring_settings, plugboard, pairs);
                char *result = encode_message(args->text, e);
                destroy_engima(e);
                double score = 1/fabs(ioc(result) - IOC_ENGLISH_TEXT);
                //double score = ngram_score(result, bigrams, 2);
                free(result);
                if (score > rs->score) {
                    strncpy(rs->positions, positions, 3);
                    rs->score = score;
                }
            }
        }
    }
    pthread_exit((void*) rs);
    return (void*) rs;
}

void *evaluate_ring_settings(void *a) {
    enigma_thread_job_args args = (enigma_thread_job_args) a;
    r_template rotors[3] = {args->templates[args->config.rotors[0]], args->templates[args->config.rotors[1]], args->templates[args->config.rotors[2]]};
    char r_id_lookup[8][5] = {"I", "II", "III", "IV", "V", "VI", "VII", "VIII"};
    char plugboard[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    char pairs[30] = "\0";
    rotor_config_and_score *rs = calloc(sizeof(rotor_config_and_score), 1);
    rs->score = -1e8;
    memcpy(rs->rotors, args->config.rotors, 3*sizeof(int));
    strncpy(rs->positions, args->config.positions, 3);
    strcpy(rs->plugboard, plugboard);
    strcpy(rs->pairs, pairs);
    for (int x = 0; x < 26; x++) {
        for (int y = 0; y < 26; y++) {
            for (int z = 0; z < 26; z++) {
                char ring_settings[3] = {x + 'A', y + 'A', z + 'A'};
                enigma e = create_enigma(rotors,  args->templates[args->num_rotors-1], args->config.positions, ring_settings, plugboard, pairs);
                char *result = encode_message(args->text, e);
                destroy_engima(e);
                double score = ngram_score(result, args->bigrams, 2);
                free(result);
                if (score > rs->score) {
                    strncpy(rs->ring_settings, ring_settings, 3);
                    rs->score = score;
                }
            }
        }
    }
    pthread_exit((void*) rs);
    return (void*) rs;
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

void multithread_jobs(void *job(void*), void *args[], int num_jobs, void *results[], int max_threads) {
    pthread_t *threads = calloc(max_threads, sizeof(pthread_t));
    for (int i = 0; i < num_jobs; i += max_threads) {
        int jobs_in_batch = num_jobs - i;
        if (jobs_in_batch > max_threads) { jobs_in_batch = max_threads; }
        for (int j = 0; j < jobs_in_batch; j++) {
            pthread_create(&(threads[j]), NULL, job, args[i+j]);
        }
        for (int j = 0; j < jobs_in_batch; j++) {
            pthread_join(threads[j], &(results[i+j]));
        }
    }
    free(threads);
}

void destroy_arr_of_ptrs(void **arr, int length) {
    for (int i = 0; i < length; i++) {
        free(arr[i]);
    }
    free(arr);
}