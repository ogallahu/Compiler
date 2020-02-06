#ifndef EXPR_H
#define EXPR_H

#include "symbol.h"
#include <stdbool.h>
#include "type.h"
typedef enum {
	EXPR_ADD =9,
	EXPR_MINUS=10,
	EXPR_MULT=11,
	EXPR_DIV=12,
    EXPR_NAME=23,
    EXPR_INTERGER_LITERAL=24,
    EXPR_CHAR_LITERAL=26,
    EXPR_STRING_LITERAL=27,
    EXPR_BOOL_LITERAL=25,
    EXPR_INCR=17,
    EXPR_DECR=18,
    EXPR_MOD=13,
    EXPR_EXP=14,
    EXPR_GT=3,
    EXPR_LT=4,
    EXPR_LTE=5,
    EXPR_GTE=6,
    EXPR_EQUAL=7,
    EXPR_NOTEQUAL=8,
    EXPR_ASSIGN=0,
    EXPR_AND=2,
    EXPR_OR=1,
    EXPR_LOGICALNOT=16,
    EXPR_ARRAY_ACCESS=19,
    EXPR_FUNC_CALL=22,
    EXPR_IN_PARENS=21,
    EXPR_ARRAY_INIT=20,
    EXPR_LIST=28,
    EXPR_PRINT=29,
    EXPR_NEGATE=15,
    EXPR_ARG = 30
} expr_t;

struct expr {
	/* used by all kinds of exprs */
	expr_t kind;
	struct expr *left;
	struct expr *right;
    int reg;
	/* used by various leaf exprs */
	const char *name;
	long long literal_value;
	const char * string_literal;
	struct symbol *symbol;
    int label;
};

struct expr * expr_create( expr_t kind, struct expr *left, struct expr *right );
void expr_delete( struct expr *e);
void expr_resolve( struct expr *e);

struct expr * expr_create_name( const char *n);
struct expr * expr_create_integer_literal( long long c );
struct expr * expr_create_boolean_literal( long long unsigned c );
struct expr * expr_create_char_literal( const char *c );
struct expr * expr_create_string_literal( const char *s );
long long expr_to_int(char *text);
void expr_print( struct expr *e );
struct type * expr_typeCheck( struct expr *e );
void expr_codegen( struct expr*, FILE*);
void expr_string_cleaner(char *yytext);
void stmt_codegen_init(struct stmt* s, FILE *file);
#endif
