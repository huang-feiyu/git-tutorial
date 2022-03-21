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
    {"\b[0-9]+\b", NUM},
    {"\b0[xX][0-9a-fA-F]+\b", HEX},
    {"\$[a-zA-Z]+", REG},
    {"\+", '+'},
    {"-", '-'}, // minus or negative
    {"\*", '*'}, // multiply or De-reference
    {"/", '/'},
    {"\(", '('},
    {"\)", ')'},
    {"&&", AND},
    {"\|\|", OR},
    {"!", NOT},
    {"\b[a-zA-Z]+\b", LETTER}
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX];

void is_op(int);

/* Rules are used for more times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
    int i;
    char error_msg[128];
    int ret;

    for (i = 0; i < NR_REGEX; i++) {
        ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
        if (ret != 0) {
            regerror(ret, &re[i], error_msg, 128);
            assert(ret != 0);
        }
    }
}

typedef struct token {
    int type;
    char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
    int position = 0;
    int i;
    regmatch_t pmatch;

    nr_token = 0;

    while (e[position] != '
