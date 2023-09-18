#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#ifndef CLAP_PREFIXED
#define CLAP_PREFIXED(TOK) cclap_ ## TOK
#endif

#define ARGS_T CLAP_PREFIXED(args_t)
#define PARSE_POSITIONAL CLAP_PREFIXED(_parse_positional_)
#define PARSE_LONG CLAP_PREFIXED(_parse_long_)
#define PARSE_SHORT CLAP_PREFIXED(_parse_short_)
#define PARSE_NAMED CLAP_PREFIXED(_parse_named_)
#define ARGS_DESTROY CLAP_PREFIXED(args_destroy)
#define PARSE CLAP_PREFIXED(parse)


#define POSITIONAL(TYPE, NAME) TYPE NAME;
#define NAMED(TYPE, NAME, ...) TYPE NAME;
#define NAMED_SHORT NAMED
#define NAMED_LONG NAMED
typedef struct ARGS_T {
    char *proccess_name;
    CCLAP_ARGS
    size_t num_extra;
    char **extra;
} ARGS_T;
#undef POSITIONAL
#undef NAMED
#undef NAMED_SHORT
#undef NAMED_LONG

static int PARSE_POSITIONAL(ARGS_T *args) {
    char **extra = args->extra;

#define NAMED(...)
#define NAMED_LONG NAMED
#define NAMED_SHORT NAMED
#define POSITIONAL(TYPE, NAME)\
    if (*extra == NULL) {                               \
        goto EXIT;                                      \
    }                                                   \
    if (strcmp(#TYPE, "char *") == 0) {                 \
        *(char **) &args->NAME = *extra;                \
    }                                                   \
    if (strcmp(#TYPE, "long *") == 0) {                 \
        char *end;                                      \
        long val = strtol(*extra, &end, 10);            \
        if (*end != '\0') {                             \
            return -1;                                  \
        }                                               \
        free(*extra);                                   \
        *extra = NULL;                                  \
        *(long**)&args->NAME = malloc(sizeof(long));    \
        if (!args->NAME) {                              \
            return -1;                                  \
        }                                               \
        *(long *) args->NAME = val;                     \
        return 2;                                       \
    }                                                   \
    extra++;

    CCLAP_ARGS

#undef POSITIONAL
#undef NAMED
#undef NAMED_SHORT
#undef NAMED_LONG

    EXIT:
    return extra - args->extra;
}

static int PARSE_LONG(ARGS_T *args, char *argv[]) {
#define POSITIONAL(...)
#define NAMED_SHORT(...)
#define NAMED_LONG NAMED
#define NAMED(TYPE, NAME, ...)\
    if (strcmp(argv[0], "--" #NAME) == 0) {                 \
        if (strcmp(#TYPE, "char *") == 0) {                 \
            if (argv[1] == NULL) {                          \
                return -1;                                  \
            } else {                                        \
                *(char **) &args->NAME = strdup(argv[1]);   \
                if (!args->NAME) {                          \
                    return -1;                              \
                }                                           \
                return 2;                                   \
            }                                               \
        }                                                   \
        if (strcmp(#TYPE, "long *") == 0) {                 \
            if (argv[1] == NULL) {                          \
                return -1;                                  \
            } else {                                        \
                char *end;                                  \
                long val = strtol(argv[1], &end, 10);       \
                if (*end != '\0') {                         \
                    return -1;                              \
                }                                           \
                *(long**)&args->NAME = malloc(sizeof(long));\
                if (!args->NAME) {                          \
                    return -1;                              \
                }                                           \
                *(long *) args->NAME = val;                 \
                return 2;                                   \
            }                                               \
        }                                                   \
        if (strcmp(#TYPE, "bool") == 0) {                   \
            *(bool *) &args->NAME = true;                   \
            return 1;                                       \
        }                                                   \
    }

    CCLAP_ARGS

#undef POSITIONAL
#undef NAMED
#undef NAMED_SHORT
#undef NAMED_LONG

    args->extra[args->num_extra++] = strdup(argv[0]);
    if (args->extra[args->num_extra - 1] == NULL) {
        return -1;
    }
    return 1;
}

static int PARSE_SHORT(ARGS_T *args, char *argv[]) {
    char unused[ARG_MAX] = { 0 };
    size_t unused_i = 0;
    int args_i = 1;
    for (char *s = *argv + 1; *s;) {

#define POSITIONAL(...)
#define NAMED_LONG(...)
#define NAMED_SHORT NAMED
#define NAMED(TYPE, NAME, SHORT)\
    if (*s == SHORT) {                                      \
        s++;                                                \
        if (strcmp(#TYPE, "char *") == 0) {                 \
            if (argv[args_i] == NULL) {                     \
                return -1;                                  \
            } else {                                        \
                *(char**)&args->NAME = strdup(argv[args_i]);\
                if (!args->NAME) {                          \
                    return -1;                              \
                }                                           \
                args_i++;                                   \
            }                                               \
        }                                                   \
        if (strcmp(#TYPE, "long *") == 0) {                 \
            if (argv[args_i] == NULL) {                     \
                return -1;                                  \
            } else {                                        \
                char *end;                                  \
                long val = strtol(argv[1], &end, 10);       \
                if (*end != '\0') {                         \
                    return -1;                              \
                }                                           \
                *(long**)&args->NAME = malloc(sizeof(long));\
                if (!args->NAME) {                          \
                    return -1;                              \
                }                                           \
                *(long *) args->NAME = val;                 \
                args_i++;                                   \
            }                                               \
        }                                                   \
        if (strcmp(#TYPE, "bool") == 0) {                   \
            *(bool *) &args->NAME = true;                   \
        }                                                   \
        continue;                                           \
    }

        CCLAP_ARGS

#undef POSITIONAL
#undef NAMED
#undef NAMED_SHORT
#undef NAMED_LONG

        unused[unused_i++] = *s++;
    }
    if (unused_i > 0) {
        char *unused_heap = calloc(unused_i + 2, sizeof(char));
        if (unused_heap == NULL) {
            return -1;
        }
        *unused_heap = '-';
        strlcat(unused_heap, unused, unused_i + 2);
        args->extra[args->num_extra++] = unused_heap;
    }
    return args_i;
}

static int PARSE_NAMED(ARGS_T *args, char *argv[]) {
    if (argv[0][1] == '-') {
        return PARSE_LONG(args, argv);
    } else {
        return PARSE_SHORT(args, argv);
    }
}

/**
 * @brief Destroys an `ARGS_T` struct returned by `PARSE`. Look at documentation
 * of `CLAP_PREFIXED(parse)` for more information. `NULL` fields are skipped, so
 * ownership can be transferred out without double free by setting fields to
 * `NULL` before passing them to this function.
 */
static void ARGS_DESTROY(ARGS_T *args) {
    if (args == NULL) {
        return;
    }
    free(args->proccess_name);
    #define FREE(TYPE, NAME, ...)               \
        if (strcmp(#TYPE, "bool")) {            \
            if ((void *) args->NAME != NULL) {  \
                free((void *) args->NAME);      \
            }                                   \
        }
    #define POSITIONAL FREE
    #define NAMED FREE
    #define NAMED_SHORT FREE
    #define NAMED_LONG FREE
    CCLAP_ARGS
    #undef FREE
    #undef POSITIONAL
    #undef NAMED
    #undef NAMED_SHORT
    #undef NAMED_LONG
    if (args->extra != NULL) {
        for (size_t i = 0; i < args->num_extra; i++) {
            free(args->extra[i]);
        }
        free(args->extra);
    }
    free(args);
}

#define CLAP_MAX_SUPPORTED_ARG_COUNT 1024

/**
 * @brief Takes up to 1024 command line arguments and parses them.
 * 
 * The name of this function is generated by `CLAP_PREFIXED(parse)`, which is
 * defined by default to add the prefix `cclap_`. It is however, possible to
 * include this file multiple times, redefining CLAP_PREFIXED to a different
 * prefix each time.
 * 
 * It returns a value of type `ARGS_T *`, which is defined to be
 * `CLAP_PREFIXED(args_t) *`.
 * 
 * The PARSE and ARGS_T macros are undefined within this file; in order to use
 * this function, you must use the expanded value, as governed by `CLAP_PREFIXED`
 * 
 * The pointer returned by `PARSE` should be passed to `ARGS_DESTROY`
 * (`CLAP_PREFIXED(args_destroy)`), which will deallocate all memory allocated by
 * it. All pointers are heap allocated using `malloc.` Ownership of the pointers
 * can be transferred out of the `ARGS_T` struct without double free by setting
 * the field in the struct to `NULL`.
 * 
 *
 * 
 * The fields of ARGS_T are
 * - `char *process_name`: the first argument in argv
 * - `char **extras`: all unprocessed arguments. This array is `NULL`-terminated.
 * - `size_t num_extras`: the number of unprocessed arguments
 * - additional fields governed by the value of the CLAP_ARGS macro when this
 *      header file is included (further information below).
 * 
 * 
 * 
 * When this file is included, the CLAP_ARGS macro should consist of 0 or more
 * whitespace separated invocations to the following macros:
 * - `POSITIONAL(type, name)`: `type` is a non-bool supported type, name is a
 *      valid C identifier which is used to access the positional argument in
 *      the `ARGS_T` struct.
 * - `NAMED(type, name, short)`: `type` is a supported type, name is a valid C
 *      identifier as above; in this case, name is also used to specify the
 *      option in the arguments, as `--name`. If this option is not a flag,
 *      the following argument is used as the value. `short` must be a character
 *      literal and specifies the short form of the name, accessed by `-short`
 * - `NAMED_LONG(type, name)`: as above, but defining an argument without a
 *      short name
 * - `NAMED_SHORT(type, name, short)`: as above, but defining an argument
 *      without a long name. The name field is still used to access the field
 *      in the struct, however.
 * 
 * The currently supported type values are:
 * - `char *`: the option is a string and should be taken as-is.
 * - `long *`: this specifies that the option should be parsed as a number.
 *      If an argument is expected to be a number but is not, `PARSE` returns
 *      NULL. Because every argument is optional, this is a pointer.
 * - `bool`: this specifies that the option is a flag. This type cannot be used
 *      for positional arguments and does not take the next argument. The
 *      corresponding field in the struct is set to true if the flag appears in
 *      the arguments and false otherwise.
 * 
 * All values are optional, meaning that it is the caller's job to verify that
 * non-bool fields of the `ARGS_T` struct as non-`NULL` before using them.
 * 
 * `PARSE` will return `NULL` in one of three cases: 
 * - an argument (positional or named) which expected a number got an argument 
 *      that could not fully be parsed as a number
 * - a non-bool named option was present without a corresponding argument to
 *      read as its value.
 * - an internal allocation fails
 * 
 * 
 * 
 * Arg parsing proceeds as follows:
 * - The first argument is placed into the `process_name` field.
 * - Named arguments are parsed next:
 *   - when a `--` is encountered, this is parsed as a long name, which grabs the
 *      the next argument as its value if it is recognized and its not a flag.
 *      If it isn't recognized, it is skipped for now.
 *   - when a `-` is encountered not followed by a second one, this is treated
 *      as a SEQUENCE of short names.
 *     - Recognized flags are set to `true`
 *     - Recognized non-flag options take their values from the following
 *          arguments, in order (so if `a` and `b` are both longs then
 *          `-ab 10 20` assigns the value `10` to `a` and `20` to `b`)
 *     - Unrecognized characters are concatenated together and left to be parsed
 *          together as a positional argument, or an extra argument, with a `-`
 *          prefix. This means that if `a` is a known flag but no others are,
 *          `-abcd` sets `a` to `true` and leaves `-bcd` for future consideration.
 *          It also means that `-100` is correctly left untouched as long as no
 *          digits are defined to be shortnames, so positional arguments can be
 *          negative numbers.
 * - Left over arguments from named parsing are read into positional arguments
 *      in order. If there are fewer left over arguments than positionals,
 *      the tail is left missing (`NULL`).
 * - Any arguments not parsed as named or positional are placed into the `extra`
 *      field and `num_extra` is set appropriately. `extra` is also 
 *      `NULL`-terminated.
 * 
 * @return ARGS_T* A nullable pointer to a struct containing the parsed args.
 */
static ARGS_T *PARSE(int argc, char *argv[]) {
    if (argc > CLAP_MAX_SUPPORTED_ARG_COUNT) {
        return NULL;
    }
    ARGS_T *args = calloc(1, sizeof(ARGS_T));
    if (args == NULL) {
        return NULL;
    }
    char *extra[CLAP_MAX_SUPPORTED_ARG_COUNT + 1] = { 0 };
    args->extra = extra;
    args->proccess_name = strdup(*argv++);
    while (*argv != NULL) {
        if (**argv == '-') {
            int num_read = PARSE_NAMED(args, argv);
            if (num_read == -1) {
                goto FAIL;
            }
            argv += num_read;
        } else {
            args->extra[args->num_extra++] = strdup(*argv++);
        }
    }
    int num_pos_read = PARSE_POSITIONAL(args);
    if (num_pos_read == -1) {
        goto FAIL;
    }

    size_t num_extra = args->num_extra - num_pos_read;
    char **extra_heap = calloc(num_extra + 1, sizeof(char *));
    memcpy(extra_heap, args->extra, sizeof(char *) * num_pos_read);
    args->extra = extra_heap;
    args->num_extra = num_extra;

    return args;
    FAIL:
    for (size_t i = 0; i < args->num_extra; i++) {
        free(extra[i]);
    }
    args->extra = NULL;
    ARGS_DESTROY(args);
    return NULL;
}

#undef ARGS_T
#undef PARSE_POSITIONAL
#undef PARSE_LONG
#undef PARSE_SHORT
#undef PARSE_NAMED
#undef ARGS_DESTROY
#undef PARSE