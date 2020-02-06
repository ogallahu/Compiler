//symbols.c
#include "symbol.h"
#include <stdlib.h>
#include "string.h"

struct symbol * symbol_create( symbol_t kind, struct type *type, char *name) {
    struct symbol *s = calloc(1, sizeof(*s));
    if (s == NULL) return NULL;
    s->kind  = kind;
    s->type  = type;
    s->name = strdup(name);
    return s;
}

struct symbol * symbol_delete( struct symbol *s ) {
    /* Check if null */
    if (s == NULL) return NULL;
    free(s->name);
    free(s);
    return NULL;
}
char* symbol_codegen( struct symbol *s ) {
    int stack_diff;
    char *result = (char*)malloc(256);
    if(!s) {
        printf("No symbol given\n");
        exit(1);
    }
    switch(s->kind) {
        case SYMBOL_LOCAL:
            //fall through
        case SYMBOL_PARAM:
            stack_diff = 8 * s->which;
            sprintf(result, "-%d(%rbp)", stack_diff);
            return result;
        case SYMBOL_GLOBAL:
            if(s->type->kind == TYPE_STRING) {
                sprintf(result, "$%s", s->name);
                return result;
            }
            return s->name;
    }
}
