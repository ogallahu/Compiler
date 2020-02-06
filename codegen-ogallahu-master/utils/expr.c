// Expr.c
#include <stdio.h>
#include <string.h>
#include "expr.h"
#include <stdbool.h>
#include <stdlib.h>
#include "scope.h"
#include "type.h"
#include "scratch.h"

extern int resolveError;
extern int typeError;
extern int labelNum;
extern struct string_node *string_head;

int opPrec [30] =  {1, 2, 3, 4, 4, 4, 4, 4, 4, 5, 5, 6, 6, 6, 7, 8, 8, 9, 9, 10, 10, 10, 10, -1, -1, -1, -1, -1, -1, -1}; 
struct expr * expr_create( expr_t kind,
			   struct expr *left,
			   struct expr *right ){
  struct expr *e = calloc(1, sizeof(*e));
  e->kind = kind;
  e->left = left;
  e->right = right;
  //e->reg = -1;
  return e;
}

long long expr_to_int(char *text) {
    char *end;
    unsigned long val = strtoul(text, &end, 10);
}

void expr_resolve(struct expr *e){
    if(!e) return;
    if(e->kind == EXPR_NAME){
        e->symbol = scope_lookup(e->name);
        if(!e->symbol){
            printf("Resolve Error, variable (%s) does not exist\n", e->name); // add line numbers 
            resolveError=1;   
        }
    }else{
        expr_resolve(e->right);
        expr_resolve(e->left);
    }

}
void expr_delete(struct expr *e){
  if(e == NULL) return;

  if(e->left) expr_delete(e->left);
  if(e->right) expr_delete(e->right);
  free(e); 

}

struct expr * expr_create_name( const char *n){
  struct expr *e = expr_create(EXPR_NAME, NULL, NULL);
  e->name = strdup(n);
  return e;
}

struct expr * expr_create_integer_literal( long long c ){
    struct expr *e = expr_create(EXPR_INTERGER_LITERAL, NULL, NULL);
    e->literal_value = c;
    return e;
}

struct expr * expr_create_boolean_literal( long long unsigned c ){
  struct expr *e = expr_create(EXPR_BOOL_LITERAL, NULL, NULL);
  e->literal_value = c;
  return e;
}

struct expr * expr_create_char_literal( const char *c ){
    struct expr *e = expr_create(EXPR_CHAR_LITERAL, NULL, NULL);
    char *s = calloc(1, strlen(c) + 1);
    strcpy(s, c);
    expr_string_cleaner(s);

    e->string_literal = (char*)s;

    return e;
}

void expr_char_str_to_char( struct expr *e ){
    // move past single quote, get rid of end quote
    char *s = calloc(1, strlen(e->string_literal) + 1);
    strcpy(s, e->string_literal);
    s++;
    char *end = s + strlen(s) - 1;
    end--;
    end[1] = '\0';

    char res = s[0];

    // check for backwhack
    if (res == '\\'){
        switch(s[1]){
            case 'n':
                res = '\n';
                break;
            case '\'':
                res = '\'';
                break;
            case '0':
                res = '\0';
                break;
            default:
                break;
        }
    }
    e->literal_value = res;
}
void expr_string_cleaner(char *yytext){
    for(int c=0; c < strlen(yytext)-1;c++){
        if(*(yytext+c)=='\\'){
                if(c+2 == strlen(yytext)){
                    //Nothing 
                }

                if(*(yytext +c+1) != '\'' && *(yytext +c+1) != '\"' && *(yytext+c+1)!='n'
                    && *(yytext+c+1) != '0' && *(yytext +c+1) != '\\'){
                    strcpy(yytext+c, yytext+c+1);
                    c++;
                    }
                    
                if(*(yytext +c+1) == '\'' || *(yytext +c+1) == '\"' && *(yytext+c+1) != *yytext){
                    strcpy(yytext+c, yytext+c+1);
                    c++;
                }

        } else if( *(yytext+c)!='\\'){
            if( (*yytext +c + 1) == '\'' || *(yytext+c+1)=='\"' && c < strlen(yytext)-2){
                if(*(yytext +c + 1) == '\'' && *(yytext)=='\"'){
                    continue;
                }
                //return NULL;
            }
        }
    }
}




                
struct expr * expr_create_string_literal( const char *s ){
  struct expr *e = expr_create(EXPR_STRING_LITERAL, NULL, NULL);
  e->string_literal = strdup(s); 
  return e;
}

/* Collaborated with jfox, elopez, ogallahu and ginglis */
void expr_print(struct expr * e){
    if(!e){
        return;
    }

     if(e->left && e->left->kind == EXPR_IN_PARENS){
    if(e->left->left){
      if(opPrec[e->left->left->kind] >= opPrec[e->kind] && e->kind != EXPR_MINUS){
    e->left = e->left->left;
      }
    }
    else{
      e->left = 0;
    }  
    }
    if(e->right && e->right->kind == EXPR_IN_PARENS){
        if(e->right->left){
          if(opPrec[e->right->left->kind] > opPrec[e->kind]){
        e->right = e->right->left;
      }
    }
    else{
      e->right = 0;
     }
    }

    // edge cases
    if(e->kind == EXPR_LOGICALNOT){ printf("!"); }
    if(e->kind == EXPR_IN_PARENS){ printf("( "); }
    if(e->kind == EXPR_ARRAY_INIT){ printf("{"); }
    if(e->kind == EXPR_NEGATE){ printf("-"); }
    expr_print(e->left);
      

    switch(e->kind){
        case(EXPR_ADD): 
            printf(" + ");
            break;
            
        case(EXPR_MINUS): 
            printf(" - ");
            //if a -(-a)
            if(e->right->kind == EXPR_NEGATE){ printf(" "); }
            break;
            
        case(EXPR_MULT): 
            printf(" * ");
            break;
            
        case(EXPR_MOD): 
            printf(" % ");
            break;
            
        case(EXPR_DIV): 
            printf(" / ");
            break;

        case EXPR_IN_PARENS:
            printf(" )");
            break;
            
        case(EXPR_INTERGER_LITERAL): 
            printf("%lld", e->literal_value);
            break;
            
        case(EXPR_NAME):
            if(e->name){ 
                printf("%s", e->name);
            }
            break;
            
        case(EXPR_CHAR_LITERAL): 
            if(e->string_literal){
                  printf("%s", e->string_literal);
                          }
            break;
            
        case(EXPR_STRING_LITERAL): 
            printf("%s", e->string_literal);
            break;
            
            
        case EXPR_BOOL_LITERAL:
             e->literal_value ? printf("true") : printf("false");
             break;

        case(EXPR_INCR): 
            printf("++");
            break;
            
        case(EXPR_DECR): 
            printf("--");
            break;
            
        case(EXPR_EXP): 
            printf("^");
            break;
            
        case(EXPR_GT): 
            printf(">");
            break;
            
        case(EXPR_LT): 
            printf("<");
            break;
            
        case(EXPR_GTE): 
            printf(">=");
            break;
            
        case(EXPR_LTE): 
            printf("<=");
            break;
            
        case(EXPR_EQUAL): 
            printf("==");
            break;
            
        case(EXPR_NOTEQUAL): 
            printf("!=");
            break;
            
        case(EXPR_ASSIGN): 
            printf("= ");
            break;
            
        case(EXPR_AND): 
            printf("&&");
            break;
            
        case(EXPR_OR): 
            printf("||");
            break;
            
        case(EXPR_LOGICALNOT): 
            printf("!");
            break;
            
        case EXPR_PRINT:
            if(e->right){
              printf(", ");
            }
            break;
        
        case EXPR_ARRAY_ACCESS:
            printf("[");
            break;
        
        case EXPR_ARRAY_INIT:
            break;

        case EXPR_FUNC_CALL:
            printf("(");
            break;
        case EXPR_NEGATE:
            break;
        default:
            fprintf(stderr, "Error: unknown expression, could not be printed\n");
            break; 

    }
    expr_print(e->right);
    if(e->kind == EXPR_FUNC_CALL){
        printf(")");
    }
    else if(e->kind == EXPR_ARRAY_ACCESS){
        printf("] ");
    }
    else if(e->kind == EXPR_ARRAY_INIT){
        printf("}");
    }
}
struct type * expr_typeCheck( struct expr *e )
{
    if(!e) return 0;
    struct type *lt = expr_typeCheck(e->left);
    struct type *rt = expr_typeCheck(e->right);
    struct type *result = NULL;
    switch(e->kind) {
        case EXPR_INTERGER_LITERAL:
            result = type_create(TYPE_INTEGER,0,0,0);
            expr_print(e);
            break;
        case EXPR_STRING_LITERAL:
            result = type_create(TYPE_STRING,0,0,0);
            break;
        /* more cases here */
        case EXPR_ADD:
            if(lt->kind == TYPE_INTEGER && rt->kind == TYPE_INTEGER) {
                    result = type_create(TYPE_INTEGER,0,0,0);
            }else{
                    printf("Type error: can not add non-integers together\n");
                    typeError=1;
            }
            break;

        case EXPR_MINUS:
            if(lt->kind == TYPE_INTEGER && rt->kind == TYPE_INTEGER) {
                    result = type_create(TYPE_INTEGER,0,0,0);
            }else{
                    printf("Type error: can not subtract non-integers together\n");
                    typeError=1;
            }
            break;

        case EXPR_MULT:
            if(lt->kind == TYPE_INTEGER && rt->kind == TYPE_INTEGER) {
                    result = type_create(TYPE_INTEGER,0,0,0);
            }else{
                    printf("Type error: can not multiply non-integers together\n");
                    typeError=1;
            }
            break;

        case EXPR_DIV:
            if(lt->kind == TYPE_INTEGER && rt->kind == TYPE_INTEGER) {
                    result = type_create(TYPE_INTEGER,0,0,0);
            }else{
                    printf("Type error: can not divide non-integers together\n");
                    typeError=1;
            }
            break;

        case EXPR_MOD:
            if(lt->kind == TYPE_INTEGER && rt->kind == TYPE_INTEGER) {
                    result = type_create(TYPE_INTEGER,0,0,0);
            }else{
                    printf("Type error: can not modulus  non-integers together\n");
                    typeError=1;
            }
            break;

        case EXPR_EXP:
            if(lt->kind == TYPE_INTEGER && rt->kind == TYPE_INTEGER) {
                    result = type_create(TYPE_INTEGER,0,0,0);
            }else{
                    printf("Type error: can not exponante  non-integers together\n");
                    typeError=1;
            }
            break;
    
        case EXPR_INCR:
            if(lt->kind == TYPE_INTEGER) {
                    result = type_create(TYPE_INTEGER,0,0,0);
            }else{
                    printf("Type error: can not increment non-integers\n");
                    typeError=1;
            }
            break;
        
        case EXPR_DECR:
            if(lt->kind == TYPE_INTEGER) {
                    result = type_create(TYPE_INTEGER,0,0,0);
            }else{
                    printf("Type error: can not decrement non-integers\n");
                    typeError=1;
            }
            break;

        case EXPR_CHAR_LITERAL:
            result = type_create(TYPE_CHARACTER,0,0,0);
            break;

        case EXPR_BOOL_LITERAL:
            result = type_create(TYPE_BOOLEAN,0,0,0);
            break;
            
        case EXPR_GT:
            if(type_equal(lt,rt) && lt->kind == TYPE_INTEGER ){
                result = type_create(TYPE_BOOLEAN,0,0,0);
            } else {
                printf("Type Error: can't compare non ints.\n");
                    typeError=1;
            }
            break;

        case EXPR_LT:
            if(type_equal(lt,rt) && lt->kind == TYPE_INTEGER ){
                result = type_create(TYPE_BOOLEAN,0,0,0);
            } else {
                printf("Type Error: can't compare non ints.\n");
                    typeError=1;
            }
            break;

        case EXPR_LTE:
            if(type_equal(lt,rt) && lt->kind == TYPE_INTEGER ){
                result = type_create(TYPE_BOOLEAN,0,0,0);
            } else {
                printf("Type Error: can't compare non ints.\n");
                    typeError=1;
            }
            break;

        case EXPR_GTE:
            if(type_equal(lt,rt) && lt->kind == TYPE_INTEGER ){
                result = type_create(TYPE_BOOLEAN,0,0,0);
            } else {
                printf("Type Error: can't compare non ints.\n");
                    typeError=1;
            }
            break;

        case EXPR_EQUAL:
            if(type_equal(lt,rt)){
                result = type_create(TYPE_BOOLEAN,0,0,0);
            } else {
                printf("Type Error: comparing two unlike types.\n");
                    typeError=1;
            }
            break;
            
        case EXPR_NOTEQUAL:
            if(type_equal(lt,rt)){
                result = type_create(TYPE_BOOLEAN,0,0,0);
            } else {
                printf("Type Error: comparing two unlike types.\n");
                    typeError=1;
            }
            break;

        case EXPR_AND:
            if(type_equal(lt,rt) && (lt->kind == TYPE_INTEGER) ){
                result = type_create(TYPE_BOOLEAN,0,0,0);
            } else {
                printf("Type Error: can't compare non ints.\n");
                    typeError=1;
            }
            break;

        case EXPR_OR:
            if(type_equal(lt,rt) && (lt->kind == TYPE_INTEGER) ){
                result = type_create(TYPE_BOOLEAN,0,0,0);
            } else {
                printf("Type Error: can't compare non ints.\n");
                    typeError=1;
            }
            break;

        case EXPR_LOGICALNOT:
            if(type_equal(lt,rt) && (lt->kind == TYPE_INTEGER || lt->kind == TYPE_BOOLEAN) ){
                result = type_create(TYPE_BOOLEAN,0,0,0);
            } else {
                printf("Type Error: can't compare non ints.\n");
                    typeError=1;
            }
            break;

        case EXPR_IN_PARENS:
            result = lt;
            break;

        case EXPR_ASSIGN:
             if(type_equal(lt,rt)){
                result = type_create(lt->kind,0,0,0);
              } else {
                printf("Type error: assignment types wrong\n"); 
                    typeError=1;
              }
              break;

        case EXPR_NAME:
             if(e->symbol){
                result = e->symbol->type;
             }
             else{
                 printf("TYPE ERROR, trying to access variable that doesn't exist\n");
             }
             break;

        case EXPR_ARRAY_INIT:
            result=lt;
            break;
        case EXPR_ARRAY_ACCESS:
            if(!lt || !rt){
                printf("TYPECHECK ERROR: Array indexing must be an integer ");
                result = lt->subtype;
                typeError=1;
            } else if (!type_equal(rt, type_create(TYPE_INTEGER, 0, 0,0 ))){
                printf("TYPECHECK ERROR: Array indexing must be an integer ");
                result = lt->subtype;
                typeError=1;
            } else { result = lt;}

            break;

        case EXPR_PRINT:
            /* TODO */
            
            break;

        case EXPR_FUNC_CALL:
            if(!lt) return NULL;
            return lt->subtype; 
            break;
        case EXPR_NEGATE:
            if(lt->kind == TYPE_INTEGER ){
                result = type_create(TYPE_INTEGER,0,0,0);
            } else {
                printf("Type Error: can't negate non ints.\n");
                    typeError=1;
            }
            break;
            

}
    return result;
}


void expr_codegen(struct expr *e, FILE *file) {
    int count = 1;
    char* string_label = (char*)malloc(9);
    char* label_name = (char*)malloc(9);
    struct expr *temp_ptr;

    if(!e) return;
    puts("       droppping into switch insdie expr_codegen");
    switch(e->kind) {
        case EXPR_LIST:
            break;
        case EXPR_ASSIGN:
            expr_codegen(e->right, file);
            fprintf(file, "\tMOV %s, %s\t\t# Assignment\n", register_name(e->right->reg), symbol_codegen(e->left->symbol));
            e->reg = e->right->reg;
            register_free(e->left->reg);
            break;
        case EXPR_OR:
            expr_codegen(e->left, file);
            expr_codegen(e->right, file);
            fprintf(file, "\tOR %s, %s\n", register_name(e->left->reg), register_name(e->right->reg));
            e->reg = e->right->reg;
            register_free(e->left->reg);
            break;
        case EXPR_AND:
            expr_codegen(e->left, file);
            expr_codegen(e->right, file);
            fprintf(file, "\tAND %s, %s\n", register_name(e->left->reg), register_name(e->right->reg));
            e->reg = e->right->reg;
            register_free(e->left->reg);
            break;
        case EXPR_LT:
            expr_codegen(e->left, file);
            expr_codegen(e->right, file);
            fprintf(file, "\tCMP %s, %s\t\t\n", register_name(e->left->reg), register_name(e->right->reg));
            fprintf(file, "\tMOV $1, %s\n", register_name(e->right->reg));
            sprintf(label_name, "done_%d", labelNum);
            fprintf(file, "\tJG %s\n", label_name);
            fprintf(file, "\tMOV $0, %s\n", register_name(e->right->reg));
            fprintf(file, "%s:\n", label_name);
            e->reg = e->right->reg;
            register_free(e->left->reg);
            labelNum++;
            break;
        case EXPR_GT:
            expr_codegen(e->left, file);
            expr_codegen(e->right, file);
            fprintf(file, "\tCMP %s, %s\t\t\n", register_name(e->left->reg), register_name(e->right->reg));
            fprintf(file, "\tMOV $1, %s\n", register_name(e->right->reg));
            fprintf(file, "\tMOV $1, %s\n", register_name(e->right->reg));
            sprintf(label_name, "done_%d", labelNum);
            fprintf(file, "\tJL %s\n", label_name);
            fprintf(file, "\tMOV $0, %s\n", register_name(e->right->reg));
            fprintf(file, "%s:\n", label_name);
            e->reg = e->right->reg;
            register_free(e->left->reg);
            labelNum++;
            break;
        case EXPR_LTE:
            expr_codegen(e->left, file);
            expr_codegen(e->right, file);
            fprintf(file, "\tCMP %s, %s\t\t\n", register_name(e->left->reg), register_name(e->right->reg));
            fprintf(file, "\tMOV $1, %s\n", register_name(e->right->reg));
            fprintf(file, "\tMOV $1, %s\n", register_name(e->right->reg));
            sprintf(label_name, "done_%d", labelNum);
            fprintf(file, "\tJGE %s\n", label_name);
            fprintf(file, "\tMOV $0, %s\n", register_name(e->right->reg));
            fprintf(file, "%s:\n", label_name);
            e->reg = e->right->reg;
            register_free(e->left->reg);
            labelNum++;
            break;
        case EXPR_GTE:
            expr_codegen(e->left, file);
            expr_codegen(e->right, file);
            fprintf(file, "\tCMP %s, %s\t\t\n", register_name(e->left->reg), register_name(e->right->reg));
            fprintf(file, "\tMOV $1, %s\n", register_name(e->right->reg));
            fprintf(file, "\tMOV $1, %s\n", register_name(e->right->reg));
            sprintf(label_name, "done_%d", labelNum);
            fprintf(file, "\tJLE %s\n", label_name);
            fprintf(file, "\tMOV $0, %s\n", register_name(e->right->reg));
            fprintf(file, "%s:\n", label_name);
            e->reg = e->right->reg;
            register_free(e->left->reg);
            labelNum++;
            break;
        case EXPR_NOTEQUAL:
            expr_codegen(e->left, file);
            expr_codegen(e->right, file);
            fprintf(file, "\tCMP %s, %s\t\t\n", register_name(e->left->reg), register_name(e->right->reg));
            fprintf(file, "\tMOV $1, %s\n", register_name(e->right->reg));
            fprintf(file, "\tMOV $1, %s\n", register_name(e->right->reg));
            sprintf(label_name, "done_%d", labelNum);
            fprintf(file, "\tJNE %s\n", label_name);
            fprintf(file, "\tMOV $0, %s\n", register_name(e->right->reg));
            fprintf(file, "%s:\n", label_name);
            e->reg = e->right->reg;
            register_free(e->left->reg);
            labelNum++;
            break;
        case EXPR_EQUAL:
            expr_codegen(e->left, file);
            expr_codegen(e->right, file);
            fprintf(file, "\tCMP %s, %s\t\t\n", register_name(e->left->reg), register_name(e->right->reg));
            fprintf(file, "\tMOV $1, %s\n", register_name(e->right->reg));
            fprintf(file, "\tMOV $1, %s\n", register_name(e->right->reg));
            sprintf(label_name, "done_%d", labelNum);
            fprintf(file, "\tJE %s\n", label_name);
            fprintf(file, "\tMOV $0, %s\n", register_name(e->right->reg));
            fprintf(file, "%s:\n", label_name);
            e->reg = e->right->reg;
            register_free(e->left->reg);
            labelNum++;
            break;
        case EXPR_ADD:
            expr_codegen(e->left, file);
            expr_codegen(e->right, file);
            fprintf(file, "\tADD %s, %s\n", register_name(e->left->reg), register_name(e->right->reg));
            e->reg = e->right->reg;
            register_free(e->left->reg);
            break;
        case EXPR_MINUS:
            expr_codegen(e->left, file);
            expr_codegen(e->right, file);
            fprintf(file, "\tSUB %s, %s\n", register_name(e->right->reg), register_name(e->left->reg));
            e->reg = e->left->reg;
            register_free(e->right->reg);
            break;
        case EXPR_MULT://TODO FIX BAD REG
            puts("IN expr_mult in expr");
            expr_codegen(e->left, file);
            puts("      compled first expr_codeGen IN expr_mult in expr");
            expr_codegen(e->right, file);
            fprintf(file, "\tMOV %s, %rax\t\t#Move left value into rax to prepare for multiplying\n", register_name(e->left->reg)); //e->left
            fprintf(file, "\tIMUL %s\t\t# Multiply %rax by the value in %s\n", register_name(e->right->reg), register_name(e->right->reg));//e->right
            fprintf(file, "\tMOV %rax, %s\t\t# Move multiplied result back to non-scratch register\n", register_name(e->right->reg)); //e->right
            e->reg = e->right->reg;
            puts("right before free in EXPR_MULT in expr_code GEn");
            register_free(e->left->reg);
            puts("right AFTER free in EXPR_MULT in expr_code GEn");
            break;
        case EXPR_DIV:
            expr_codegen(e->left, file);
            expr_codegen(e->right, file);
            fprintf(file, "\tMOVQ %s, %rax\t\t#Move left value into rax to prepare for dividing\n", register_name(e->left->reg));
            fprintf(file, "\tCQTO\n");
            fprintf(file, "\tIDIVQ %s\t\t\t# Divide %rax by the value in %s\n", register_name(e->right->reg), register_name(e->right->reg));
            fprintf(file, "\tMOVQ %rax, %s\t\t# Move divided result back to non-scratch register\n", register_name(e->right->reg));
            e->reg = e->right->reg;
            register_free(e->left->reg);
            break;
        case EXPR_MOD:
            expr_codegen(e->left, file);
            expr_codegen(e->right, file);
            fprintf(file, "\tMOVQ %s, %rax\t\t#Move left value into rax to prepare for dividing\n", register_name(e->left->reg));
            fprintf(file, "\tCQTO\n");
            fprintf(file, "\tIDIVQ %s\t\t\t# Divide %rax by the value in %s\n", register_name(e->right->reg), register_name(e->right->reg));
            fprintf(file, "\tMOVQ %rdx, %s\t\t# Move remainder back to non-scratch register\n", register_name(e->right->reg));
            e->reg = e->right->reg;
            register_free(e->left->reg);
            break;
        case EXPR_EXP:
            expr_codegen(e->left, file);
            expr_codegen(e->right, file);
            fprintf(file, "\tMOV %s, %rdi\t\t# Move first argument for base of power\n", register_name(e->left->reg));
            fprintf(file, "\tMOV %s, %rsi\t\t# Move second argument for exponent\n", register_name(e->right->reg));
            fprintf(file, "\n\tPUSHQ %r10\n");
            fprintf(file, "\tPUSHQ %r11\n");
            fprintf(file, "\n\tCALL integer_power\n\n");
            fprintf(file, "\tPOPQ %r11\n");
            fprintf(file, "\tPOPQ %r10\n");
            fprintf(file, "\tMOV %rax, %s\t\t# Move result of integer_power into register\n", register_name(e->right->reg));
            e->reg = e->right->reg;
            register_free(e->left->reg);
            break;
        case EXPR_NEGATE:
            expr_codegen(e->right, file);
            fprintf(file, "\tNEG %s\t\t# Negate the value\n", register_name(e->right->reg));
            e->reg = e->right->reg;
            break;
        case EXPR_LOGICALNOT:
            expr_codegen(e->right, file);
            fprintf(file, "\tSUB $1, %s\t\t# Negate the boolean\n", register_name(e->right->reg));
            e->reg = e->right->reg;
            break;
        case EXPR_INCR:
            // TODO: Delay incrementing
            expr_codegen(e->left, file);
            fprintf(file, "\tINC %s\n", register_name(e->left->reg));
            fprintf(file, "\tMOV %s, %s\n", register_name(e->left->reg), symbol_codegen(e->left->symbol));
            e->reg = e->left->reg;
            break;
        case EXPR_DECR:
            expr_codegen(e->left, file);
            fprintf(file, "\tDEC %s\n", register_name(e->left->reg));
            fprintf(file, "\tMOV %s, %s\n", register_name(e->left->reg), symbol_codegen(e->left->symbol));
            e->reg = e->left->reg;
            break;
        case EXPR_FUNC_CALL:
            // setup function args
            fprintf(file, "\tMOV $0, %rax\n");
            temp_ptr = e->right;            
            while(temp_ptr) {
                if(count > 6) {
                    printf("You may only supple 6 arguments to a function\n");
                    exit(1);
                }
                expr_codegen(temp_ptr->left, file);
                fprintf(file, "\tMOV %s, %s\n", register_name(temp_ptr->left->reg), num_to_arg(count++));
                temp_ptr = temp_ptr->right;
            }

            // call function
            fprintf(file, "\n\tPUSH %r10\n");
            fprintf(file, "\tPUSH %r11\n");
            fprintf(file, "\n\tCALL %s\n\n", e->left->name);
            fprintf(file, "\tPOP %r11\n");
            fprintf(file, "\tPOP %r10\n");
            e->reg = register_alloc();
            fprintf(file, "\tMOV %rax, %s\n", register_name(e->reg));
            break;
        case EXPR_BOOL_LITERAL:
            e->reg = register_alloc();          
            fprintf(file, "\tMOV $%d,  %s\n", e->literal_value, register_name(e->reg));
            break;
        case EXPR_INTERGER_LITERAL:
            puts("Integer literal in expr_code_gen");
            e->reg = register_alloc();          
            fprintf(file, "\tMOV $%d,  %s\n", e->literal_value, register_name(e->reg));
            puts("Done wiht int in expr_code_egn");
            break;
        case EXPR_CHAR_LITERAL:
            e->reg = register_alloc();
            expr_char_str_to_char(e);     
            fprintf(file, "\tMOV $%d,  %s\n", e->literal_value, register_name(e->reg));
            break;
        case EXPR_STRING_LITERAL:

            e->reg = register_alloc();
            fprintf(file, "\nMOVQ $.L%d,  %s\n", e->label, register_name(e->reg));

            break;
       
        case EXPR_NAME:
            e->reg = register_alloc();
            fprintf(file, "\tMOV %s, %s\n", symbol_codegen(e->symbol), register_name(e->reg));
            break;
        case EXPR_ARRAY_INIT:
            break;
        case EXPR_ARRAY_ACCESS:
            break;  
        case EXPR_IN_PARENS:
            expr_codegen(e->left, file);
            e->reg = e->left->reg;
            break;
    }
}
