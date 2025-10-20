#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#include "ast.h"

typedef struct symbol_table
{
    char *name;
    char *type;
    int args;
    int pos;
    struct symbol_table *next;
} SymTab;

typedef struct ast
{
    int num;
    NodeType type;
    SymTab *symtab;
    struct ast *sub0, *sub1, *sub2;
} AST;

SymTab *search_symtab(int scope, char *name);
SymTab *search_all_symtab(char *name);
SymTab *add_symtab(int scope, char *name, char *type);
void reset_table(int scope);
AST *new_ast(NodeType ntype);

#endif