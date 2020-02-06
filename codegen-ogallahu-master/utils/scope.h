#ifndef SCOPE_H
#define SCOPE_H

#include "symbol.h"
#include "hash_table.h"

struct scope_node{
    struct hash_table *ht;
    struct scope_node *next;

    int level;
    int num_vars;
};

void scope_enter();
void scope_exit();
int scope_level();
void scope_reset();

void scope_bind( const char *name, struct symbol *sym );
struct symbol * scope_lookup( const char *name );
struct symbol * scope_lookup_current( const char *name );

#endif
