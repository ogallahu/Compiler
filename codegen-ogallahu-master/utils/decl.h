
#ifndef DECL_H
#define DECL_H

#include "type.h"
#include "stmt.h"
#include "expr.h"
#include <stdio.h>
#include <stdbool.h>
#include "scope.h"

struct decl {
	char *name;
	struct type *type;
	struct expr *value;
	struct stmt *code;
	struct symbol *symbol;
	struct decl *next;
};

struct decl * decl_create( char *name, struct type *type, struct expr *value, struct stmt *code,  struct decl *next );
    
void decl_delete( struct decl *d);
void decl_print(struct decl * d, int indent);
void decl_resolve(struct decl *d);
void decl_typeCheck(struct decl *d);
void decl_codegen(struct decl *d, FILE *file);
void decl_global_data_codegen(struct decl *d, FILE *file);
void decl_global_functions_codegen(struct decl *d, FILE *file);
#endif
