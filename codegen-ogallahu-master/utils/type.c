//TYpe.c
#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "param_list.h"
#include "expr.h"

struct type * type_create( type_t kind, struct type *subtype, struct param_list *params, struct expr *array_init_expr ) {
    struct type *t = calloc(1, sizeof(*t));

    t->kind   = kind;
    t->subtype   = subtype;
    t->params = params;
    t->array_init_expr = array_init_expr;

    return t;
}

struct type * type_delete( struct type *t ) {
    //if(t){
    //    if(t->subtype) type_delete(t->subtype);
    //    if(t->params) param_list_delete(t->params ,1);
    //    if(t->array_init_expr) expr_delete(t->array_init_expr);
    //    free(t);
    //}
    return NULL;
}

void type_print ( struct type *t ) {
    switch (t->kind) {
        case TYPE_VOID:
            printf("void");
            break;
        case TYPE_BOOLEAN:
            printf("boolean");
            break;
        case TYPE_CHARACTER:
            printf("char");
            break;
        case TYPE_INTEGER:
            printf("integer");
            break;
        case TYPE_STRING:
            printf("string");
            break;
        case TYPE_ARRAY:
            printf("array [");
            expr_print(t->array_init_expr);
            printf("] ");
            break;
        case TYPE_FUNCTION:
            printf("function ");
            break;
        case TYPE_AUTO:
            printf("auto ");
            break;
    }
    if (t->subtype) {
        type_print(t->subtype);
    }
    if (t->kind == TYPE_FUNCTION) {
        param_list_print(t->params);
    }
}

int type_equal(struct type *a, struct type *b){
    if (a->kind == TYPE_FUNCTION ){
            return type_equal(a->subtype, b->subtype) && param_list_equal(a->params, b->params);
        }

    if((a&&!b) || (!a&&b)) return 0;
    if(!a && !b) return 1;
    if(a->kind == b->kind) return 1;
    if(a->kind != b->kind) return 0;

    if(a->kind == TYPE_ARRAY && b->kind == TYPE_ARRAY){
        return type_equal(a->subtype, b->subtype);
    }

    if(a->kind == TYPE_ARRAY && b->kind != TYPE_ARRAY){
        return type_equal(a->subtype, b);
    }

    if(b->kind == TYPE_ARRAY && a->kind != TYPE_ARRAY){
        return type_equal(a->subtype, b->subtype);
    }
    
    return type_equal(a, b->subtype);


}
