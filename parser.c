#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

long eval(mpc_ast_t*);
long eval_op(long, char*, long);

/* lval => lisp value */
typedef struct lval {
    int type;
    long num;
    int err;

} lval;

/* Create Enumeration of Possible lval Types */
enum { LVAL_NUM, LVAL_ERR };

/* Create Enumeration of Possible Error Types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

int main(void)
{
    /* Create Some Parsers */
    mpc_parser_t* Signed    = mpc_new("signed");
    mpc_parser_t* Int       = mpc_new("int");
    mpc_parser_t* Float     = mpc_new("float");
    mpc_parser_t* Number    = mpc_new("number");
    mpc_parser_t* Toperator = mpc_new("toperator");
    mpc_parser_t* Operator  = mpc_new("operator");
    mpc_parser_t* Expr      = mpc_new("expr");
    mpc_parser_t* Lispy     = mpc_new("lispy");

    /* Define them with the following Language */
    mpca_lang(MPCA_LANG_DEFAULT,
            "                                                                    \
            signed        : /-?[0-9]+/ ;                                         \
            int           : /[0-9]+/ ;                                           \
            float         : <signed>+'.'<int>+ ;                                 \
            number        : <signed> | <float> ;                                 \
            toperator     : /add/ | /sub/ | /mul/ | /div/ | /mod/ | /pow/ ;      \
            operator      : '+' | '-' | '*' | '/' | '%' | '^' | <toperator> ;    \
            expr          : <number> | '(' <operator> <expr>+ ')' ;              \
            lispy         : /^/ <operator> <expr>+ /$/ ;                         \
            ",
            Signed, Int, Float, Number, Toperator, Operator, Expr, Lispy);

    puts("Lispy REPL");
    puts("Press Ctrl+c to Exit\n");

    /* Init repl */
    while(1) {
        /* Read input */
        char* input = readline("lispy> ");

        /* Write input to history */
        add_history(input);

        /* Attempt to Parse the user Input */
        mpc_result_t r;
        if (mpc_parse("<stdin>", input, Lispy, &r)) {
            /* On Success Print the AST */
            long result = eval(r.output);
            printf("%li\n", result);
            //mpc_ast_print(r.output);
            //mpc_ast_delete(r.output);
        } else {
            /* Otherwise Print the Error */
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }

        free(input);
    }

    /* Undefine and delete our parsers */
    mpc_cleanup(8, Signed, Int, Float, Number, Toperator, Operator, Expr, Lispy);
    return 0;
}

long eval(mpc_ast_t* t)
{
    /* If tagged as number return it directly. */
    if (strstr(t->tag, "number"))
        return atoi(t->contents);

    /* The operator is always second child. */
    char* op = t->children[1]->contents;

    /* Store the third child in x */
    long x = eval(t->children[2]);

    int i = 3;
    while (strstr(t->children[i]->tag, "expr")) {
        x = eval_op(x, op, eval(t->children[i]));
        i++;
    }

    return x;
}

lval eval_op(lval x, char* op, lval y)
{
    /* If either value is an error return it */
    if (x.type == LVAL_ERR) return x;
    if (y.type == LVAL_ERR) return y;

    if (strcmp(op, "+") == 0 || strcmp(op, "add") == 0 )
        return lval_num(x.num + y.num);

    if (strcmp(op, "-") == 0 || strcmp(op, "sub") == 0)
        return lval_num(x.num - y.num);

    if (strcmp(op, "*") == 0 || strcmp(op, "mul") == 0 )
        return lval_num(x.num * y.num);

    if (strcmp(op, "/") == 0 || strcmp(op, "div") == 0)
        return y.num == 0
            ? lval_err(LERR_DIV_ZERO)
            : lval_num(x.num / y.num);

    if (strcmp(op, "%") == 0 || strcmp(op, "mod") == 0)
        return lval_num(x.num % y.num);

    if (strcmp(op, "^") == 0 || strcmp(op, "pow") == 0)
        return lval_num(pow((double) x.num, y.num));

    return 0;
}

/* Create a new number type lval */
lval lval_num(long x)
{
    lval v;
    v.type = LVAL_NUM;
    v.num = x;
    return v;
}

/* Create a new error type lval */
lval lval_err(int x) {
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}

/* Print an "lval" */
void lval_print(lval v) {
    switch (v.type) {
        /* In the case the type is a number print it */
        /* Then 'break' out of the switch. */
        case LVAL_NUM:
            printf("%li", v.num); break;

        /* In the case the type is an error */
        case LVAL_ERR:
            /* Check what type of error it is and print it */
            if (v.err == LERR_DIV_ZERO)
                printf("Error: Division By Zero!");

            if (v.err == LERR_BAD_OP)
                printf("Error: Invalid Operator!");

            if (v.err == LERR_BAD_NUM)
                printf("Error: Invalid Number!");

            break;
    }
}
