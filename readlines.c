#include "readlines.h"

char **read_lines (FILE* src, char ***lines, int *num_lines, int *lines_arr_size) {
    char input_buffer[BUFFER_SIZE]; // Create buffer to receive input
    while (!feof(src)) { // Until we reach EOF
        char *success = fgets(input_buffer, BUFFER_SIZE, src); // Read from input stream
        if (success == NULL) { // Check we haven't just read to EOF
            break; // If we have, stop looping
        }
        int line_length = strlen(input_buffer); // Get the length of the string
        char *new_line = malloc(line_length + 1); // Allocate just enough memory for it (+1 for the null character)
        if (new_line == NULL) { // If memory allocation fails
            fprintf(stderr, "Error: failed to allocate memory when reading in string!");
            exit(1); // Complain and exit
        }
        strcpy(new_line, input_buffer); // Copy from input buffer to the newly allocated memory
        if (*num_lines >= *lines_arr_size) { // If we need to expand the array
            *lines = realloc(*lines, (*lines_arr_size+LINES_ARR_LEN)*sizeof(char*)); // Expand it
            if (lines == NULL) { // Complain and exit if realloc fails
                fprintf(stderr, "Error: failed to allocate memory when reading in string!");
                exit(1);
            }
            *lines_arr_size += LINES_ARR_LEN; // Update with new array size
        }
        (*lines)[*num_lines] = new_line; // Add the new line to the array
        (*num_lines)++; // Increment the counter of items in the array
    }
    return *lines; // Return a pointer to the array
}

void destroy_lines(char **lines, int num_lines) {
    for (int i = 0; i < num_lines; i++) {
        free(lines[i]);
    }
    free(lines);
}