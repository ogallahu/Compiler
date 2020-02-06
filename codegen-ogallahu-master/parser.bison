/*
Owen 'ogallahu' Gallahue
parser.bison
*/

%{
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "token.h"
#include "utils/decl.h"
#include "utils/stmt.h"
#include "utils/symbol.h"
#include "utils/type.h"
#include "utils/param_list.h"
#include <string.h>



extern char *yytext;
extern int yylex();
extern int yyerror( char *str );
bool pPrinter= false;
bool resolve = false;
bool typeCheck = false;
int typeError = 0;
int resolveError=0;
struct decl* parser_result;
bool codeGen;

%}
%token TOKEN_EOF
%token TOKEN_WHILE
%token TOKEN_FOR 
%token TOKEN_IF
%token TOKEN_AUTO
%token TOKEN_ARRAY
%token TOKEN_BOOLEAN
%token TOKEN_CHAR
%token TOKEN_INTEGER
%token TOKEN_STRING
%token TOKEN_VOID
%token TOKEN_FUNCTION
%token TOKEN_PRINT
%token TOKEN_RETURN
%token TOKEN_TRUE
%token TOKEN_FALSE
%token TOKEN_LPAREN
%token TOKEN_RPAREN
%token TOKEN_LBRACKET
%token TOKEN_RBRACKET
%token TOKEN_LCURLY
%token TOKEN_RCURLY
%token TOKEN_COMMA
%token TOKEN_INCREMENT
%token TOKEN_DECREMENT
%token TOKEN_MINUS
%token TOKEN_LOGICALNOT
%token TOKEN_EXPONENT
%token TOKEN_MULTIPLY
%token TOKEN_DIVIDE
%token TOKEN_MODULUS
%token TOKEN_ADD
%token TOKEN_SUBTRACT
%token TOKEN_LT 
%token TOKEN_LTE 
%token TOKEN_GT 
%token TOKEN_GTE
%token TOKEN_EQUAL
%token TOKEN_NOTEQUAL
%token TOKEN_COLON
%token TOKEN_SEMICOLON
%token TOKEN_LOGICALAND
%token TOKEN_LOGICALOR
%token TOKEN_ASSIGN
%token TOKEN_IDENT
%token TOKEN_INTEGER_LITERAL
%token TOKEN_ERROR
%token TOKEN_BACKSLASH
%token TOKEN_STRING_LITERAL
%token TOKEN_CHAR_LITERAL
%token TOKEN_ELSE

%union {
    struct decl *decl;
    struct stmt *stmt;
    struct expr *expr;
    struct type *type;
    struct param_list *param_list;
    struct symbol *symbol;
}

%type <decl> program decls decl
%type <stmt> return print stmt stmts matched unmatched other_stmt for
%type <expr> assign_expr atom  or_expr and_expr eq_expr rel_expr add_expr multi_expr exp_expr unary_expr post_expr def expr exprs integer_literal string_literal char_literal for_param ident
%type <type> type typeFunc
%type <param_list> func_param  next


%%

program 
    :  decls
        { 
          if(pPrinter){decl_print($1,0);}
          else if(codeGen) {decl_resolve($1);} 
          else if(resolve) {decl_resolve($1);}
          else if(typeCheck) { 
                decl_resolve($1); 
                if(resolveError ==0) {
                    decl_typeCheck($1);
                } else return 1;
            } 
        }
    | //Epsilon
        { $$ = NULL;}
    ;

decls
    : decl decls 
        { ($1)->next = $2; parser_result = $1;} 
    | decl
        {parser_result = $1;}
    ;


decl : ident TOKEN_COLON type TOKEN_ASSIGN def
        {
            $$ = decl_create(strdup($1->name), $3, $5, 0, 0 );
        }
    /*| ident TOKEN_COLON type TOKEN_ASSIGN def
        {
            $$ = decl_create( strdup($1->name), $3, 0, 0, 0 );
        }*/
    | ident TOKEN_COLON typeFunc TOKEN_ASSIGN TOKEN_LCURLY stmts TOKEN_RCURLY
        { 
            //expr_print($1);
            //type_print($3);
            //($6)->next = $7;
            //puts("delc stmt");
            //stmt_print($6, 0);
            //puts("delc stmt");
            $$ = decl_create( strdup($1->name), $3, 0, $6, 0 );
        }
    | ident TOKEN_COLON typeFunc TOKEN_SEMICOLON
        {
            $$ = decl_create( strdup($1->name), $3, 0, 0, 0 );
        }
    | ident TOKEN_COLON type TOKEN_SEMICOLON
        {
            $$ = decl_create( strdup($1->name), $3, 0, 0, 0 );
        }
    ;

typeFunc : TOKEN_FUNCTION type TOKEN_LPAREN func_param TOKEN_RPAREN
    { $$ = type_create(TYPE_FUNCTION,$2,$4, NULL); }

def : expr TOKEN_SEMICOLON
        { 
            $$ = $1; 

            while( ($$) && ($$)->kind == EXPR_IN_PARENS ){
                $$ = ($$)->left;
            }
        
        }
    /*| TOKEN_ASSIGN TOKEN_LCURLY exprs TOKEN_RCURLY
        { $$ = expr_create_array_init($3); } */
    | TOKEN_SEMICOLON
        { $$ = 0; }
    ;

stmts 
    : stmt stmts
        { $1->next = $2; $$ = $1;}
    | // epsilon
        { $$ = NULL; }
    ;

stmt        
    : matched
        { $$ = $1; }
    | unmatched
        { $$ = $1; }
    ;

matched     
    : TOKEN_IF TOKEN_LPAREN expr TOKEN_RPAREN matched TOKEN_ELSE matched
          { struct expr * e = $3;
           
           while( (e) && (e)->kind == EXPR_IN_PARENS){
             e = (e)->left;
           }
           
           $$ = stmt_create(STMT_IF_ELSE, 0, 0, e, 0, $5, $7, 0); }
    | other_stmt
        { $$ = $1; }
    ;

unmatched   
    : TOKEN_IF TOKEN_LPAREN expr TOKEN_RPAREN matched
        {$$ = stmt_create(STMT_IF_ELSE, 0, 0, $3, 0, $5, 0, 0); }
    | TOKEN_IF TOKEN_LPAREN expr TOKEN_RPAREN unmatched
        {$$ = stmt_create(STMT_IF_ELSE, 0, 0, $3, 0, $5, 0, 0); }
    | TOKEN_IF TOKEN_LPAREN expr TOKEN_RPAREN matched TOKEN_ELSE unmatched
        {$$ =  stmt_create(STMT_IF_ELSE, 0, 0, $3, 0, $5, $7, 0);}
    ;

other_stmt  
    : TOKEN_LCURLY stmts TOKEN_RCURLY
        { $$ = stmt_create(STMT_BLOCK, 0, 0, 0, 0, $2, 0, 0); }
    | return TOKEN_SEMICOLON
        { $$ = $1; }
    | print TOKEN_SEMICOLON
        { $$ = $1; }
    | for
        { $$ = $1; }
    | expr TOKEN_SEMICOLON
        { $$ = stmt_create(STMT_EXPR, 0, 0, $1, 0, 0, 0, 0); }
    | decl
        {$$ = stmt_create(STMT_DECL, $1, 0, 0, 0, 0, 0, 0);}
    ;

return 
    : TOKEN_RETURN expr
        { $$ = stmt_create(STMT_RETURN, 0, 0, $2, 0, 0, 0, 0);}
    | TOKEN_RETURN
        { $$ = stmt_create(STMT_RETURN, 0, 0, 0, 0, 0, 0, 0); }
    ;

print  
    : TOKEN_PRINT exprs
        { $$ = stmt_create(STMT_PRINT, 0, 0, $2, 0, 0, 0, 0); }
    | TOKEN_PRINT
        { $$ = stmt_create(STMT_PRINT, 0, 0, 0, 0, 0, 0, 0); } 
    ;

for 
    : TOKEN_FOR TOKEN_LPAREN for_param TOKEN_SEMICOLON for_param TOKEN_SEMICOLON for_param TOKEN_RPAREN stmt
        { $$ =  stmt_create(STMT_FOR, 0, $3, $5, $7, $9, 0, 0); }   
    ;

for_param  
    : expr {         
      $$ = $1; 
      while( ($$) && ($$)->kind == EXPR_IN_PARENS){
        $$ = ($$)->left;
      }
    }
    | // epsilon
        {$$ = 0;}
    ;

assign_expr : or_expr
        { $$ = $1; }
    | assign_expr TOKEN_ASSIGN or_expr
        { 
       struct expr *e = $3;
       while( (e) && (e)->kind == EXPR_IN_PARENS){//was maybe EXPR_ARGS?
     e = (e)->left;
       }
      $$ = expr_create(EXPR_ASSIGN, $1, e); 
    }
    | assign_expr TOKEN_ASSIGN TOKEN_LCURLY exprs TOKEN_RCURLY
        { $$ = expr_create(EXPR_ASSIGN, $1, expr_create(EXPR_ARRAY_INIT, $4, 0)); }
    | assign_expr TOKEN_ASSIGN TOKEN_LCURLY TOKEN_RCURLY
        { $$ = expr_create(EXPR_ASSIGN, $1, expr_create(EXPR_ARRAY_INIT, 0, 0)); }
    ; 


type        
    : TOKEN_INTEGER
        {$$ = type_create(TYPE_INTEGER, 0, 0, 0);}
    | TOKEN_BOOLEAN
        {$$ = type_create(TYPE_BOOLEAN, 0, 0, 0);}
    | TOKEN_CHAR
        {$$ = type_create(TYPE_CHARACTER, 0, 0, 0);}
    | TOKEN_STRING
        {$$ = type_create(TYPE_STRING, 0, 0, 0);}
    | TOKEN_FUNCTION type TOKEN_LPAREN func_param TOKEN_RPAREN
        { $$ = type_create(TYPE_FUNCTION, $2, $4, 0); }
    | TOKEN_ARRAY TOKEN_LBRACKET expr TOKEN_RBRACKET type
        {$$ = type_create(TYPE_ARRAY, $5, NULL, $3); }
    | TOKEN_ARRAY TOKEN_LBRACKET TOKEN_RBRACKET type
        {$$ = type_create(TYPE_ARRAY, $4, NULL, NULL); }
    | TOKEN_AUTO
        { $$ = type_create(TYPE_AUTO, 0, 0, 0); }
    | TOKEN_VOID
        {$$ = type_create(TYPE_VOID, 0, 0, 0);}
    ;

func_param 
    : ident TOKEN_COLON type next
        {$$ = param_list_create(strdup(($1)->name), $3, $4);}
    | /* epsilon */
        { $$ = 0; }
    ;

next  
    : TOKEN_COMMA ident TOKEN_COLON type next
        {$$ = param_list_create(strdup(($2)->name), $4, $5);}
    | /* epsilon */
        { $$ = 0;}
    ;


/*def
    : TOKEN_ASSIGN expr
        {  $$ = $2;

         while( ($$) && ($$)->kind == EXPR_IN_PARENS){
              $$ = ($$)->left;
         }
        }

    | TOKEN_ASSIGN TOKEN_LCURLY exprs TOKEN_RCURLY 
        {$$ = expr_create(EXPR_ARRAY_INIT, $3, 0);}
    | // epsilon
        { $$ = NULL;}
    ;
*/

/*exprs 
    : expr TOKEN_COMMA exprs 
        { struct expr *e = $1;   
    while((e) && (e)->kind == EXPR_IN_PARENS ){
      e = (e)->left;
    }
    
    $$ = expr_create(EXPR_PRINT, e, $3);
        
      }
   | expr { 
        $$ = $1; 
    while(($$) && ($$)->kind == EXPR_IN_PARENS ){
      $$ = ($$)->left;
    }
    
      }
   ; 
*/
exprs
    : expr TOKEN_COMMA exprs
        {$$ = expr_create(EXPR_ARG, $1, $3);}
    | expr
        { $$ = $1; }

expr        
    : assign_expr
        { $$=$1; }
    ;

or_expr     
    : and_expr
        { $$=$1; }
    | or_expr TOKEN_LOGICALOR and_expr
        { $$ = expr_create(EXPR_OR, $1, $3); }
    ;

and_expr    
    : eq_expr
        { $$=$1; }
    | and_expr TOKEN_LOGICALAND eq_expr
        { $$ = expr_create(EXPR_AND, $1, $3); }
    ;

eq_expr     
    : rel_expr
        { $$=$1; }
    | eq_expr TOKEN_EQUAL rel_expr
        { $$ = expr_create(EXPR_EQUAL, $1, $3); }
    | eq_expr TOKEN_NOTEQUAL rel_expr
        { $$ = expr_create(EXPR_NOTEQUAL, $1, $3); }
    ;

rel_expr    
    : add_expr
        { $$=$1; }
    | rel_expr TOKEN_LT add_expr
        { $$ = expr_create(EXPR_LT, $1, $3); }
    | rel_expr TOKEN_LTE add_expr
        { $$ = expr_create(EXPR_LTE, $1, $3); }
    | rel_expr TOKEN_GT add_expr
        { $$ = expr_create(EXPR_GT, $1, $3); }
    | rel_expr TOKEN_GTE add_expr
        { $$ = expr_create(EXPR_GTE, $1, $3); }
    ;

add_expr    
    : multi_expr
        { $$=$1; }
    | add_expr TOKEN_ADD multi_expr
        { $$ =expr_create(EXPR_ADD, $1, $3); }
    | add_expr TOKEN_MINUS multi_expr
        { $$ =expr_create(EXPR_MINUS, $1, $3); }
    ;

multi_expr    
    : exp_expr
        { $$=$1; }
    | multi_expr TOKEN_MULTIPLY exp_expr
        { $$ =expr_create(EXPR_MULT, $1, $3); }
    | multi_expr TOKEN_DIVIDE exp_expr
        { $$ =expr_create(EXPR_DIV, $1, $3); }
    | multi_expr TOKEN_MODULUS exp_expr
        { $$ =expr_create(EXPR_MOD, $1, $3); }

exp_expr    
    : unary_expr
        { $$=$1; }
    | exp_expr TOKEN_EXPONENT unary_expr
        { $$ = expr_create(EXPR_EXP, $1, $3); }
    ;

unary_expr  
    : post_expr
        { $$=$1; }
    | TOKEN_LOGICALNOT unary_expr
        { $$ = expr_create(EXPR_LOGICALNOT, $2, 0); }
    | TOKEN_MINUS unary_expr
        { $$ = expr_create(EXPR_NEGATE, $2, 0); }
    ;

post_expr   
    : atom
        { $$=$1; }
    | post_expr TOKEN_LBRACKET expr TOKEN_RBRACKET
        { $$ = expr_create(EXPR_ARRAY_ACCESS, $1, $3); }
    | ident  TOKEN_LPAREN TOKEN_RPAREN
        { $$ = expr_create(EXPR_FUNC_CALL, $1, NULL); }
    | ident  TOKEN_LPAREN exprs TOKEN_RPAREN
        { $$ = expr_create(EXPR_FUNC_CALL, $1, $3); }
    | post_expr TOKEN_INCREMENT
        { $$ = expr_create(EXPR_INCR, $1, NULL); }
    | post_expr TOKEN_DECREMENT
        { $$ = expr_create(EXPR_DECR, $1, NULL); }
    ;

ident
    : TOKEN_IDENT
        { $$ =expr_create_name(strdup(yytext)); }
    ;
string_literal 
    : TOKEN_STRING_LITERAL 
        { $$ = expr_create_string_literal(strdup(yytext)); }
    ; 

char_literal 
    : TOKEN_CHAR_LITERAL 
        { $$ = expr_create_char_literal(strdup(yytext)); }
    ; 
integer_literal 
        : TOKEN_INTEGER_LITERAL
            { $$ = expr_create_integer_literal(expr_to_int(yytext)); }
        ; 
 
atom      
    : integer_literal
        { $$ = $1; }
    | string_literal
        { $$ = $1; }
    | char_literal
        { $$ = $1; }
    | TOKEN_TRUE
        { $$ = expr_create_boolean_literal(1); }
    | TOKEN_FALSE
        { $$ = expr_create_boolean_literal(0); }
    | ident
        { $$=$1; }
    | TOKEN_LPAREN expr TOKEN_RPAREN
        {
            if(!($2)){ $$ = 0; }   
            else if( ($2)->kind == EXPR_IN_PARENS){ $$ = $2; }   
            else { $$ = expr_create(EXPR_IN_PARENS, $2, NULL); }
        
         }
    | TOKEN_LCURLY exprs TOKEN_RCURLY
        { $$ = expr_create(EXPR_ARRAY_INIT,$2, NULL); }
    ;

%%
int yyerror( char *str )
{
    printf("parse error: %s\n",str);
    return 1;
}
