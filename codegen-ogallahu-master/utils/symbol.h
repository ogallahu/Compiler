
#ifndef SYMBOL_H
#define SYMBOL_H

#include "type.h"

typedef enum {
	SYMBOL_LOCAL,
	SYMBOL_PARAM,
	SYMBOL_GLOBAL
} symbol_t;

struct symbol {
	symbol_t kind;
	struct type *type;
	char *name;
	int which;
    int num_occ;
    int prototype;
};

struct symbol * symbol_create( symbol_t kind, struct type *type, char *name);
struct symbol * symbol_delete(struct symbol *s);
char* symbol_codegen( struct symbol *s );
#endif
