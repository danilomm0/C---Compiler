// Cumhur Aygar
// CSC 453
// parser.c : parser for assg1

#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

extern char back(char curr);
extern int get_token(); /* from the scanner */
extern char *lexeme;
extern int linenum;
extern int chk_decl_flag;

int tok;
char *prev;

typedef struct Symbols
{
    char *name;

    // Type 0 == Function
    // Type 1 == Variable

    int type;
    int size;
    int offset;
    struct Symbols *next;
} table;

// Index 0 == Global scope
// Index 1 == Local scope
table *symbol_table[2] = {NULL, NULL};

table *lookup(char *name, int scope)
{
    table *temp = symbol_table[scope];
    while (temp != NULL)
    {
        if (chk_decl_flag)
        {
            if (strcmp(temp->name, name) != 0)
            {
                return temp;
            }
        }
        temp = temp->next;
    }
    return NULL;
}

table *add_to_table(char *name, int type, int size, int offset, int scope)
{
    table *new = (table *)malloc(sizeof(table));

    if (lookup(name, scope) != NULL)
    {
        fprintf(stderr, "Error: %s already declared\n", name);
        exit(1);
    }
    if (new == NULL)
    {
        fprintf(stderr, "Error: malloc failed\n");
        exit(1);
    }

    new->name = strdup(name);
    new->type = type;
    new->size = size;
    new->offset = offset;
    new->next = symbol_table[scope];
    symbol_table[scope] = new;
    return new;
}

struct Func
{
    int type;
    char *name;
    struct Var *vars;
    int num_vars;
};

// Define prototypes for all functions so we don't run into any "undeclared function" errors.
void fn_call();
void stmt();
void opt_formals();
void opt_var_decls();
void opt_stmt_list();
void opt_expr_list();
void func_defn();
void prog();
int type();
void match(int expected);
int parse();
void var_decl();
void expr();
void id_list();
void formals();

void match(int expected)
{
    // printf("Expected: %d, Got: %d\n", expected, tok);
    // printf("After match:");
    if (tok == expected)
    {
        prev = lexeme;
        tok = get_token();
    }
    else
    {
        fprintf(stderr, "Syntax error on line %d: expected %d, got %d\n", linenum, expected, tok);
        exit(1);
    }
}

int type()
{
    // printf("Checking type");
    match(kwINT);
    return 0;
}

void expr()
{
    match(ID);
    match(SEMI);
}

void opt_expr_list()
{
    if (tok == ID)
    {
        match(ID);
        while (tok == COMMA)
        {
            match(COMMA);
            expr();
        }
    }
}

void fn_call()
{
    match(ID);
    if (chk_decl_flag)
    {
        table *temp = lookup(prev, 0);
        if (temp == NULL)
        {
            temp = lookup(prev, 1);
        }

        if (temp == NULL)
        {
            fprintf(stderr, "Scope error on line %d:\n", linenum);

            fprintf(stderr, "Got id: %s", lexeme);
            exit(1);
        }

        if (temp->type != 0)
        {
            fprintf(stderr, "Scope error on line %d:\n", linenum);
            fprintf(stderr, "ID: %s is not a function", temp->name);
            exit(1);
        }
    }

    // Check if function exists
    match(LPAREN);
    // opt_expr_list();
    match(RPAREN);
}

void id_list()
{
    // printf("In id_list\n");

    match(ID);
    add_to_table(prev, 1, 0, 0, 1);

    // printf("Checking ID_LIST\n");
    while (tok == COMMA)
    {
        match(COMMA);
        match(ID);
        add_to_table(prev, 1, 0, 0, 1);
    }
}

void var_decl()
{
    match(kwINT);
    // printf("In var_decl\n");
    id_list();
    match(SEMI);
}

void stmt()
{
    fn_call();
    match(SEMI);
}

void opt_formals()
{
    if (tok != RPAREN)
    {
        formals();
    }
}

void formals()
{

    // printf("Matching TYPE\n");

    type();

    // printf("Matching ID\n");

    match(ID);

    add_to_table(prev, 1, 0, 0, 1);

    while (tok == COMMA)
    {
        // printf("Matching COMMA\n");

        match(COMMA);
        // printf("Matching TYPE\n");

        type();

        // printf("Matching ID\n");

        match(ID);
        add_to_table(prev, 1, 0, 0, 1);
    }
}

void opt_var_decls()
{
    // printf("in opt_var_decls\n");
    // printf("Token: %d\n", tok);

    // printf("Getting in var_decl\n");
    while (tok == kwINT)
    {
        var_decl();
    }
}

void opt_stmt_list()
{
    while (tok == ID)
    {
        stmt();
    }
}

void func_defn()
{
    // printf("Matching LPAREN\n");
    match(LPAREN);

    // printf("In formals\n");
    opt_formals();
    // printf("Matching RParen\n");
    match(RPAREN);

    // printf("Matching LBrace\n");

    match(LBRACE);

    opt_var_decls();

    opt_stmt_list();

    match(RBRACE);
}

void prog()
{
    while (tok != EOF)
    {
        // printf("Matching INT\n");
        match(kwINT);
        // printf("Matching ID\n");
        match(ID);

        if (tok == LPAREN)
        {

            add_to_table(prev, 0, 0, 0, 0);

            // printf("Matching FUNC\n");
            func_defn();
        }
        else
        {
            // printf("Matching OPT_VAR\n");
            // printf("Token: %d\n", tok);
            add_to_table(prev, 1, 0, 0, 0);

            while (tok == COMMA)
            {
                match(COMMA);
                match(ID);
                add_to_table(prev, 1, 0, 0, 0);
            }
            match(SEMI);
        }
    }
}

int parse()
{
    tok = get_token();
    prog();
    return 0;
}