#include "nemu.h"
#include "cpu/reg.h"
#include "memory/memory.h"

#include <stdlib.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
    NOTYPE = 256, // <SPACE>
    EQ,
    NEQ,
    NUM,
    HEX,
    REG,
    AND,
    OR,
    NOT,
    LETTER,
    DEREF,
    NEG
};

static struct rule {
    char *regex;
    int token_type;
} rules[] = {
    {" +", NOTYPE}, // white space
    {"==", EQ},
    {"!=", NEQ},
    {"\\b[0-9]+\\b", NUM},
    {"\\b0[xX][0-9a-fA-F]+\\b", HEX},
    {"\\$[a-zA-Z]+", REG},
    {"\\+", '+'},
    {"-", '-'}, // minus or negative
    {"\\*", '*'}, // multiply or De-reference
    {"/", '/'},
    {"\\(", '('},
    {"\\)", ')'},
    {"&&", AND},
    {"\\|\\|", OR},
    {"!", NOT},
    {"\\b[a-zA-Z]+\\b", LETTER}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))
