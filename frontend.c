#include "enigma.h"

int main() {
    FILE* file = fopen("data/rotors", "r");
    int num_rotors = 0;
    r_template *templates = load_templates_from_file(file, &num_rotors);
    fclose(file);

    FILE *config_file = fopen("data/config", "r");
    enigma e = create_enigma_from_file(config_file, templates, num_rotors);
    fclose(config_file);
    free(templates);

    display_config(e);

    printf("Enter message to be encoded (ctrl+d to submit): ");
    char *plaintext = read_in_line(stdin);

    char *result = encode_message(plaintext, e);

    printf("Result: %s\n", result);
    destroy_engima(e);
    free(result);
}