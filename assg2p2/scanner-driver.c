/*
 * File: scanner-driver.c
 * Author: Saumya Debray and Danilo Malo-Molina
 * Purpose: A modified driver from milestone 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"

char *token_name[] = {
    "UNDEF",
    "ID",
    "INTCON",
    "LPAREN",
    "RPAREN",
    "LBRACE",
    "RBRACE",
    "COMMA",
    "SEMI",
    "kwINT",
    "kwIF",
    "kwELSE",
    "kwWHILE",
    "kwRETURN",
    "opASSG",
    "opADD",
    "opSUB",
    "opMUL",
    "opDIV",
    "opEQ",
    "opNE",
    "opGT",
    "opGE",
    "opLT",
    "opLE",
    "opAND",
    "opOR",
    "opNOT",
};

extern int get_token();
extern int line;
extern int gen_code_flag;
extern int chk_decl_flag;
extern char *lexeme;
char *last;
Token current;

typedef struct symbol_table
{
    char *name;
    char *type;
    int args;
    struct symbol_table *next;
} SymTab;

SymTab *table[2] = {NULL, NULL}; // table[0] is global, table[1] is local

SymTab *search_symtab(int scope, char *name)
{
    SymTab *temp;
    for (temp = table[scope]; temp != NULL; temp = temp->next)
        if (strcmp(name, temp->name) == 0)
            return temp;
    return NULL;
}

SymTab *search_all_symtab(char *name)
{
    SymTab *temp;
    temp = search_symtab(1, name);
    if (temp != NULL)
        return temp;
    temp = search_symtab(0, name);
    if (temp != NULL)
        return temp;

    if (chk_decl_flag)
    {
        fprintf(stderr, "Error line %d, undeclared ID %s\n", line, name);
        exit(1);
    }
    return NULL;
}

SymTab *add_symtab(int scope, char *name, char *type)
{
    SymTab *temp;
    if (chk_decl_flag && search_symtab(scope, name) != NULL)
    {
        fprintf(stderr, "Error line %d, multiple declarations of %s\n", line, name);
        exit(1);
    }
    temp = malloc(sizeof(SymTab));
    if (temp == NULL)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    temp->name = name;
    temp->type = type;
    temp->next = table[scope];
    table[scope] = temp;
    return temp;
}

void match(Token expected)
{
    if (current == expected)
    {
        last = lexeme;
        current = get_token();
    }
    else
    {
        fprintf(stderr, "Error line %d, unexpected token %s\n", line, lexeme);
        exit(1);
    }
}

void arith_expr()
{
    if (current == ID)
    {
        match(ID);
        if (chk_decl_flag)
        {
            SymTab *found;
            found = search_all_symtab(last);
            if (strcmp(found->type, "function") == 0)
            {
                fprintf(stderr, "Error line %d, arithmetic on a function %s\n", line, last);
                exit(1);
            }
        }
    }
    else
        match(INTCON);
}

void stmt()
{
    if (current == kwIF)
    {
        match(kwIF);
        match(LPAREN);
        arith_expr();
        if (current == opEQ || current == opNE || current == opLE || current == opLT || current == opGE || current == opGT)
            match(current);
        else
        {
            fprintf(stderr, "Error line %d, unexpected token %s\n", line, lexeme);
            exit(1);
        }
        arith_expr();
        match(RPAREN);
        stmt();
        if (current == kwELSE)
        {
            match(kwELSE);
            stmt();
        }
    }
    else if (current == kwWHILE)
    {
        match(kwWHILE);
        match(LPAREN);
        arith_expr();
        if (current == opEQ || current == opNE || current == opLE || current == opLT || current == opGE || current == opGT)
            match(current);
        else
        {
            fprintf(stderr, "Error line %d, unexpected token %s\n", line, lexeme);
            exit(1);
        }
        arith_expr();
        match(RPAREN);
        stmt();
    }
    else if (current == ID)
    {
        match(ID);
        SymTab *found;
        found = search_all_symtab(last);
        if (current == LPAREN)
        {
            if (chk_decl_flag && strcmp(found->type, "function") != 0)
            {
                fprintf(stderr, "Error line %d, ID is not a function %s\n", line, last);
                exit(1);
            }
            match(LPAREN);
            int num = 0;
            if (current == ID || current == INTCON)
            {
                arith_expr();
                num++;
                while (current == COMMA)
                {
                    match(COMMA);
                    arith_expr();
                    num++;
                }
            }
            if (chk_decl_flag != 0 && found->args != num)
            {
                fprintf(stderr, "Error line %d, incorrect number of arguments %s\n", line, last);
                exit(1);
            }
            match(RPAREN);
        }
        else
        {
            if (chk_decl_flag && strcmp(found->type, "function") == 0)
            {
                fprintf(stderr, "Error line %d, assignment on a function %s\n", line, last);
                exit(1);
            }
            match(opASSG);
            arith_expr();
        }
        match(SEMI);
    }
    else if (current == kwRETURN)
    {
        match(kwRETURN);
        if (current == ID || current == INTCON)
            arith_expr();
        match(SEMI);
    }
    else if (current == LBRACE)
    {
        match(LBRACE);
        while (current == kwIF || current == kwWHILE || current == ID || current == kwRETURN || current == LBRACE || current == SEMI)
        {
            stmt();
        }
        match(RBRACE);
    }
    else if (current == SEMI)
    {
        match(SEMI);
    }
    else
    {
        fprintf(stderr, "Error line %d, unexpected token %s\n", line, lexeme);
        exit(1);
    }
}

int parse()
{
    current = get_token();
    while (current != EOF)
    {
        match(kwINT);
        match(ID);
        if (current == LPAREN)
        {
            SymTab *func;
            func = add_symtab(0, last, "function");
            match(LPAREN);
            int i = 0;
            if (current == kwINT)
            {
                match(kwINT);
                match(ID);
                add_symtab(1, last, "variable");
                i++;
                while (current == COMMA)
                {
                    match(COMMA);
                    match(kwINT);
                    match(ID);
                    add_symtab(1, last, "variable");
                    i++;
                }
            }
            func->args = i;
            match(RPAREN);
            match(LBRACE);
            while (current == kwINT)
            {
                match(kwINT);
                match(ID);
                add_symtab(1, last, "variable");
                while (current == COMMA)
                {
                    match(COMMA);
                    match(ID);
                    add_symtab(1, last, "variable");
                }
                match(SEMI);
            }
            while (current == kwIF || current == kwWHILE || current == ID || current == kwRETURN || current == LBRACE || current == SEMI)
            {
                stmt();
            }
            match(RBRACE);
            table[1] = NULL;
        }
        else
        {
            add_symtab(0, last, "variable");
            while (current == COMMA)
            {
                match(COMMA);
                match(ID);
                add_symtab(0, last, "variable");
            }
            match(SEMI);
        }
    }
    return 0;
}