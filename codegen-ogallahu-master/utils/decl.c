#include <stdlib.h>
#include <string.h>
#include "decl.h"
#include "stmt.h"
#include "scope.h"
#include "symbol.h"
#include "scratch.h"
extern int typeError;
extern int resolveError;
extern struct decl* parser_result;
struct decl * decl_create( char *name, struct type *type, struct expr *value, struct stmt *code, struct decl *next ) {
    /* allocate decl, check if allocation fails */
    struct decl *new_decl = calloc(1, sizeof(struct decl));

    /* strdup name */
    new_decl->name = strdup(name);

    /* assign the rest */
    new_decl->type = type;
    new_decl->value = value;
    new_decl->code = code;
    new_decl->next = next;

    return new_decl;
}

void decl_resolve( struct decl *d) {
    if(!d) return;
    symbol_t kind = scope_level() > 1 ? SYMBOL_LOCAL : SYMBOL_GLOBAL;

    d->symbol = symbol_create(kind, d->type, d->name);
    if(!(d->name)){
        fprintf(stderr,"Resolve Error: unamed decl\n");
        resolveError = 1;
        return;
    }
    if(!(d->type)){
        fprintf(stderr,"Resolve Error: decl has no type\n");
        resolveError = 1;
        return;
    }
    
    if(d->value){
        expr_resolve(d->value);
    } 

    if(d->code){
        struct symbol *sym = scope_lookup_current(d->name);
        if(sym){
            if(sym->prototype){ printf("Resolve Error: Bad redeclaration of function %s\n", d->name); resolveError=1;}
            
        } else {
            d->symbol->prototype = 1;
        }
        
        scope_bind(d->name, d->symbol);
        scope_enter();
        param_list_resolve(d->type->params);
        scope_reset();
        stmt_resolve(d->code);
        scope_exit();
    } else {
        struct symbol *sym = scope_lookup_current(d->name);
        if(sym){
            printf("Resolve Error: Bad redeclaration of function %s\n",d->name);
            resolveError=1;
        }
        d->symbol->prototype = 0;
        scope_bind(d->name, d->symbol);
    }
    decl_resolve(d->next);
}
void decl_delete( struct decl *d) {
    /* check if null */
    if ( !d ){
        return;
    }
    /*
    decl_delete(d->next);

    free(d->name);
    type_delete(d->type);
    expr_delete(d->value);

    free(d);
    */
    return;
}

void decl_print(struct decl *d, int nestitude){
    for(int i = 0; i < nestitude; i++) {
        printf("\t");
    }

    while(d){

        printf("%s: ", d->name);
        type_print(d->type);
        if (d->value){
            printf(" = ");
            expr_print(d->value);
            printf(";");
        } else if (d->code){
            printf(" = ");
            printf("{\n");
            stmt_print(d->code, nestitude + 1);
            printf("}\n");
        } else {
            printf(";");
        }
        printf("\n");


        d = d->next;
    }
}
// Shout out Jfox and gingles for the help here.
void decl_typeCheck( struct decl *d ){
    if(d == NULL){
        return;
    }
    if(d->code){
        stmt_typeCheck(d->code, d);
    }

    if(d->value){
        struct type * t = expr_typeCheck(d->value);
        if(d->type->subtype){
            if( !type_equal(d->type->subtype, t) ){
                printf("Type Error: Assignment types must match.\n");
                typeError=1;
            }
        }
        else if( !type_equal(d->type, t) ){
            printf("Type Error: Assignment types must match.\n");
            typeError=1;
        }
    }

    // Check if prototype matches actual definition in return type
    if(d->type && !d->code){
        if(d->type->kind == TYPE_FUNCTION){
            struct decl *nxt = d->next;
            while(nxt){
                if(nxt->type && nxt->code){
                    if(!strcmp(d->name, nxt->name)){
                        if(!type_equal(nxt->type, d->type)){
                            printf("[TYPE ERROR] Prototype for function %s does not match definition", d->name);
                            typeError=1;
                        }
                    }
                }
                nxt = nxt->next;
            }
        }
    if (d->next) decl_typeCheck(d->next);
}
}

void decl_codegen(struct decl *d, FILE *file) {
    int reg;
    printf("INSDIE DECL_CODE GEN\n");
    if(!d) return;
    switch(d->type->kind) {
        case TYPE_BOOLEAN:
            /* fall through */
        case TYPE_CHARACTER:
            /* fall through */
        case TYPE_INTEGER:
            if(d->value) {
                expr_codegen(d->value, file);
                printf("Inside type_token in decl_codeGen\n");
                fprintf(file, "\tMOVQ %s, %s\t\t# Setting variable %s\n", register_name(d->value->reg), symbol_codegen(d->symbol), d->symbol->name);
                puts("       after the fprintf");
                register_free(d->value->reg);
            } else {
                fprintf(file, "\tMOVQ $0, %s\t\t# Setting default value for var %s\n", symbol_codegen(d->symbol), d->symbol->name);
            }
            break;
        case TYPE_STRING:
            expr_codegen(d->value, file);
            fprintf(file, "\tMOVQ %s, %s\t\t# Setting string %s\n", register_name(d->value->reg), symbol_codegen(d->symbol), d->symbol->name);
            register_free(d->value->reg);
            break;
        case TYPE_ARRAY:
            printf("Arrays are not currently supported in this compiler\n");
            exit(1);
        case TYPE_FUNCTION:
            /* fall through */
        case TYPE_VOID:
            /* sanity check */
            printf("Should not be possible to have a void or a function declaration occur within this function\n");
            exit(1);
    }
    decl_codegen(d->next, file);
}

void decl_global_functions_codegen(struct decl *d, FILE *file) {
    printf("inside decl_global_funciton_codegen\n");
    int local_vars, count = 1;
    struct param_list *param_ptr;
    if(!d) return;
    printf("D not null\n");
    if (d->type->kind == TYPE_FUNCTION) {
            printf("insdie if inside decl global\n");
            fprintf(file, "\n.global %s\n", d->name);
            if(d->code) {
                fprintf(file, ".type %s, @function\n%s:\n", d->name, d->name);
                fprintf(file, "\n\t#### Function preamble\n\n");
                fprintf(file, "\tPUSHQ %rbp\n");
                fprintf(file, "\tMOVQ %rsp, %rbp\n\n");

                param_ptr = d->type->params;
                while(param_ptr) {
                    if(count > 6) {
                        printf("You may only supply six function arguments\n");
                        exit(1);
                    }
                    fprintf(file, "\tPUSHQ %s\n", num_to_arg(count++));
                    param_ptr = param_ptr->next;
                }
                
                local_vars = stmt_count_local_variables(d->code);
                fprintf(file, "\tSUBQ $%d, %rsp\t\t#allocate %d more local variables\n\n", 8*local_vars, local_vars);

                fprintf(file, "\tPUSHQ %rbx\n");
                fprintf(file, "\tPUSHQ %r12\n");
                fprintf(file, "\tPUSHQ %r13\n");
                fprintf(file, "\tPUSHQ %r14\n");
                fprintf(file, "\tPUSHQ %r15\n");

                fprintf(file, "\n\t#### Main function body\n\n");
                stmt_codegen(d->code, file);

                fprintf(file, "\n\t#### Prepare to return\n\n");
                fprintf(file, "\tPOPQ %r15\n");
                fprintf(file, "\tPOPQ %r14\n");
                fprintf(file, "\tPOPQ %r13\n");
                fprintf(file, "\tPOPQ %r12\n");
                fprintf(file, "\tPOPQ %rbx\n");
                fprintf(file, "\tMOVQ %rbp, %rsp\n");
                fprintf(file, "\tPOPQ %rbp\n");
                fprintf(file, "\tRET\n");
            }
    }
    decl_global_functions_codegen(d->next, file);
}

void decl_codegen_init(struct decl * d, FILE *file){
    if(!d) return;

    if(d->code){
        stmt_codegen_init(d->code, file);
    }
    decl_codegen_init(d->next, file);

}

void decl_global_data_codegen(struct decl *d, FILE *file) {
    struct stmt *s;
    struct expr *e;
    if(!d) return;
    switch(d->type->kind) {
        case TYPE_CHARACTER:
            fprintf(file, "%s:\n", d->name);
            if(d->value) {
                expr_create_char_literal(d->value);
                puts("After cleanign string in decl_global");
                fprintf(file, "\t.quad %d\n", d->value->literal_value);
            }

        case TYPE_BOOLEAN:
            /* fall through */
            
        case TYPE_INTEGER:
            fprintf(file, "%s:\n", d->name);
            if(d->value) {
                fprintf(file, "\t.quad %d\n", d->value->literal_value);
            }
            break;
        case TYPE_STRING:
            // All literal strings are handled by main.c
            // Here we only need to handle declared strings that aren't set equal to anything
            if(!d->value) {
                fprintf(file, "%s:\n", d->name);
                fprintf(file, "\t.string \"\"\n");
            } else {
                fprintf(file, "%s:\n", d->name);
                fprintf(file, "\t.string %s\n", d->value->string_literal);
            }
            break;
        case TYPE_ARRAY:
            printf("No array support\n");
            exit(1);
        case TYPE_FUNCTION:
        case TYPE_VOID:
            break;
        default:
            break;
    }
    decl_global_data_codegen(d->next, file);
}




