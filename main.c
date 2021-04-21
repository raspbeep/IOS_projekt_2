/**
 * @name IOS Projekt 2
 * @author Pavel Kratochvil
 * login: xkrato61
 * version: V0.0
 * -std=gnu99 -Wall -Wextra -Werror -pedantic
 * -pthread, -lrt
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void print_help () {
    printf("Usage: ./proj2 NE NR TE TR\n");
    printf("        NE - the number of elves, 0 < NE < 1000\n");
    printf("        NR - the number of reindeer, 0 < NR < 20\n");
    printf("        TE - maximum time in ms the elf works on independently, 0 <= TE <= 1000\n");
    printf("        TR - maximum time in ms, after which the reindeer come from holidays to help, 0 <= RE <= 1000\n");
}

void incorrect_params_exit () {
    printf("Incorrect params format!\n\n");
    print_help();
}

int is_int (const char *string_val) {
    char *ptr;
    char *empty = "";

    double number_d = strtod(string_val, &ptr);
    if (*ptr != *empty) return 0;
    int number_i = (int)number_d;
    return number_d == number_i;
}

unsigned int str_to_int (const char *string_val) {
    char *ptr;
    return (unsigned int)strtod(string_val, &ptr);
}

int correct_arg_count(int argc, char *argv[]) {
    if (argc == 1 || !strcmp(argv[1], "--help" )) {
        print_help();
        return 0;
    }

    if (argc != 5) {
        incorrect_params_exit();
        return 0;
    }
    return 1;
}

int check_get_params(char *argv[], unsigned int *n_e, unsigned int *n_r, unsigned int *t_e, unsigned int *t_r) {
    if (!is_int(argv[1]) || !is_int(argv[1]) || !is_int(argv[1]) || !is_int(argv[4])) {
        incorrect_params_exit();
        return 0;
    }
    *n_e = str_to_int(argv[1]);
    *n_r = str_to_int(argv[2]);
    *t_e = str_to_int(argv[3]);
    *t_r = str_to_int(argv[4]);
    return 1;
}

int main(int argc, char *argv[]) {

    if (!correct_arg_count(argc, argv)) return 1;

    unsigned int n_e, n_r, t_e, t_r;

    if (!check_get_params(argv, &n_e, &n_r, &t_e, &t_r)) return 1;





    return 0;
}


