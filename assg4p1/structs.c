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
SymTab *currenttbl = NULL;
int labelcount = 0;
int tempcount = 0;

void gen_3addr(AST *ast);
void print_quad(Quad *qtemp);

void *allocate(int size)
{
    void *ptr;
    ptr = calloc(size, 1);
    if (ptr == NULL)
    {
        fprintf(stderr, "Out of memory!\n");
        exit(1);
    }
    return ptr;
}

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

SymTab *add_symtab(int scope, char *name, SymType type)
{
    SymTab *temp;
    if (chk_decl_flag && search_symtab(scope, name) != NULL)
    {
        fprintf(stderr, "Error line %d, multiple declarations of %s\n", line, name);
        exit(1);
    }
    temp = allocate(sizeof(SymTab));
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

int find_size(SymTab *ptr)
{
    SymTab *temp;
    int num = 0;
    for (temp = table[1]; temp != NULL; temp = temp->next)
    {
        if (temp->type != LOCAL)
            continue;
        if (temp->pos > 0)
        {
            temp->size = 8 + 4 * (temp->pos - 1);
        }
        else
        {
            temp->size = -(4 + 4 * num);
            num += 1;
        }
    }
    return num * 4;
}

AST *new_ast(NodeType type)
{
    AST *new = allocate(sizeof(AST));
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

void gen_global()
{
    SymTab *temp;
    printf("\n");
    printf(".data\n");
    printf(".align 2\n");
    for (temp = table[0]; temp != NULL; temp = temp->next)
        if (temp->type == GLOBAL)
            printf("_%s:  .space 4\n", temp->name);
    printf("\n");
    printf(".align 2\n");
    printf(".data\n");
    printf("__nl: .asciiz \"\\n\"\n");
    printf(".align 2\n");
    printf(".text\n");
    printf(" _println:\n");
    printf("    li $v0, 1\n");
    printf("    lw $a0, 0($sp)\n");
    printf("    syscall\n");
    printf("    li $v0, 4\n");
    printf("    la $a0, __nl\n");
    printf("    syscall\n");
    printf("    jr $ra\n");
    printf("\n");
    printf("main:\n");
    printf("    j _main\n");
    printf("\n");
}

Operand *make_op(int num, SymTab *ttemp, Quad *qtemp)
{
    Operand *temp = allocate(sizeof(Operand));
    if (ttemp != NULL)
    {
        temp->type = SYMTAB;
        temp->val.table = ttemp;
    }
    else if (qtemp != NULL)
    {
        temp->type = QUAD;
        temp->val.ptr = qtemp;
    }
    else
    {
        temp->type = INT;
        temp->val.num = num;
    }
    return temp;
}

Quad *make_quad(OpType op, Operand *src1, Operand *src2, Operand *dest)
{
    Quad *temp = allocate(sizeof(Quad));
    temp->op = op;
    temp->src1 = src1;
    temp->src2 = src2;
    temp->dest = dest;
    return temp;
}

SymTab *make_tab()
{
    char name[24];
    snprintf(name, 24, "tmp$%d", tempcount++);
    SymTab *temp = add_symtab(1, strdup(name), LOCAL);
    return temp;
}

OpType node2optype(NodeType type)
{
    if (type == EQ)
        return IF_EQ;
    else if (type == NE)
        return IF_NE;
    else if (type == LE)
        return IF_LE;
    else if (type == LT)
        return IF_LT;
    else if (type == GE)
        return IF_GE;
    else if (type == GT)
        return IF_GT;
    else if (type == ADD)
        return OPADD;
    else if (type == SUB)
        return OPSUB;
    else if (type == MUL)
        return OPMUL;
    else if (type == DIV)
        return OPDIV;
    else if (type == UMINUS)
        return OPUMINUS;
    else
    {
        fprintf(stderr, "Unhandled AST nodetype %d\n", type);
        exit(1);
    }
}

void gen_boolexp(AST *ast, Quad *yes, Quad *no)
{
    assert(ast != NULL);
    if (ast->type == EQ || ast->type == NE || ast->type == LE || ast->type == LT || ast->type == GE || ast->type == GT)
    {
        OpType type = node2optype(ast->type);
        gen_3addr(ast->sub0);
        gen_3addr(ast->sub1);
        Quad *temp1 = make_quad(type, make_op(0, ast->sub0->loc, NULL), make_op(0, ast->sub1->loc, NULL), make_op(0, NULL, yes));
        Quad *temp2 = make_quad(GOTO, NULL, NULL, make_op(0, NULL, no));
        if (ast->sub0->head == NULL)
        {
            if (ast->sub1->head == NULL)
                ast->head = temp1;
            else
            {
                ast->head = ast->sub1->head;
                ast->sub1->tail->next = temp1;
            }
        }
        else
        {
            ast->head = ast->sub0->head;
            if (ast->sub1->head == NULL)
                ast->sub0->tail->next = temp1;
            else
            {
                ast->sub0->tail->next = ast->sub1->head;
                ast->sub1->tail->next = temp1;
            }
        }
        temp1->next = temp2;
        ast->tail = temp2;
    }
    else if (ast->type == AND || ast->type == OR)
    {
        Quad *label = make_quad(LABEL, make_op(labelcount++, NULL, NULL), NULL, NULL);
        if (ast->type == AND)
            gen_boolexp(ast->sub0, label, no);
        else
            gen_boolexp(ast->sub0, yes, label);
        gen_boolexp(ast->sub1, yes, no);
        ast->head = ast->sub0->head;
        ast->sub0->tail->next = label;
        label->next = ast->sub1->head;
        ast->tail = ast->sub1->tail;
    }
    else
    {
        fprintf(stderr, "Unhandled AST nodetype %d\n", ast->type);
        exit(1);
    }
}

void gen_3addr(AST *ast)
{
    if (ast == NULL)
        return;
    if (ast->type == FUNC_DEF)
    {
        AST *func = ast->sub0;
        currenttbl = ast->symtab;
        gen_3addr(func);
        Operand *temp = make_op(0, currenttbl, NULL);
        Quad *enter = make_quad(ENTER, temp, NULL, NULL);
        Quad *leave = make_quad(LEAVE, temp, NULL, NULL);
        Quad *reeturn = make_quad(REETURN, NULL, NULL, NULL);
        if (func == NULL)
            enter->next = leave;
        else
        {
            enter->next = func->head;
            func->tail->next = leave;
        }
        leave->next = reeturn;
        ast->head = enter;
        ast->tail = reeturn;
        currenttbl = NULL;
    }
    else if (ast->type == FUNC_CALL)
    {
        AST *temp;
        Quad *head = NULL, *tail = NULL, *param, *call, *retrieve;
        SymTab *ttab = make_tab();
        gen_3addr(ast->sub0);
        for (temp = ast->sub0; temp != NULL; temp = temp->sub1)
        {
            param = make_quad(PARAM, make_op(0, temp->sub0->loc, NULL), NULL, NULL);
            if (head == NULL)
            {
                head = param;
                tail = param;
            }
            else
            {
                param->next = head;
                head = param;
            }
        }
        call = make_quad(CALL, make_op(0, ast->symtab, NULL), make_op(ast->symtab->args, NULL, NULL), NULL);
        retrieve = make_quad(RETRIEVE, make_op(0, ttab, NULL), NULL, NULL);
        ast->loc = ttab;
        if (ast->sub0 == NULL)
            ast->head = call;
        else
        {
            if (ast->sub0->head == NULL)
                ast->head = param;
            else
            {
                ast->head = ast->sub0->head;
                ast->sub0->tail->next = param;
            }
            tail->next = call;
        }
        call->next = retrieve;
        ast->tail = retrieve;
    }
    else if (ast->type == IF)
    {
        Quad *then = make_quad(LABEL, make_op(labelcount++, NULL, NULL), NULL, NULL);
        Quad *els = make_quad(LABEL, make_op(labelcount++, NULL, NULL), NULL, NULL);
        Quad *after = make_quad(LABEL, make_op(labelcount++, NULL, NULL), NULL, NULL);
        Quad *jump = make_quad(GOTO, NULL, NULL, make_op(0, NULL, after));
        gen_boolexp(ast->sub0, then, els);
        ast->head = ast->sub0->head;
        ast->sub0->tail->next = then;
        if (ast->sub1 == NULL)
            then->next = jump;
        else
        {
            gen_3addr(ast->sub1);
            then->next = ast->sub1->head;
            ast->sub1->tail->next = jump;
        }
        jump->next = els;
        if (ast->sub2 == NULL)
            els->next = after;
        else
        {
            gen_3addr(ast->sub2);
            els->next = ast->sub2->head;
            ast->sub2->tail->next = after;
        }
        ast->tail = after;
    }
    else if (ast->type == WHILE)
    {
        Quad *before = make_quad(LABEL, make_op(labelcount++, NULL, NULL), NULL, NULL);
        Quad *body = make_quad(LABEL, make_op(labelcount++, NULL, NULL), NULL, NULL);
        Quad *after = make_quad(LABEL, make_op(labelcount++, NULL, NULL), NULL, NULL);
        Quad *main = make_quad(GOTO, NULL, NULL, make_op(0, NULL, before));
        gen_boolexp(ast->sub0, body, after);
        gen_3addr(ast->sub1);
        ast->head = before;
        before->next = ast->sub0->head;
        ast->sub0->tail->next = body;
        if (ast->sub1->head == NULL)
            body->next = main;
        else
        {
            body->next = ast->sub1->head;
            ast->sub1->tail->next = main;
        }
        main->next = after;
        ast->tail = after;
    }
    else if (ast->type == ASSG)
    {
        gen_3addr(ast->sub0);
        Quad *temp = make_quad(MOVE, make_op(0, ast->sub0->loc, NULL), NULL, make_op(0, ast->symtab, NULL));
        if (ast->sub0->head == NULL)
        {
            ast->head = temp;
            ast->tail = temp;
        }
        else
        {
            ast->head = ast->sub0->head;
            ast->sub0->tail->next = temp;
            ast->tail = temp;
        }
    }
    else if (ast->type == RETURN)
    {
        Quad *leave = make_quad(LEAVE, make_op(0, currenttbl, NULL), NULL, NULL);
        Quad *reeturn = make_quad(REETURN, NULL, NULL, NULL);
        leave->next = reeturn;
        if (ast->sub0)
        {
            gen_3addr(ast->sub0);
            reeturn->src1 = make_op(0, ast->sub0->loc, NULL);
            ast->head = ast->sub0->head;
            ast->sub0->tail->next = leave;
            ast->tail = reeturn;
        }
        else
        {
            ast->head = leave;
            ast->tail = reeturn;
        }
    }
    else if (ast->type == STMT_LIST || ast->type == EXPR_LIST)
    {
        AST *temp0, *temp1, *temp2;
        for (temp0 = ast; temp0 != NULL; temp0 = temp0->sub1)
        {
            gen_3addr(temp0->sub0);
        }
        ast->head = ast->sub0->head;
        for (temp0 = ast; temp0 != NULL; temp0 = temp0->sub1)
        {
            temp1 = temp0->sub0;
            if (temp0->sub1 == NULL)
                ast->tail = temp1->tail;
            else
            {
                temp2 = temp0->sub1->sub0;
                temp1->tail->next = temp2->head;
            }
        }
    }
    else if (ast->type == IDENTIFIER)
    {
        Quad *inst = make_quad(NOP, NULL, NULL, NULL);
        ast->head = inst;
        ast->tail = inst;
        ast->loc = ast->symtab;
    }
    else if (ast->type == INTCONST)
    {
        SymTab *stemp = make_tab();
        Quad *qtemp = make_quad(MOVE, make_op(ast->num, NULL, NULL), NULL, make_op(0, stemp, NULL));
        ast->loc = stemp;
        ast->head = qtemp;
        ast->tail = qtemp;
    }
    else if (ast->type == EQ || ast->type == NE || ast->type == LE || ast->type == LT || ast->type == GE || ast->type == GT)
    {
        assert(0);
    }
    else if (ast->type == ADD || ast->type == SUB || ast->type == MUL || ast->type == DIV || ast->type == UMINUS)
    {
        ast->loc = make_tab();
        gen_3addr(ast->sub0);
        Operand *temp1 = make_op(0, ast->sub0->loc, NULL);
        Operand *temp2;
        OpType op;
        if (ast->type == UMINUS)
        {
            op = OPUMINUS;
            temp2 = NULL;
        }
        else
        {
            op = node2optype(ast->type);
            gen_3addr(ast->sub1);
            temp2 = make_op(0, ast->sub1->loc, NULL);
        }
        Quad *arith = make_quad(op, temp1, temp2, make_op(0, ast->loc, NULL));
        if (ast->type == UMINUS)
            ast->sub0->tail->next = arith;
        else
        {
            ast->sub0->tail->next = ast->sub1->head;
            ast->sub1->tail->next = arith;
        }
        ast->head = ast->sub0->head;
        ast->tail = arith;
    }
    else
    {
        fprintf(stderr, "Unrecognized AST type: %d\n", ast->type);
        exit(1);
    }
}

void mips_load(Operand *op, int i)
{
    if (op->type == INT)
        printf("    li $t%d, %d\n", i, op->val.num);
    else if (op->type == SYMTAB)
    {
        SymTab *temp = op->val.table;
        if (temp->type == GLOBAL)
            printf("    lw $t%d, _%s\n", i, temp->name);
        else
        {
            assert(temp->type == LOCAL);
            printf("    lw $t%d, %d($fp)\n", i, temp->size);
        }
    }
}

void print_operand(Operand *op)
{
    if (op == NULL)
        return;
    else if (op->type == INT)
        printf("%d", op->val.num);
    else if (op->type == SYMTAB)
        printf("%s", op->val.table->name);
    else if (op->type == QUAD)
        print_quad(op->val.ptr);
    else
        fprintf(stderr, "Unrecognized operand type: %d\n", op->type);
}

void print_quad(Quad *qtemp)
{
    assert(qtemp != NULL);
    char *names[] = {"no_op", "move", "goto", "if_eq", "if_ne", "if_le", "if_lt", "if_ge", "if_gt", "label", "enter", "leave", "param", "call", "return", "retrieve"};
    printf("%s ", names[qtemp->op]);
    print_operand(qtemp->src1);
    if (qtemp->src1 != NULL && (qtemp->src2 != NULL || qtemp->dest != NULL))
        printf(", ");
    print_operand(qtemp->src2);
    if (qtemp->src2 != NULL && qtemp->dest != NULL)
        printf(", ");
    print_operand(qtemp->dest);
}

void gen_mips(AST *ast)
{
    SymTab *ttemp = ast->symtab, *searched;
    Quad *qtemp;
    char *name = ast->symtab->name;
    assert(ast->type == FUNC_DEF);
    ast->symtab->size = find_size(ttemp);
    printf(".text\n");
    printf("# function %s\n", ttemp->name);
    printf("# formals: ");
    for (int i = 0; i < ttemp->args; i++)
    {
        char *this_var = func_def_argname(ast, i + 1);
        searched = search_symtab(1, this_var);
        printf("%s (loc: %d); ", this_var, searched->size);
    }
    printf("\n");
    printf("# locals: ");
    for (searched = table[1]; searched != NULL; searched = searched->next)
    {
        if (searched->type == LOCAL && searched->pos == 0)
        {
            printf("%s (loc: %d); ", searched->name, searched->size);
        }
    }
    printf("\n");
    printf("# space for locals: %d bytes\n", ast->symtab->size);
    printf("#\n");
    printf("_%s:\n", ast->symtab->name);
    for (qtemp = ast->head; qtemp != NULL; qtemp = qtemp->next)
    {
        printf("    # ");
        print_quad(qtemp);
        printf("\n");

        if (qtemp->op == NOP || qtemp->op == LEAVE)
        {
        }
        else if (qtemp->op == MOVE)
        {
            SymTab *ptr = (qtemp->dest->val).table;
            mips_load(qtemp->src1, 0);
            if (ptr->type == LOCAL)
                printf("    sw $t0, %d($fp)\n", ptr->size);
            else if (ptr->type == GLOBAL)
                printf("    sw $t0, _%s\n", ptr->name);
            else
            {
                fprintf(stderr, "Unrecognized table type: %d\n", ptr->type);
                exit(1);
            }
        }
        else if (qtemp->op == GOTO)
        {
            printf("    j Lbl%d\n", ((qtemp->dest->val).ptr->src1->val).num);
        }
        else if (qtemp->op == IF_EQ || qtemp->op == IF_NE || qtemp->op == IF_LE || qtemp->op == IF_LT || qtemp->op == IF_GE || qtemp->op == IF_GT)
        {
            mips_load(qtemp->src1, 0);
            mips_load(qtemp->src2, 1);
            if (qtemp->op == IF_EQ)
                printf("    beq $t0, $t1, Lbl%d\n", ((qtemp->dest->val).ptr->src1->val).num);
            else if (qtemp->op == IF_NE)
                printf("    bne $t0, $t1, Lbl%d\n", ((qtemp->dest->val).ptr->src1->val).num);
            else if (qtemp->op == IF_LE)
                printf("    ble $t0, $t1, Lbl%d\n", ((qtemp->dest->val).ptr->src1->val).num);
            else if (qtemp->op == IF_LT)
                printf("    blt $t0, $t1, Lbl%d\n", ((qtemp->dest->val).ptr->src1->val).num);
            else if (qtemp->op == IF_GE)
                printf("    bge $t0, $t1, Lbl%d\n", ((qtemp->dest->val).ptr->src1->val).num);
            else if (qtemp->op == IF_GT)
                printf("    bgt $t0, $t1, Lbl%d\n", ((qtemp->dest->val).ptr->src1->val).num);
            else
                printf("    Unrecognized symbol type: %d", qtemp->op);
        }
        else if (qtemp->op == OPADD || qtemp->op == OPSUB || qtemp->op == OPMUL || qtemp->op == OPDIV || qtemp->op == OPUMINUS)
        {
            mips_load(qtemp->src1, 0);
            if (qtemp->op == OPUMINUS)
                printf("    neg $t2, $t0\n");
            else
            {
                mips_load(qtemp->src2, 1);
                if (qtemp->op == OPADD)
                    printf("    add $t2, $t0, $t1\n");
                else if (qtemp->op == OPSUB)
                    printf("    sub $t2, $t0, $t1\n");
                else if (qtemp->op == OPMUL)
                    printf("    mul $t2, $t0, $t1\n");
                else if (qtemp->op == OPDIV)
                    printf("    div $t2, $t0, $t1\n");
                else
                    printf("    Unrecognized symbol type: %d", qtemp->op);
            }
            if (qtemp->dest->type == SYMTAB)
            {
                SymTab *temp = (qtemp->dest->val).table;
                if (temp->type == GLOBAL)
                    printf("    sw $t2, _%s\n", temp->name);
                if (temp->type == LOCAL)
                    printf("    sw $t2, %d($fp)\n", temp->size);
            }
        }
        else if (qtemp->op == LABEL)
        {
            printf("Lbl%d:\n", (qtemp->src1->val).num);
        }
        else if (qtemp->op == ENTER)
        {
            printf("    la $sp, -8($sp)    # allocate space for old $fp and $ra\n");
            printf("    sw $fp, 4($sp)     # save old $fp\n");
            printf("    sw $ra, 0($sp)     # save old $ra\n");
            printf("    la $fp, 0($sp)     # $fp := $sp\n");
            printf("    la $sp, -%d($sp)   # allocate stack frame\n", (qtemp->src1->val).table->size);
        }
        else if (qtemp->op == PARAM)
        {
            mips_load(qtemp->src1, 0);
            printf("    la $sp, -4($sp)\n");
            printf("    sw $t0, 0($sp)\n");
        }
        else if (qtemp->op == CALL)
        {
            printf("    jal _%s\n", (qtemp->src1->val).table->name);
            printf("    la $sp, %d($sp)\n", 4 * (qtemp->src2->val).num);
        }
        else if (qtemp->op == REETURN)
        {
            printf("    00010\n");
            if (qtemp->src1 != NULL)
            {
                if (qtemp->src1->type == INT)
                    printf("    li $v0, %d\n", (qtemp->src1->val).num);
                else if (qtemp->src1->type == SYMTAB)
                {
                    SymTab *temp = (qtemp->src1->val).table;
                    if (temp->type == GLOBAL)
                        printf("    lw $v0, _%s\n", temp->name);
                    if (temp->type == LOCAL)
                        printf("    lw $v0, %d($fp)\n", temp->size);
                }
            }
            printf("    la $sp, 0($fp)     # deallocate locals\n");
            printf("    lw $ra, 0($sp)     # restore return address\n");
            printf("    lw $fp, 4($sp)     # restore frame pointer\n");
            printf("    la $sp, 8($sp)     # restore stack pointer\n");
            printf("    jr $ra\n");
        }
        else if (qtemp->op == RETRIEVE)
        {
            if (qtemp->src1->type == SYMTAB)
            {
                SymTab *temp = (qtemp->src1->val).table;
                if (temp->type == GLOBAL)
                    printf("    sw $v0, _%s\n", temp->name);
                if (temp->type == LOCAL)
                    printf("    sw $v0, %d($fp)\n", temp->size);
            }
        }
        else
        {
            fprintf(stderr, "Unrecognized op: %d\n", qtemp->op);
            exit(1);
        }
        printf("\n");
    }
    printf("# end function %s\n\n", name);
}

void gen_code(AST *ast)
{
    gen_3addr(ast);
    gen_mips(ast);
}