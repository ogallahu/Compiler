//Owen Gallahue


#include "decl.h"
#include "stmt.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "scope.h"
#include "type.h"
#include "scratch.h"
extern int typeError;
extern int resolveError;
extern int labelNum;

struct stmt * stmt_create( stmt_t kind, struct decl *decl, struct expr *init_expr, struct expr *expr, struct expr *next_expr, struct stmt *body, struct stmt *else_body, struct stmt *next ){
       struct stmt *s = calloc(1, sizeof (*s));
       s->kind = kind;
       s->decl = decl;
       s->init_expr = init_expr;
       s->expr = expr;
       s->next_expr = next_expr;
       s->body = body;
       s->else_body = else_body;
       s->next = next;

       return s; 
}
void stmt_resolve(struct stmt *s){
    if(!s) return;

    if(s->decl) decl_resolve(s->decl);
    if(s->init_expr) expr_resolve(s->init_expr);
    if(s->expr) expr_resolve(s->expr);
    if(s->next_expr) expr_resolve(s->next_expr);

    if(s->body) {
        scope_enter();
        stmt_resolve(s->body);
        scope_exit();
    }
    if(s->else_body){
        scope_enter();
        stmt_resolve(s->else_body);
        scope_exit();
    }
    stmt_resolve(s->next);

}
void stmt_print(struct stmt *s, int nestitude){
    while(s){
        printTabs(nestitude);
        switch(s->kind){
            case STMT_DECL:
                decl_print(s->decl,nestitude-1);
                break;
            case STMT_EXPR:
                expr_print(s->expr);
                printf(";\n");
                break;
            case STMT_BLOCK:
                printf("{\n");
                stmt_print(s->body, nestitude+1);
                printTabs(nestitude);
                printf("}\n");
            case STMT_IF_ELSE:
                printf("if (");
                expr_print(s->expr);
                printf(") {\n");
                stmt_print(s->body, nestitude+1);
                printf("\n");
                printTabs(nestitude);
                if (s->else_body){
                    printf(" else {\n");
                    stmt_print(s->else_body, nestitude + 1);
                    printTabs(nestitude);
                    printf("}");
                } else {
                    printf("}\n");
                }
                printf("\n");
                break;

            case STMT_FOR:
                /* for stmt */
                printf("for(");
                expr_print(s->init_expr);
                printf("; ");
                expr_print(s->expr);
                printf("; ");
                expr_print(s->next_expr);
                printf(") {\n");
                stmt_print(s->body, nestitude+ 1);
                printf("\n");
                printTabs(nestitude);
                printf("}\n");
                break;
            case STMT_RETURN:
                if (s->expr) {
                    printf("return ");
                    expr_print(s->expr);
                    printf(";\n");
                } else {
                    printf("return;\n");
                }
                break;
            case STMT_PRINT:
                if (s->expr) {
                    printf("print ");
                    expr_print(s->expr);
                    printf(";\n");
                } else {
                    printf("print;\n");
                }
                break;
            default:
                fprintf(stderr,"Error: stmt type unknown, cannot print\n");
                break;
    }
    s = s->next;

    }
}
void printTabs(int tabs){
    for( int i = 0; i < tabs; i++){
        printf("\t");
    }
}

void stmt_typeCheck( struct stmt *s , struct decl *d)
{
    struct type *t;
    if(!s) return;

    switch(s->kind) {
        case STMT_EXPR:
            expr_typeCheck(s->expr);
            break;
        case STMT_IF_ELSE:
            t = expr_typeCheck(s->init_expr);
            if(t && t->kind != TYPE_BOOLEAN) {
                printf("Type Error: not a Boolean inside if statment");
                typeError=1;
            }
            stmt_typeCheck(s->body, d);
            stmt_typeCheck(s->else_body, d);
            break;
        /* more cases here */
        case STMT_DECL:
            decl_typeCheck(s->decl);
            break;

        case STMT_FOR:
            expr_typeCheck(s->init_expr);
            t = expr_typeCheck(s->expr);
            if(t){
                if(t->kind!=TYPE_BOOLEAN){
                    printf("TYPE ERROR: Boolean must be inside of loop");
                    typeError = 1;
                }
            }
            expr_typeCheck(s->next_expr);
            stmt_typeCheck(s->body, d);
            break;

        case STMT_PRINT:
            t = expr_typeCheck(s->expr);
            break;

        case STMT_RETURN:
              t = expr_typeCheck(s->expr);
              if(t){
                type_print(t);
                type_print(d->type->subtype);

                if(!type_equal(d->type->subtype, t)){
                  printf("Typecheck error: unexpected return type in function %s\n", d->name);
                  typeError = 1;
                }
              }
              else { 
                if( d->type->subtype->kind != TYPE_VOID){
                  printf("Typecheck error: cannot return void value from non-void function");
                  typeError = 1;
                  break;
                } 
              }
              break;
        case STMT_BLOCK:
            stmt_typeCheck(s->body,d);
            break;
        default:
            printf("Resolve Error: statment type not defined (%d)\n", s->kind);
            resolveError=1;
            break;

    }
    if(s->next && d->next) stmt_typeCheck(s->next, d->next);
 }


void stmt_codegen( struct stmt *s, FILE *file ) {
    printf("IN SIDE STMT CODE GEN\n");
    struct expr *e;
    struct string_node *sn;
    int string_count = 0;
    char* string_label = (char*)malloc(9);
    char* label_name = (char*)malloc(9);
    struct symbol *sym;
    int label1, label2;
    if(!s) return;
    puts("S is not null in stmt_codegen\n");
    switch(s->kind) {
        case STMT_DECL:
            decl_codegen(s->decl, file);
            puts("FLAG 1");
            break;
        case STMT_EXPR:
            expr_codegen(s->expr, file);
            puts("FLAG 2");
            break;
        case STMT_IF_ELSE:
            label1 = labelNum;
            label2 = labelNum + 1;
            expr_codegen(s->expr, file);
            fprintf(file, "\tCMP $0, %s\n", register_name(s->expr->reg));
            labelNum++;
            sprintf(label_name, "l1_%d", label1);
            fprintf(file, "\tJE %s\n", label_name);
            stmt_codegen(s->body, file);
            labelNum++;
            sprintf(label_name, "l2_%d", label2);
            fprintf(file, "\tJMP %s\n", label_name);
            sprintf(label_name, "l1_%d", label1);
            fprintf(file, "%s:\n", label_name);
            stmt_codegen(s->else_body, file);
            sprintf(label_name, "l2_%d", label2);
            fprintf(file, "%s:\n", label_name);
            break;
        case STMT_FOR:
            puts("FLAG 4");
            label1 = labelNum;
            label2 = labelNum + 1;
            expr_codegen(s->init_expr, file);
            labelNum++;
            sprintf(label_name, "lf1_%d", label1);
            fprintf(file, "%s:\n", label_name);
            if(s->init_expr) register_free(s->init_expr->reg);
            if(s->expr) {
                expr_codegen(s->expr, file);
            } else {
                s->expr = expr_create(EXPR_BOOL_LITERAL, 0, 0);
                s->expr->reg = register_alloc();
                fprintf(file, "\tMOV $1, %s\n", register_name(s->expr->reg));
            }
            fprintf(file, "\tCMP $0, %s\n", register_name(s->expr->reg));
            labelNum++;
            sprintf(label_name, "lf2_%d", label2);
            fprintf(file, "\tJE %s\n", label_name);
            stmt_codegen(s->body, file);
            expr_codegen(s->next_expr, file);
            sprintf(label_name, "lf1_%d", label1);
            fprintf(file, "\tJMP %s\n", label_name);
            sprintf(label_name, "lf2_%d", label2);
            fprintf(file, "%s:\n", label_name);
            if(!s->expr) register_free(s->expr->reg);
            break;
        case STMT_PRINT:
            puts("FLAG 3");
            fprintf(file, "\n");
            e = s->expr;
            while(e) {
                printf("In stmnt print\n");
                if(e->kind != EXPR_ARG){
                    expr_codegen(e, file);
                    printf("    after call of expr_codegen\n ");
                    fprintf(file, "\tMOVQ %s, %rdi\n", register_name(e->reg));
                    fprintf(file, "\tPUSHQ %r10\n");
                    fprintf(file, "\tPUSHQ %r11\n");
                    printf("     before switch\n");
                }
                switch(e->kind) {
                    case EXPR_ARG:
                        fprintf(file, "\tPUSHQ %r10\n");
                        fprintf(file, "\tPUSHQ %r11\n");
                        while(e->right){
                            switch(e->left->kind){
                                case EXPR_ADD: 
                                case EXPR_MULT:
                                case EXPR_DIV:
                                case EXPR_MOD:
                                case EXPR_EXP:
                                case EXPR_MINUS:
                                case EXPR_INCR:
                                case EXPR_DECR:
                                case EXPR_INTERGER_LITERAL:
                                    expr_codegen(e->left, file);
                                    fprintf(file, "\tMOVQ %s, %rdi\n", register_name(e->left->reg));
                                    fprintf(file, "\n\tCALL print_integer\n\n");
                                    register_free(e->left->reg);
                                    break;
                                case EXPR_CHAR_LITERAL:
                                    expr_codegen(e->left, file);
                                    fprintf(file, "\tMOVQ %s, %rdi\n", register_name(e->left->reg));
                                    fprintf(file, "\n\tCALL print_character\n\n");
                                    register_free(e->left->reg);
                                    break;
                                case EXPR_STRING_LITERAL:
                                    expr_codegen(e->left, file);
                                    fprintf(file, "\tMOVQ %s, %rdi\n", register_name(e->left->reg));
                                    fprintf(file, "\n\tCALL print_string\n\n");
                                    register_free(e->left->reg);
                                    break;
                                case EXPR_NAME:
                                    expr_codegen(e->left, file);
                                    fprintf(file, "\tMOVQ %s, %rdi\n", register_name(e->left->reg));
                                    switch(expr_typeCheck(e)->kind) {
                                        case TYPE_BOOLEAN:
                                            fprintf(file, "\n\tCALL print_boolean\n\n");
                                            break;
                                        case TYPE_CHARACTER:
                                            fprintf(file, "\n\tCALL print_character\n\n");
                                            break;
                                        case TYPE_INTEGER:
                                            fprintf(file, "\n\tCALL print_integer\n\n");
                                            break;
                                        case TYPE_STRING:
                                            fprintf(file, "\n\tCALL print_string\n\n");
                                            break;
                                        default:
                                            break;
                                    }
                                    break;
                                default:
                                    break;
                            }
                            if(e->left->kind != EXPR_ARG && e->right->kind != EXPR_ARG){
                                expr_codegen(e->right, file);
                                fprintf(file, "\tMOVQ %s, %rdi\n", register_name(e->right->reg));
                                switch(e->right->kind){
                                    case EXPR_INTERGER_LITERAL:
                                        fprintf(file, "\n\tCALL print_integer\n\n");
                                        break;
                                    case EXPR_CHAR_LITERAL:
                                        fprintf(file, "\n\tCALL print_character\n\n");
                                        break;
                                    case EXPR_STRING_LITERAL:
                                        fprintf(file, "\n\tCALL print_string\n\n");
                                        break;
                                    case EXPR_NAME:
                                        switch(expr_typeCheck(e)->kind) {
                                            case TYPE_BOOLEAN:
                                                fprintf(file, "\n\tCALL print_boolean\n\n");
                                                break;
                                            case TYPE_CHARACTER:
                                                fprintf(file, "\n\tCALL print_character\n\n");
                                                break;
                                            case TYPE_INTEGER:
                                                fprintf(file, "\n\tCALL print_integer\n\n");
                                                break;
                                            case TYPE_STRING:
                                                fprintf(file, "\n\tCALL print_string\n\n");
                                                break;
                                            default:
                                                break;
                                        }
                                        break;
                                }
                                register_free(e->right->reg);
                                e = e->right;
                                break;
                            }

                            e = e->right;
                        }
                        fprintf(file, "\tPOPQ %r11\n");
                        fprintf(file, "\tPOPQ %r10\n\n");
                        break;
                    case EXPR_OR:
                    case EXPR_AND:
                    case EXPR_LT:
                    case EXPR_GT:
                    case EXPR_LTE:
                    case EXPR_GTE:
                    case EXPR_NOTEQUAL:
                    case EXPR_EQUAL:
                    case EXPR_NEGATE:
                    case EXPR_BOOL_LITERAL:
                        fprintf(file, "\n\tCALL print_boolean\n\n");
                        //e->right = NULL;
                        break;
                    case EXPR_ADD:
                    case EXPR_MULT:
                    case EXPR_DIV:
                    case EXPR_MOD:
                    case EXPR_EXP:
                    case EXPR_MINUS:
                    case EXPR_INCR:
                    case EXPR_DECR:
                    case EXPR_INTERGER_LITERAL:
                        fprintf(file, "\n\tCALL print_integer\n\n");
                        e->right = NULL;
                        break;
                    case EXPR_ASSIGN:
                        switch(expr_typeCheck(e)->kind) {
                            case TYPE_BOOLEAN:
                                fprintf(file, "\n\tCALL print_boolean\n\n");
                                break;
                            case TYPE_CHARACTER:
                                fprintf(file, "\n\tCALL print_character\n\n");
                                break;
                            case TYPE_INTEGER:
                                fprintf(file, "\n\tCALL print_integer\n\n");
                                break;
                            case TYPE_STRING:
                                fprintf(file, "\n\tCALL print_string\n\n");
                                break;
                            default:
                                fprintf(file, "\n\t# Tried to print but couldn't determine type\n\n");
                                break;
                        }
                        break;
                    case EXPR_FUNC_CALL:
                        switch(expr_typeCheck(e)->kind) {
                            case TYPE_BOOLEAN:
                                fprintf(file, "\n\tCALL print_boolean\n\n");
                                break;
                            case TYPE_CHARACTER:
                                fprintf(file, "\n\tCALL print_character\n\n");
                                break;
                            case TYPE_INTEGER:
                                fprintf(file, "\n\tCALL print_integer\n\n");
                                break;
                            case TYPE_STRING:
                                fprintf(file, "\n\tCALL print_string\n\n");
                                break;
                            default:
                                break;
                        }
                        break;
                    case EXPR_CHAR_LITERAL:
                        fprintf(file, "\n\tCALL print_character\n\n");
                        break;
                    case EXPR_STRING_LITERAL:
                        fprintf(file, "\n\tCALL print_string\n\n");
                        break;
                    case EXPR_NAME:
                        switch(expr_typeCheck(e)->kind) {
                            case TYPE_BOOLEAN:
                                fprintf(file, "\n\tCALL print_boolean\n\n");
                                break;
                            case TYPE_CHARACTER:
                                fprintf(file, "\n\tCALL print_character\n\n");
                                break;
                            case TYPE_INTEGER:
                                fprintf(file, "\n\tCALL print_integer\n\n");
                                break;
                            case TYPE_STRING:
                                fprintf(file, "\n\tCALL print_string\n\n");
                                break;
                            default:
                                break;
                        }
                        break;
                    /*
                    case EXPR_ARRAY:
                    case EXPR_ARRAY_LITERAL:
                        printf("No Array Support\n");
                        exit(1);
                        break;
                    */
                }
                if(e->kind != EXPR_ARG){
                    fprintf(file, "\tPOPQ %r11\n");
                    fprintf(file, "\tPOPQ %r10\n\n");
                    register_free(e->reg);
                }
                e = e->right;
            }

            break;
        case STMT_RETURN:
            expr_codegen(s->expr, file);
            fprintf(file, "\tMOVQ %s, %rax\t\t# Setup rax for returning\n", register_name(s->expr->reg));
            register_free(s->expr->reg);
            fprintf(file, "\n\t#### Prepare to return\n\n");
            fprintf(file, "\tPOPQ %r15\n");
            fprintf(file, "\tPOPQ %r14\n");
            fprintf(file, "\tPOPQ %r13\n");
            fprintf(file, "\tPOPQ %r12\n");
            fprintf(file, "\tPOPQ %rbx\n");
            fprintf(file, "\tMOVQ %rbp, %rsp\n");
            fprintf(file, "\tPOPQ %rbp\n");
            fprintf(file, "\tRET\n");
            break;
        case STMT_BLOCK:
            stmt_codegen(s->body, file);
            break;
    }

    // FREE ALL REG
    register_free(1);
    register_free(10);
    register_free(11);
    register_free(12);
    register_free(13);
    register_free(14);
    register_free(15);

    stmt_codegen(s->next, file);
}
int stmt_count_local_variables( struct stmt *s ) {
    if(!s) return 0;
    if(s->kind == STMT_DECL) {
        return 1 + stmt_count_local_variables(s->next);
    } else {
        return 0 + stmt_count_local_variables(s->next);
    }
}
void stmt_codegen_init( struct stmt *s, FILE *file ){
    if(!s) return;
    if (s->kind == STMT_DECL){
        if (s->decl){
            if (s->decl->value){
                if (s->decl->value->kind == EXPR_STRING_LITERAL){
                    s->decl->value->label = labelNum;
                    fprintf(file, "\n.L%d:\n", labelNum++);
                    fprintf(file, "\n\t.string\t%s\n", s->decl->value->string_literal);
                }
            }
        }
    }
    else if (s->expr){
        if (s->expr->kind == EXPR_STRING_LITERAL){
            s->expr->label = labelNum;
            fprintf(file, "\n.L%d:\n", labelNum++);
            fprintf(file, "\n\t.string\t%s\n", s->expr->string_literal);
        } else if (s->expr->kind == EXPR_ARG) {
            // check print args list
            struct expr *e = s->expr;
            while(e->right){
                if (e->left->kind == EXPR_STRING_LITERAL){
                    e->left->label = labelNum;
                    fprintf(file, "\n.L%d:\n", labelNum++);
                    fprintf(file, "\n\t.string\t%s\n", e->left->string_literal);
                }
                if(e->left->kind != EXPR_ARG && e->right->kind != EXPR_ARG && e->right->kind == EXPR_STRING_LITERAL){
                    e->right->label = labelNum;
                    fprintf(file, "\n.L%d:\n", labelNum++);
                    fprintf(file, "\n\t.string\t%s\n", e->right->string_literal);
                }
                e = e->right;
            }
        }
    }
    stmt_codegen_init(s->body, file);
    stmt_codegen_init(s->else_body, file);
    stmt_codegen_init(s->next, file);
}
