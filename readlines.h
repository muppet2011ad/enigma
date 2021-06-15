#ifndef readlines_h

#define readlines_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 4096
#define LINES_ARR_LEN 8

char **read_lines (FILE* src, char ***lines, int *num_lines, int *lines_arr_size);

#endif