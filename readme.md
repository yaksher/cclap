Takes up to 1024 command line arguments and parses them.

The names functions in this file are generated by macro `CLAP_PREFIXED(.)`, 
which is defined by default to add the prefix `cclap_`. It is however, possible
to include the `cclap.h` file multiple times, redefining `CLAP_PREFIXED` to a
different prefix each time.

It returns a value of type `ARGS_T *`, which is defined to be
`CLAP_PREFIXED(args_t) *`.

The `PARSE` and `ARGS_T` macros are undefined within this file; in order to use
this function, you must use the expanded value, as governed by `CLAP_PREFIXED`

The pointer returned by `PARSE` should be passed to `ARGS_DESTROY`
(`CLAP_PREFIXED(args_destroy)`), which will deallocate all memory allocated by
it. All pointers are heap allocated using `malloc`. Ownership of the pointers
can be transferred out of the `ARGS_T` struct without double free by setting
the field in the struct to `NULL`.



The fields of ARGS_T are
- `char *process_name`: the first argument in argv
- `char **extras`: all unprocessed arguments. This array is `NULL`-terminated.
- `size_t num_extras`: the number of unprocessed arguments
- additional fields governed by the value of the CLAP_ARGS macro when this
     header file is included (further information below).



When this file is included, the CLAP_ARGS macro should consist of 0 or more
whitespace separated invocations to the following macros:
- `POSITIONAL(type, name)`: `type` is a non-bool supported type, name is a
     valid C identifier which is used to access the positional argument in
     the `ARGS_T` struct.
- `NAMED(type, name, short)`: `type` is a supported type, name is a valid C
     identifier as above; in this case, name is also used to specify the
     option in the arguments, as `--name`. If this option is not a flag,
     the following argument is used as the value. `short` must be a character
     literal and specifies the short form of the name, accessed by `-short`
- `NAMED_LONG(type, name)`: as above, but defining an argument without a
     short name
- `NAMED_SHORT(type, name, short)`: as above, but defining an argument
     without a long name. The name field is still used to access the field
     in the struct, however.

The currently supported type values are:
- `char *`: the option is a string and should be taken as-is.
- `long *`: this specifies that the option should be parsed as a number.
     If an argument is expected to be a number but is not, `PARSE` returns
     NULL. Because every argument is optional, this is a pointer.
- `bool`: this specifies that the option is a flag. This type cannot be used
     for positional arguments and does not take the next argument. The
     corresponding field in the struct is set to true if the flag appears in
     the arguments and false otherwise.

All values are optional, meaning that it is the caller's job to verify that
non-bool fields of the `ARGS_T` struct as non-`NULL` before using them.

`PARSE` will return `NULL` in one of three cases: 
- an argument (positional or named) which expected a number got an argument 
     that could not fully be parsed as a number
- a non-bool named option was present without a corresponding argument to
     read as its value.
- an internal allocation fails



Arg parsing proceeds as follows:
- The first argument is placed into the `process_name` field.
- Named arguments are parsed next:
  - when a `--` is encountered, this is parsed as a long name, which grabs the
     the next argument as its value if it is recognized and its not a flag.
     If it isn't recognized, it is skipped for now.
  - when a `-` is encountered not followed by a second one, this is treated
     as a SEQUENCE of short names.
    - Recognized flags are set to `true`
    - Recognized non-flag options take their values from the following
         arguments, in order (so if `a` and `b` are both longs then
         `-ab 10 20` assigns the value `10` to `a` and `20` to `b`)
    - Unrecognized characters are concatenated together and left to be parsed
         together as a positional argument, or an extra argument, with a `-`
         prefix. This means that if `a` is a known flag but no others are,
         `-abcd` sets `a` to `true` and leaves `-bcd` for future consideration.
         It also means that `-100` is correctly left untouched as long as no
         digits are defined to be shortnames, so positional arguments can be
         negative numbers.
- Left over arguments from named parsing are read into positional arguments
     in order. If there are fewer left over arguments than positionals,
     the tail is left missing (`NULL`).
- Any arguments not parsed as named or positional are placed into the `extra`
     field and `num_extra` is set appropriately. `extra` is also 
     `NULL`-terminated.

An example usage is:
```c
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
}
```