/*
 * File: scanner.c
 * Author: Danilo Malo-Molina
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "scanner.h"

char buffer[128];
char *lexeme;
int line = 1;

int getch()
{
    int temp = getchar();
    if (temp == '\n')
        line++;
    return temp;
}

void putch(char temp)
{
    if (temp == '\n')
        line--;
    ungetc(temp, stdin);
}

int get_token()
{
    char current, next;
    int i, loop;
    do
    {
        current = getch();
        while (current != EOF && isspace(current))
            current = getch();
        if (current == EOF)
            loop = 1;
        else if (current == '/')
        {
            next = getch();
            if (next == '*')
            {
                loop = 1;
                while (next != EOF && loop)
                {
                    while ((next = getch()) != '*')
                    {
                    }
                    while (next == '*')
                        next = getch();
                    if (next == '/')
                        loop = 0;
                }
            }
            else
            {
                putch(next);
                putch(current);
                loop = 1;
            }
        }
        else
        {
            putch(current);
            loop = 1;
        }
    } while (!loop);
    while ((current = getch()) != EOF)
    {
        if (isalpha(current))
        {
            i = 0;
            while (isalnum(current) || current == '_')
            {
                buffer[i++] = current;
                current = getch();
            }
            buffer[i] = '\0';
            if (current != EOF)
                putch(current);
            lexeme = strdup(buffer);
            if (!strcmp(buffer, "int"))
                return kwINT;
            else if (!strcmp(buffer, "if"))
                return kwIF;
            else if (!strcmp(buffer, "else"))
                return kwELSE;
            else if (!strcmp(buffer, "while"))
                return kwWHILE;
            else if (!strcmp(buffer, "return"))
                return kwRETURN;
            else
                return ID;
        }
        else if (isdigit(current))
        {
            i = 0;
            while (isdigit(current))
            {
                buffer[i++] = current;
                current = getch();
            }
            buffer[i] = '\0';
            if (current != EOF)
                putch(current);
            lexeme = strdup(buffer);
            return INTCON;
        }
        else
        {
            switch (current)
            {
            case '(':
                lexeme = "(";
                return LPAREN;
            case ')':
                lexeme = ")";
                return RPAREN;
            case '{':
                lexeme = "{";
                return LBRACE;
            case '}':
                lexeme = "}";
                return RBRACE;
            case ',':
                lexeme = ",";
                return COMMA;
            case ';':
                lexeme = ";";
                return SEMI;
            case '+':
                lexeme = "+";
                return opADD;
            case '-':
                lexeme = "-";
                return opSUB;
            case '*':
                lexeme = "*";
                return opMUL;
            case '/':
                lexeme = "/";
                return opDIV;
            case '=':
                next = getch();
                if (next == '=')
                {
                    lexeme = "==";
                    return opEQ;
                }
                else
                {
                    putch(next);
                    lexeme = "=";
                    return opASSG;
                }
            case '!':
                next = getch();
                if (next == '=')
                {
                    lexeme = "!=";
                    return opNE;
                }
                else
                {
                    putch(next);
                    lexeme = "!";
                    return opNOT;
                }
            case '<':
                next = getch();
                if (next == '=')
                {
                    lexeme = "<=";
                    return opLE;
                }
                else
                {
                    putch(next);
                    lexeme = "<";
                    return opLT;
                }
            case '>':
                next = getch();
                if (next == '=')
                {
                    lexeme = ">=";
                    return opGE;
                }
                else
                {
                    putch(next);
                    lexeme = ">";
                    return opGT;
                }
            case '|':
                next = getch();
                if (next == '|')
                {
                    lexeme = "||";
                    return opOR;
                }
                else
                {
                    fprintf(stderr, "Illegal character: %c\n", next);
                    exit(1);
                }
            case '&':
                next = getch();
                if (next == '&')
                {
                    lexeme = "&&";
                    return opAND;
                }
                else
                {
                    fprintf(stderr, "Illegal character: %c\n", next);
                    exit(1);
                }
            default:
                fprintf(stderr, "Illegal character: %c\n", current);
                exit(1);
            }
        }
    }
    lexeme = "";
    return EOF;
}
