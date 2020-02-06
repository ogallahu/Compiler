/*
 * Owen 'ogallahu' Gallahue
 * scanner.flex
 */
%{
#include "token.h"

%}

DIGIT  [0-9]
LETTER [a-zA-Z_]+[a-zA-Z_0-9]* 

%%
(" "|\t|\r|\n)


"/*"    { /* Ignore multiline comments: https://efxa.org/2013/05/05/ignoring-multiline-comments-with-flex/ */
                register int c;
                for (;;)
                  {
                    while ((c = input ()) != '*' && c != EOF && c != 0) {
                    }
                    if (c == '*')
                    {
                    while ((c = input ()) == '*')
                        ;
                        if (c == '/')
                        break;
                    }
                    if (c == EOF || c == 0){
                    printf("Scan Error: Unclosed multiLine Comment\n");
                    exit(1);
                    break;
            }
    }
    }


\/\/.*

while      { return TOKEN_WHILE; }
for        { return TOKEN_FOR; }
if         { return TOKEN_IF; }
else       { return TOKEN_ELSE; }

auto       { return TOKEN_AUTO; }
array      { return TOKEN_ARRAY; }
boolean    { return TOKEN_BOOLEAN; }
char       { return TOKEN_CHAR; }
integer    { return TOKEN_INTEGER; }
string     { return TOKEN_STRING; }
void       { return TOKEN_VOID; }

function   { return TOKEN_FUNCTION; }
print      { return TOKEN_PRINT; }
return     { return TOKEN_RETURN; }

true       { return TOKEN_TRUE; }
false      { return TOKEN_FALSE; }

\(         { return TOKEN_LPAREN; }
\)         { return TOKEN_RPAREN; }
\[         { return TOKEN_LBRACKET; }
\]         { return TOKEN_RBRACKET; }
\{         { return TOKEN_LCURLY; }
\}         { return TOKEN_RCURLY; }


\-          { return TOKEN_MINUS; }
\+         { return TOKEN_ADD; }
!          { return TOKEN_LOGICALNOT; }

\+\+       { return TOKEN_INCREMENT; }
--         { return TOKEN_DECREMENT; }

\^         { return TOKEN_EXPONENT; }
\*         { return TOKEN_MULTIPLY; }
\/         { return TOKEN_DIVIDE; }
%          { return TOKEN_MODULUS; }

\\         { return TOKEN_BACKSLASH; }

\<         { return TOKEN_LT; }
\<=        { return TOKEN_LTE; }
\>         { return TOKEN_GT; }
\>=        { return TOKEN_GTE; }
==         { return TOKEN_EQUAL; }
!=         { return TOKEN_NOTEQUAL; }

:          { return TOKEN_COLON; }
;          { return TOKEN_SEMICOLON; }
,          { return TOKEN_COMMA; }

&&         { return TOKEN_LOGICALAND; }
\|\|       { return TOKEN_LOGICALOR; }
=          { return TOKEN_ASSIGN; }

\"([^\"\n\\]|\\\n|\\.)*\"     { return TOKEN_STRING_LITERAL; }
\'\\?.\'       { return TOKEN_CHAR_LITERAL; }

[+-]?{DIGIT}+   { return TOKEN_INTEGER_LITERAL; }
{LETTER}+       { return TOKEN_IDENT; }

.          { printf("Scan error"); exit(1); }
%%
int yywrap() { return 1; }
