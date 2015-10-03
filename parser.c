#include <stdio.h>
#include <stdlib.h>
#include "mpc/mpc.h"

#ifdef _WIN32
#include <string.h>

static char buffer[2048];

/* readline function for windows */
char* readline(char* prompt) {
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = malloc(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}

/* Fake add_history function */
void add_history(char* unused) {}

#else
#include <editline/readline.h>
#include <editline/history.h>
#endif

int main(void)
{
    /* Create Some Parsers */
    mpc_parser_t* Number    = mpc_new("number");
    mpc_parser_t* Toperator = mpc_new("toperator");
    mpc_parser_t* Operator  = mpc_new("operator");
    mpc_parser_t* Expr      = mpc_new("expr");
    mpc_parser_t* Lispy     = mpc_new("lispy");

    /* Define them with the following Language */
    mpca_lang(MPCA_LANG_DEFAULT,
            "                                                                    \
            number        : /-?[0-9]+/ ;                                         \
            toperator     : /add/ | /sub/ | /mul/ | /div/ ;                      \
            operator      : '+' | '-' | '*' | '/' | '%' | <toperator> ;          \
            expr          : <number> | '(' <operator> <expr>+ ')' ;              \
            lispy         : /^/ <operator> <expr>+ /$/ ;                         \
            ",
            Number, Toperator, Operator, Expr, Lispy);

    puts("Lispy REPL");
    puts("Press Ctrl+c to Exit\n");

    // Init repl
    while(1) {
        // Read input
        char* input = readline("lispy> ");

        // Write input to history
        add_history(input);

        /* Attempt to Parse the user Input */
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            /* On Success Print the AST */
            mpc_ast_print(r.output);
            mpc_ast_delete(r.output);
        } else {
            /* Otherwise Print the Error */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);

    }

    /* Undefine and delete our parsers */
    mpc_cleanup(5, Number, Toperator, Operator, Expr, Lispy);
    return 0;
}
