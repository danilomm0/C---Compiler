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
extern char *lexeme;
Token current;

void match(Token expected)
{
    if (current == expected)
        current = get_token();
    else
    {
        fprintf(stderr, "Error line %d, unexpected token %d\n", line, current);
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
        match(LPAREN);
        if (current == kwINT)
        {
            match(kwINT);
            match(ID);
            while (current == COMMA)
            {
                match(COMMA);
                match(kwINT);
                match(ID);
            }
        }
        match(RPAREN);
        match(LBRACE);
        while (current != RBRACE)
        {
            match(ID);
            match(LPAREN);
            if (current == ID)
            {
                match(ID);
                while (current == COMMA)
                {
                    match(COMMA);
                    match(ID);
                    match(SEMI);
                }
            }
            match(RPAREN);
            match(SEMI);
        }
        match(RBRACE);
    }
    return 0;
}