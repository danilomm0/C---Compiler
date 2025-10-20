/*
 * File: scanner-driver.c
 * Author: Danilo Malo-Molina
 * Purpose: Contains all of the functions that manipulate structs directly
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "scanner.h"
#include "structs.h"
#include "ast.h"

extern int chk_decl_flag;
extern int line;

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

void reset_table(int scope)
{
    table[scope] = NULL;
}

AST *new_ast(NodeType type)
{
    AST *new = malloc(sizeof(AST));
    if (new == NULL)
    {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    new->type = type;
    return new;
}

NodeType ast_node_type(void *ptr)
{
    AST *ast = ptr;
    assert(ast != NULL);
    return ast->type;
}

char *func_def_name(void *ptr)
{
    AST *ast = ptr;
    assert(ast != NULL);
    SymTab *temp = ast->symtab;
    assert(temp != NULL);
    return temp->name;
}

int func_def_nargs(void *ptr)
{
    AST *ast = ptr;
    assert(ast != NULL);
    SymTab *temp = ast->symtab;
    assert(temp != NULL);
    return temp->args;
}

void *func_def_body(void *ptr)
{
    AST *ast = ptr;
    assert(ast != NULL);
    return ast->sub0;
}

char *func_def_argname(void *ptr, int n)
{
    AST *ast = ptr;
    assert(ast != NULL);
    SymTab *temp = ast->symtab;
    assert(temp != NULL);
    int args = temp->args;
    assert(n > 0 && n <= args);
    for (SymTab *temp = table[1]; temp != NULL; temp = temp->next)
        if (temp->pos == n)
            return temp->name;
    return NULL;
}

char *func_call_callee(void *ptr)
{
    return func_def_name(ptr);
}

void *func_call_args(void *ptr)
{
    return func_def_body(ptr);
}

void *stmt_list_head(void *ptr)
{
    return func_def_body(ptr);
}

void *stmt_list_rest(void *ptr)
{
    AST *ast = ptr;
    assert(ast != NULL);
    return ast->sub1;
}

void *expr_list_head(void *ptr)
{
    return func_def_body(ptr);
}

void *expr_list_rest(void *ptr)
{
    return stmt_list_rest(ptr);
}

char *expr_id_name(void *ptr)
{
    return func_def_name(ptr);
}

int expr_intconst_val(void *ptr)
{
    AST *ast = ptr;
    assert(ast != NULL);
    return ast->num;
}

void *expr_operand_1(void *ptr)
{
    return func_def_body(ptr);
}

void *expr_operand_2(void *ptr)
{
    return stmt_list_rest(ptr);
}

void *stmt_if_expr(void *ptr)
{
    return func_def_body(ptr);
}

void *stmt_if_then(void *ptr)
{
    return stmt_list_rest(ptr);
}

void *stmt_if_else(void *ptr)
{
    AST *ast = ptr;
    assert(ast != NULL);
    return ast->sub2;
}

char *stmt_assg_lhs(void *ptr)
{
    return func_def_name(ptr);
}

void *stmt_assg_rhs(void *ptr)
{
    return func_def_body(ptr);
}

void *stmt_while_expr(void *ptr)
{
    return func_def_body(ptr);
}

void *stmt_while_body(void *ptr)
{
    return stmt_list_rest(ptr);
}

void *stmt_return_expr(void *ptr)
{
    return func_def_body(ptr);
}