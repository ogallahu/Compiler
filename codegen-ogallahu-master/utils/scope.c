#include <stdlib.h>
#include <string.h>
#include "scope.h"


struct scope_node *top;

void scope_enter(){
    struct hash_table *ht = hash_table_create(0, 0);
    struct scope_node *sn = calloc(1, sizeof(*sn));

    sn->ht = ht;
    sn->next = NULL;
    sn->num_vars = 0;

    if(!top){
        sn->level = 1;
        top = sn;
    }
    else{
        sn->level = top->level + 1;
        if(sn->level > 1){ 
            sn->num_vars = top->num_vars;
        }else {
            sn->num_vars=0;
        }

        sn->next = top;
        top = sn;
    }
}

void scope_exit(){
    /* pop from top of scope stack using hash_table_delete */
    struct scope_node *sn = top->next;
    hash_table_delete(top->ht);
    top = sn;
}

int scope_level(){
    /* return level of scope, aka number of hash tables */
    if (!top) scope_enter();
    return top->level;
}

void scope_bind( const char *name, struct symbol *sym ){
    /* add a symbol to the topmost hash table */
   if (!top) scope_enter();
   hash_table_insert(top->ht, name, sym);



   if(top->level > 1) top->num_vars++;

   
   /* Print resolving */
	printf("%s resolves to ", name);
	switch(sym->kind){
		case SYMBOL_GLOBAL:
			printf("global %s\n", name);
			break;
		case SYMBOL_LOCAL:
            sym->which = top->num_vars;
			printf("local %d\n", top->num_vars);
			break;
		case SYMBOL_PARAM:
            sym->which = top->num_vars;
			printf("param %d\n", top->num_vars);
			break;
		default:
			break;
	}
}

struct symbol * scope_lookup( const char *name ){
    /* look in all hash tables for a given symbol name */
    /* return null if no match found */
    if (!top) scope_enter();
    struct scope_node *tmp = top;
    while (tmp){
        if(hash_table_lookup(tmp->ht, name)){
            return hash_table_lookup(tmp->ht, name);
        }
        tmp = tmp->next;
    }
    return NULL;
}

struct symbol * scope_lookup_current( const char *name ){
    /* look in only topmost hash table for a given symbol name */
    /* used to check if a symbol already exists in current scope */
    /* return null if no match found */
    if (!top) return NULL;
        return hash_table_lookup(top->ht, name);
}

void scope_reset(){
    top->num_vars=0;
}
