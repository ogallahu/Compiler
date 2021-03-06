#ifndef TYPE_H
#define TYPE_H

#include "param_list.h"
#include <stdbool.h>
typedef enum {
	TYPE_VOID,
	TYPE_BOOLEAN,
	TYPE_CHARACTER,
	TYPE_INTEGER,
	TYPE_STRING,
	TYPE_ARRAY,
	TYPE_FUNCTION,
    TYPE_AUTO
} type_t;

struct type {
	type_t kind;
	struct param_list *params;
	struct type *subtype;
    struct expr *array_init_expr;
};

void          type_print( struct type *t );
struct type * type_create( type_t kind, struct type *subtype, struct param_list *params, struct expr *array_init_expr);
struct type * type_delete( struct type *t);
int type_equal(struct type *a, struct type *b);


#endif
