#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "enigma.h"

double ioc(char *text);
char *read_in_line(FILE *input_file);

int main() {
    int num_rotors = 0;
    FILE* templates_file = fopen("rotors", "r");
    r_template *templates = load_templates_from_file(templates_file, &num_rotors);
    fclose(templates_file);
    FILE* source_file = fopen("secret", "r");
    char *contents = read_in_line(source_file);
    printf("%s\n%f\n", contents, ioc(contents));
    free(contents);
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