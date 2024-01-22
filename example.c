#include <stdio.h>
#include <assert.h>

#define CCLAP_ARGS \
    POSITIONAL(long *, test_num)    \
    NAMED(char *, test_str, 't', "Invokes baz.")    \
    NAMED(bool, test_flag_1, 'f', "Tells foo to be bar.")   \
    NAMED(bool, test_flag_2, 'g')   \
    NAMED(bool, help, 'h', "Prints this description.")

#include "cclap.h"


int main(int argc, char *argv[]) {
    cclap_args_t *args = cclap_parse(argc, argv);
    assert(args);
    if (args->help) {
        cclap_fprint_descriptions(stdout);
        return 0;
    }
    if (args->test_num) {
        printf("test_num = %ld\n", *args->test_num);
    }
    if (args->test_str) {
        printf("--test_str (alias -t) = `%s`\n", args->test_str);
    }
    printf("--test_flag_1 (alias -f) = %d\n", args->test_flag_1);
    printf("--test_flag_2 (alias -g) = %d\n", args->test_flag_2);
    if (args->num_extra > 0) {
        printf("extras (%lu):\n", args->num_extra);
        for (size_t i = 0; i < args->num_extra; i++) {
            printf("    %s\n", args->extra[i]);
        }
    }
}