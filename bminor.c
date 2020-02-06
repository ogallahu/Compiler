/*
 * Owen 'ogallahu' Gallahue
 * bminor.c
 */
#include "token.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "utils/decl.h"

extern FILE *yyin;
extern int yylex();
extern char *yytext;
extern bool pPrinter;
extern bool resolve;
extern bool codeGen;
extern bool typeCheck;
extern int typeError;
extern int resolveError;
extern struct node *head;
extern struct string_node *string_head;
extern struct decl* parser_result;
int labelNum; 
char* isValidInt(char* text);
char* validString(char* text);
void printToken(enum yytokentype t, char* text, int mode);

int main(int argc, char * argv[])
{
    char *PROGRAM_NAME=argv[0];
    int mode = 0;
    if ((argc < 2)) {
            fprintf(stderr, "usage: %s -scan fileName\n", PROGRAM_NAME);
            return 1;
        }

        if (!strcmp(argv[1],"-codegen")) {
            codeGen = 1;
            pPrinter = 0;
            mode = 1;
	        yyin = fopen(argv[2],"r");
	        if(!yyin) {
		        printf("could not open %s!\n", argv[2]);
		        return 1;
	        }
        }
        else if (!strcmp(argv[1], "-help")){
            printf("usage: %s -scan fileName\n", PROGRAM_NAME);
            return 1;

        }else if (!strcmp(argv[1], "-scan") && argc ==3){
            mode = 0;
            pPrinter = false;
	        yyin = fopen(argv[2],"r");
	        if(!yyin) {
		        printf("could not open %s!\n", argv[2]);
		        return 1;
	        }
        }
        else if (!strcmp(argv[1], "-print") && argc ==3){
            mode = 1;
            pPrinter = true;
	        yyin = fopen(argv[2],"r");

	        if(!yyin) {
		        printf("could not open %s!\n", argv[2]);
		        return 1;
	        }
        }
        else if (!strcmp(argv[1], "-parse") && argc ==3){
            mode = 1;
            pPrinter = false;
	        yyin = fopen(argv[2],"r");
	        if(!yyin) {
		        printf("could not open %s!\n", argv[2]);
		        return 1;
	        }
        }
        else if (!strcmp(argv[1], "-resolve") && argc ==3){
            mode = 1;
            pPrinter = false;
            resolve = true;
	        yyin = fopen(argv[2],"r");
	        if(!yyin) {
		        printf("could not open %s!\n", argv[2]);
		        return 1;
	        }
        }
        else if (!strcmp(argv[1], "-typecheck") && argc ==3){
            mode = 1;
            pPrinter = false;
            typeCheck = true;
	        yyin = fopen(argv[2],"r");
	        if(!yyin) {
		        printf("could not open %s!\n", argv[2]);
		        return 1;
	        }
        }
        else{
            fprintf(stderr, "usage: %s -scan fileName\n", PROGRAM_NAME);
            return 1;
       } 
   
    if(mode == 0){ 
	    while(1) {
		    enum yytokentype t = yylex();
		    if(t==0) break;
            printToken(t, yytext, mode);
	    }
    }
    else if(resolve==1){
        if(yyparse() == 0 ){
                return resolveError;
        }else {
                printf("Resolve  Failed.\n");
                return 1;
        }
    }
    else if( codeGen == 1){
        printf("FLAG 1\n");
        if(yyparse() != 0 ){
                printf("CodeGen  Failed.\n");
                return 1;
        }

        int string_count = 0;
        struct string_node *sn;
        const char* filename = argv[2];
        char * output_filename=argv[3];
        typeError = 0;
        FILE *output;
        output = fopen(output_filename, "w");
        fprintf(output, ".file \"%s\"\n\n", filename);
        fprintf(output, ".data\n");
        // print out all literal strings into data
        /*
        sn = string_head;
        while(sn) {
            fprintf(output, "LC%d:\n", string_count++);
            fprintf(output, "\t.string %s\n",sn->text);
            sn = sn->next;
        }
        */
        printf("FLAG TYY\n");
        printf(parser_result);
        decl_global_data_codegen(parser_result, output);
        decl_codegen_init(parser_result,output);
        fprintf(output, "\n.text\n");
        labelNum = 0;
        decl_global_functions_codegen(parser_result, output);
        exit(0);

    }
    else if(typeCheck == 1){
        if(yyparse() == 0 ){
                return typeError + resolveError;
        }else {
                printf("Checker Failed.\n");
                return 1;
        }
    }
        else{
        if(yyparse() == 0 ){
                return 0;
        }else {
                printf("Parse Failed.\n");
                return 1;

        }
    }
    return 0;
}

void printToken(enum yytokentype t, char* text, int mode){
    char * keep;

    switch(t){
        case TOKEN_WHILE:
            if(mode == 0) printf("WHILE");
            break;
        case TOKEN_FOR:
            if(mode == 0) printf("FOR");
            break;
        case TOKEN_IF:
            if(mode == 0) printf("IF");
            break;
        case TOKEN_ELSE:
            if(mode == 0) printf("ELSE");
            break;
        case TOKEN_AUTO:
            if(mode == 0) printf("AUTO");
            break;
        case TOKEN_ARRAY:
            if(mode == 0) printf("ARRAY");
            break;
        case TOKEN_BOOLEAN:
            if(mode == 0) printf("BOOLEAN");
            break;
        case TOKEN_INTEGER_LITERAL:
            keep = text;
            text = isValidInt(text);
            if(!text){
                printf("scan error, interger %s out of range", text);
                text = keep;
                exit(1);
               } 
            else{ if(mode == 0) printf("INTEGER_LITERAL %s", text); }
            break;
        case TOKEN_STRING_LITERAL:
            keep = text;
            text = validString(text);
            if(!text){
                printf("scan error, %s is not an acceptable string", text);
                text = keep;
                exit(1);
               } 
            else{ if(mode == 0) printf("STRING_LITERAL %s", text); }
            break;
        case TOKEN_CHAR:
            printf("CHAR");
            break;
        case TOKEN_CHAR_LITERAL:
            keep = text;
            text = validString(text);
            if(!text){
                printf("scan error, %s is not an acceptable string", text);
                text = keep;
                exit(1);
               } 
            else{ if(mode == 0) printf("CHAR_LITERAL %s", text); }
            break;
        case TOKEN_INTEGER:
            if(mode == 0) printf("INTEGER");
            break;
        case TOKEN_STRING:
            if(mode == 0) printf("STRING");
            break;
        case TOKEN_VOID:
            if(mode == 0) printf("VOID");
            break;
        case TOKEN_FUNCTION:
            if(mode == 0) printf("FUNCTION");
            break;
        case TOKEN_PRINT:
            if(mode == 0) printf("PRINT");
            break;
        case TOKEN_RETURN:
            if(mode == 0) printf("RETURN");
            break;
        case TOKEN_TRUE:
            if(mode == 0) printf("TRUE");
            break;
        case TOKEN_FALSE:
            if(mode == 0) printf("FALSE");
            break;
        case TOKEN_LPAREN:
            if(mode == 0) printf("LEFT_PARENTHESE");
            break;
        case TOKEN_RPAREN:
            if(mode == 0) printf("RIGHT_PARENTHESE");
            break;
        case TOKEN_LBRACKET:
            if(mode == 0) printf("LEFT_BRACKET");
            break;
        case TOKEN_RBRACKET:
            if(mode == 0) printf("RIGHT_BRACKET");
            break;
        case TOKEN_LCURLY:
            if(mode == 0) printf("LEFT_CURLY");
            break;
        case TOKEN_RCURLY:
            if(mode == 0) printf("RIGHT_CURLY");
            break;
        case TOKEN_COMMA:
            if(mode == 0) printf("COMMA");
            break;
        case TOKEN_INCREMENT:
            if(mode == 0) printf("INCREMENT");
            break;
        case TOKEN_DECREMENT:
            if(mode == 0) printf("DECREMENT");
            break;
        case TOKEN_MINUS:
            if(mode == 0) printf("MINUS");
            break;
        case TOKEN_LOGICALNOT:
            if(mode == 0) printf("LOGICALNOT");
            break;
        case TOKEN_EXPONENT:
            if(mode == 0) printf("EXPONENT");
            break;
        case TOKEN_MULTIPLY:
            if(mode == 0) printf("MULTIPLY");
            break;
        case TOKEN_DIVIDE:
            if(mode == 0) printf("DIVIDE");
            break;
        case TOKEN_MODULUS:
            if(mode == 0) printf("MODULUS");
            break;
        case TOKEN_ADD:
            if(mode == 0) printf("ADD");
            break;
        case TOKEN_SUBTRACT:
            if(mode == 0) printf("SUBTRACT");
            break;
        case TOKEN_LT:
            if(mode == 0) printf("LESS_THAN");
            break;
        case TOKEN_LTE:
            if(mode == 0) printf("LESS_THAN_OR_EQUAL_TO");
            break;
        case TOKEN_GT:
            if(mode == 0) printf("GREATER_THAN");
            break;
        case TOKEN_GTE:
            if(mode == 0) printf("GREATER_THAN_OR_EQUAL_TO");
            break;
        case TOKEN_EQUAL:
            if(mode == 0) printf("EQUAL");
            break;
        case TOKEN_NOTEQUAL:
            if(mode == 0) printf("NOT_EQUAL");
            break;
        case TOKEN_COLON:
            if(mode == 0) printf("COLON");
            break;
        case TOKEN_SEMICOLON:
            if(mode == 0) printf("SEMICOLON");
            break;
        case TOKEN_LOGICALAND:
            if(mode == 0) printf("LOGICAL_AND");
            break;
        case TOKEN_LOGICALOR:
            if(mode == 0) printf("LOGICAL_OR");
            break;
        case TOKEN_ASSIGN:
            if(mode == 0) printf("ASSIGN");
            break;
        case TOKEN_IDENT:
            if(mode == 0) printf("IDENTIFIER %s", text);
            break;
        
        default:
            printf("scan error %s is not a valid charecter", text);
            exit(1);
            break;
    }
       if(mode == 0) printf("\n"); 
}
char * isValidInt(char * text){
    char buffer[256];
    bool minus = false;
    // strips the nu,ber of signs and leading zeros
    while( (*text == '0' || *text == '+' || *text == '-') && strlen(text) > 1 ){
        if(*text == '-'){
            minus = true;
        }
        text++;
    }
    // Neat trick, if overflow atoi will return -1 and becasue we skip signs above only error will get into this case
    int i = atoi(text);
    if(i == -1){
        return NULL;
    }
    // apply the signs
    if(minus){
        i *= -1;
    }
    sprintf(buffer, "%d",i); // gets the int back to a string
    text = strdup(buffer);
    return text;
}

char * validString(char * text){
    char * k;
    bool match = true;
    int c;
    if (strlen(text) > 255){
        return NULL;
    }
    // checks for not allowed escapes and non escaped quotes
    for(c = 0; c < strlen(text) - 1; c++){
        if(*(text + c) == '\\'){
            if(c + 2 == strlen(text)){
                return NULL;
            }
            if( *(text + c + 1) != '\'' && *(text + c + 1) != '\"' && *(text + c + 1) != 'n' && *(text + c + 1) != '0' ){
                strcpy(text + c, text + c + 1);
                
                c++;
                
            }
        }
        else if(*(text + c) != '\\'){
            if( ( *(text + c + 1) == '\'' || *(text + c + 1) == '\"' ) && c < strlen(text) - 2 ){
                if( *(text + c + 1) == '\'' && *(text) == '\"'){
                    continue;
                }
                return NULL;
            }
        }
        
    }
    // Swaps the allowed esacpes into ascii
    do{
        match = true;
        if( k = strstr(text,"\\n") ){
            match = false;
            *k = '\n';
            strcpy(k+1, k+2);
        }
        if( k = strstr(text,"\\0") ){
            match = false;
            *k = '\0';
            strcpy(k+1, k+2);
        }
        if( k = strstr(text,"\\\'") ){
            match = false;
            *k = '\'';
            strcpy(k+1, k+2);
        }
        if( k = strstr(text,"\\\"") ){
            match = false;
            *k = '\"';
            strcpy(k+1, k+2);
        }
    }
    while(!match);

    // get ride of the quotes
    text++;
    text[strlen(text) - 1] = '\0';
    
    return text;
}

