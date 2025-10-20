/*
 * File: scanner-driver.c
 * Author: Saumya Debray and Danilo Malo-Molina
 * Purpose: A modified driver from milestone 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scanner.h"
#include "structs.h"
#include "ast.h"

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

AST *expr_item();
AST *expr();
AST *arith_expr();
AST *stmt();
AST *opt_stmt_list();

extern int get_token();
extern int line;
extern int gen_code_flag;
extern int chk_decl_flag;
extern int print_ast_flag;
extern char *lexeme;
char *last;
Token current;

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

void fn_call_arg(SymTab *found, AST *ast)
{
    if (chk_decl_flag && strcmp(found->type, "function") != 0)
    {
        fprintf(stderr, "Error line %d, ID is not a function %s\n", line, last);
        exit(1);
    }
    match(LPAREN);
    ast->type = FUNC_CALL;
    ast->symtab = found;
    AST *temp = NULL;
    if (current == ID || current == INTCON || current == LPAREN || current == opSUB)
    {
        temp = new_ast(EXPR_LIST);
        temp->sub0 = arith_expr(expr());
        AST *head = NULL, *tail = NULL, *temp1;
        while (current == COMMA)
        {
            match(COMMA);
            temp1 = new_ast(EXPR_LIST);
            temp1->sub0 = arith_expr(expr());
            if (head == NULL)
            {
                head = temp1;
                tail = temp1;
            }
            else
            {
                tail->sub1 = temp1;
                tail = temp1;
            }
        }
        temp->sub1 = head;
    }
    ast->sub0 = temp;
    if (chk_decl_flag != 0)
    {
        AST *temp;
        int num = 0;
        for (temp = ast->sub0; temp != NULL; temp = temp->sub1)
        {
            num += 1;
        }
        if (found->args != num)
        {
            fprintf(stderr, "Error line %d, %s expected %d arguments got %d instead %s\n", line, found->name, found->args, num, lexeme);
            exit(1);
        }
    }
    match(RPAREN);
}

AST *expr_item()
{
    AST *ast;
    if (current == ID)
    {
        match(ID);
        SymTab *found = search_all_symtab(last);
        ast = new_ast(DUMMY);
        if (current == LPAREN)
        {
            fn_call_arg(found, ast);
        }
        else
        {
            if (chk_decl_flag && strcmp(found->type, "function") == 0)
            {
                fprintf(stderr, "Error line %d, incorrect use of function %s\n", line, last);
                exit(1);
            }
            ast->type = IDENTIFIER;
            ast->symtab = found;
        }
    }
    else if (current == INTCON)
    {
        match(INTCON);
        ast = new_ast(INTCONST);
        ast->num = atoi(last);
    }
    else if (current == LPAREN)
    {
        match(LPAREN);
        ast = arith_expr(expr());
        match(RPAREN);
    }
    else if (current == opSUB)
    {
        match(opSUB);
        ast = new_ast(UMINUS);
        ast->sub0 = expr_item();
    }
    else
    {
        fprintf(stderr, "Error line %d, unexpected token %s\n", line, lexeme);
        exit(1);
    }
    return ast;
}

AST *expr()
{
    AST *ast, *temp;
    ast = expr_item();
    while (current == opMUL || current == opDIV)
    {
        if (current == opMUL)
        {
            match(opMUL);
            temp = new_ast(MUL);
        }
        else
        {
            match(opDIV);
            temp = new_ast(DIV);
        }
        temp->sub0 = ast;
        temp->sub1 = expr_item();
        ast = temp;
    }
    return ast;
}

AST *arith_expr(AST *temp)
{
    if (current == opADD || current == opSUB)
    {
        AST *ast = new_ast(DUMMY);
        if (current == opADD)
        {
            match(opADD);
            ast->type = ADD;
        }
        else
        {
            match(opSUB);
            ast->type = SUB;
        }
        ast->sub0 = temp;
        ast->sub1 = expr();
        ast = arith_expr(ast);
        return ast;
    }
    return temp;
}

AST *bool()
{
    AST *ast = new_ast(DUMMY);
    ast->sub0 = arith_expr(expr());
    if (current == opEQ)
    {
        match(opEQ);
        ast->type = EQ;
    }
    else if (current == opNE)
    {
        match(opNE);
        ast->type = NE;
    }
    else if (current == opLE)
    {
        match(opLE);
        ast->type = LE;
    }
    else if (current == opLT)
    {
        match(opLT);
        ast->type = LT;
    }
    else if (current == opGE)
    {
        match(opGE);
        ast->type = GE;
    }
    else if (current == opGT)
    {
        match(opGT);
        ast->type = GT;
    }
    else
    {
        fprintf(stderr, "Error line %d, unexpected token %s\n", line, lexeme);
        exit(1);
    }
    ast->sub1 = arith_expr(expr());
    return ast;
}

AST *bool_term()
{
    AST *ast, *temp;
    ast = bool();
    while (current == opAND)
    {
        match(opAND);
        temp = new_ast(AND);
        temp->sub0 = ast;
        temp->sub1 = bool();
        ast = temp;
    }
    return ast;
}

AST *bool_exp()
{
    AST *ast, *temp;
    ast = bool_term();
    while (current == opOR)
    {
        match(opOR);
        temp = new_ast(OR);
        temp->sub0 = ast;
        temp->sub1 = bool_term();
        ast = temp;
    }
    return ast;
}

AST *stmt()
{
    if (current == kwIF)
    {
        AST *ast = new_ast(IF);
        match(kwIF);
        match(LPAREN);
        ast->sub0 = bool_exp();
        match(RPAREN);
        ast->sub1 = stmt();
        if (current == kwELSE)
        {
            match(kwELSE);
            ast->sub2 = stmt();
        }
        return ast;
    }
    else if (current == kwWHILE)
    {
        AST *ast = new_ast(WHILE);
        match(kwWHILE);
        match(LPAREN);
        ast->sub0 = bool_exp();
        match(RPAREN);
        ast->sub1 = stmt();
        return ast;
    }
    else if (current == ID)
    {
        match(ID);
        SymTab *found = search_all_symtab(last);
        AST *ast = new_ast(DUMMY);
        if (current == LPAREN)
        {
            fn_call_arg(found, ast);
        }
        else
        {
            if (chk_decl_flag && strcmp(found->type, "function") == 0)
            {
                fprintf(stderr, "Error line %d, assignment on a function %s\n", line, last);
                exit(1);
            }
            match(opASSG);
            ast->type = ASSG;
            ast->symtab = found;
            ast->sub0 = arith_expr(expr());
        }
        match(SEMI);
        return ast;
    }
    else if (current == kwRETURN)
    {
        AST *ast = new_ast(RETURN);
        match(kwRETURN);
        if (current == ID || current == INTCON || current == LPAREN || current == opSUB)
            ast->sub0 = arith_expr(expr());
        match(SEMI);
        return ast;
    }
    else if (current == LBRACE)
    {
        match(LBRACE);
        AST *ast = opt_stmt_list();
        match(RBRACE);
        return ast;
    }
    else if (current == SEMI)
    {
        match(SEMI);
        return NULL;
    }
    else
    {
        fprintf(stderr, "Error line %d, unexpected token %s\n", line, lexeme);
        exit(1);
    }
    return 0;
}

AST *opt_stmt_list()
{
    AST *head = NULL, *tail = NULL, *temp, *sub;
    while (current == kwIF || current == kwWHILE || current == ID || current == kwRETURN || current == LBRACE || current == SEMI)
    {
        sub = stmt();
        temp = new_ast(STMT_LIST);
        temp->sub0 = sub;
        if (head == NULL)
            head = temp;
        else
            tail->sub1 = temp;
        tail = temp;
    }
    return head;
}

int parse()
{
    current = get_token();
    while (current != EOF)
    {
        match(kwINT);
        match(ID);
        SymTab *func, *temp;
        AST *node;
        if (current == LPAREN)
        {
            func = add_symtab(0, last, "function");
            node = new_ast(FUNC_DEF);
            node->symtab = func;
            match(LPAREN);
            int i = 0;
            if (current == kwINT)
            {
                match(kwINT);
                match(ID);
                temp = add_symtab(1, last, "variable");
                i += 1;
                temp->pos = i;
                while (current == COMMA)
                {
                    match(COMMA);
                    match(kwINT);
                    match(ID);
                    temp = add_symtab(1, last, "variable");
                    i += 1;
                    temp->pos = i;
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
            node->sub0 = opt_stmt_list();
            match(RBRACE);
            if (print_ast_flag)
                print_ast(node);
            reset_table(1);
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
    match(EOF);
    return 0;
}