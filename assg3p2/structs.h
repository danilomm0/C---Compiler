#ifndef __STRUCTS_H__
#define __STRUCTS_H__

#include "ast.h"

typedef enum
{
    FUNC,
    GLOBAL,
    LOCAL
} SymType;

typedef struct symbol_table
{
    char *name;
    SymType type;
    int args;
    int pos;
    int size;
    struct symbol_table *next;
} SymTab;

typedef enum
{
    INT,
    SYMTAB,
    QUAD
} OperandType;

typedef struct operand
{
    OperandType type;
    union
    {
        int num;
        SymTab *table;
        struct quad *ptr;
    } val;
} Operand;

typedef enum
{
    NOP,
    MOVE,
    GOTO,
    IF_EQ,
    IF_NE,
    IF_LE,
    IF_LT,
    IF_GE,
    IF_GT,
    LABEL,
    ENTER,
    LEAVE,
    PARAM,
    CALL,
    REETURN,
    RETRIEVE
} OpType;

typedef struct quad
{
    OpType op;
    Operand *src1, *src2, *dest;
    struct quad *next;
} Quad;

typedef struct ast
{
    int num;
    NodeType type;
    SymTab *symtab, *loc;
    struct ast *sub0, *sub1, *sub2;
    struct quad *head, *tail;
} AST;

SymTab *search_symtab(int scope, char *name);
SymTab *search_all_symtab(char *name);
SymTab *add_symtab(int scope, char *name, SymType type);
void reset_table(int scope);

AST *new_ast(NodeType ntype);

void gen_global();
void gen_code(AST *ast);
void gen_3addr(AST *ast);

#endif