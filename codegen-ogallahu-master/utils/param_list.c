//param_list.c
#include "param_list.h"
#include <stdlib.h>
#include <string.h>
#include "scope.h"




struct param_list * param_list_create( char *name, struct type *type, struct param_list *next ){
       /* allocate decl, check if allocation fails */
    struct param_list *new_p_list = calloc(1, sizeof(struct param_list));
    if (new_p_list == NULL) return NULL;
    /* strdup name */
    new_p_list->name = strdup(name);

    /* assign the rest */
    new_p_list->type = type;
    new_p_list->next = next;

    return new_p_list;
} 
void param_list_resolve ( struct param_list *a ) {
    if(!a) return;
    struct symbol *s = symbol_create(SYMBOL_PARAM, a->type, a->name);
    scope_bind(a->name, s);
    param_list_resolve(a->next);

}
void param_list_print ( struct param_list *a ) {
    printf("( ");
    while (a) {
        printf("%s: ", a->name);
        type_print(a->type);

        if (a->next) {
            printf(", ");
        }
        a = a->next;
    }
    printf(")");
}

struct param_list * param_list_delete( struct param_list *a, bool recursion){
    return NULL;
}
int param_list_equal(struct param_list *a, struct param_list *b){
    if((a&&!b) || (!a&&b)) return 0;
    if(!a && !b) return 1;
    return type_equal(a->type, b->type) && param_list_equal(a->next, b->next);


}
