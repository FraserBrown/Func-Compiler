#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"

int symb;

extern int yylex(void);	/* returns the next token */
extern FILE * yyin;		/* the input file given */
extern char * yytext;	/* the last match is stored in yytext string */
extern void printSymb(void);


/**
 * NODE DEFINITIONS
 * */
#define NODE struct node
struct branches {NODE * n1; NODE * n2; };
union fields {struct branches b; int value; char * id;};
struct node {int tag; union fields f;};


NODE * newName(char * i){
	NODE * n;
	char * cur = strdup(i);
	n = (NODE *)malloc(sizeof(NODE));
	n->tag = NAME;
	n->f.id = cur;
	return n;
}

NODE * newNumber(char * i){
	NODE * n;
	char * cur = strdup(i);
	n = (NODE *)malloc(sizeof(NODE));
	n->tag = NAME;
	n->f.id = cur;
	return n;
}

NODE * newNode(int tag) {  
	NODE * n;
	n = (NODE *)malloc(sizeof(NODE));
	n->tag = tag;
	n->f.b.n1 = NULL;
	n->f.b.n2 = NULL;
	return n;
}

/**
 * Match Tokens to strings, return relevant string for token
 * */
char * showSymb(int symb){
	switch(symb){  
		case  FUNCTION: return "FUNCTION";
		case  RETURN: return "RETURN";
		case  IS: return "IS";
		case  TBEGIN: return "BEGIN";
		case  END: return "END";
		case  INTEGER: return "INTEGER";
		case  ARRAYOFSIZE: return "ARRAYOFSIZE";
		case  READ: return "READ";
		case  WRITE: return "WRITE";
		case  LSQBRA: return "[";
		case  RSQBRA: return " ] ";
		case  ASSIGN: return " := ";
		case  COLON: return " : ";  
		case  SEMI:	return ";";
		case  COMMA: return ",";
		case  IF: return "IF";
		case  THEN: return "THEN";
		case  ELSE: return "ELSE";
		case  WHILE: return "WHILE";
		case  LOOP: return "LOOP";
		case  LPAREN: return "(";
		case  RPAREN: return ")";
		case  LT: return "LessThan";
		case  LE: return "LessEqual";
		case  EQ: return "Equal";
		case  NEQ: return "NotEqual"; 
		case GT: return "GreaterThan";
		case GEQ: return "GreaterEqual";
		case NAME: return "NAME";
		case NUMBER: return "NUMBER";	
		case MAIN: return "MAIN";	
		case EOF: return "EOF";
		default: printf("bad symbol: %d",symb);
   }
}

/**
 * PRINTING TREE
 * */
void showTree(NODE * tree,int depth) { 
	int i;
	if(tree==NULL) {
		return;
	}
	for(i=0;i<depth;i++){
		putchar('-');
	}
	switch(tree->tag){
		case NAME: printf("%s\n",tree->f.id); return;
		case NUMBER: printf("%d\n",tree->f.value);return;
		default: printf("%s\n",showSymb(tree->tag));
				 showTree(tree->f.b.n1,depth+1);
				 showTree(tree->f.b.n2,depth+1);
	}
}

/**
 * Print each token and string to terminal
 * */
void printSymb(){ 	
	char * s;
    printf("%s ",showSymb(symb));	//print symbol associated with token to screen
    
    //if symbol is an ID or an INT
	if(symb==NAME || symb==NUMBER){
		if(yytext == NULL){
			printf("Error: yytext is null");
		}
		else{
			printf("%s\n",yytext);
		}
	}else{
		printf("\n");
	}
}

/**
 * print the symbol to the terminal
 * get next symbol
 * */
void lex(){  
	//printSymb();
	symb = yylex();
}

/**
 * prints error messages
 * */
void error(char * rule,char * message){  
	printf("%s: found %s\n",rule,showSymb(symb));
	printf("%s: %s\n",rule,message); exit(0); 
}

/*Recursive decent functions for FUNC*/
/**
* PROGRAM Reccursive Decent
* */
NODE * program(){
	extern NODE * functions();

	NODE * p;
	p = newNode(SEMI);

	if(symb==FUNCTION){
		p->f.b.n1 = functions();
	}else{
		printf("%s \n", "exit program");
	}
	return p;
}

/**
* FUNCTIONS Reccursive Decent
* */
NODE * functions(){
	extern NODE * function();
	
	NODE * functionNode;
	NODE * currentFunction;
	currentFunction = function();
	
	functionNode = newNode(SEMI);	//CREATE FUNCTIONS NODE
	if (symb == FUNCTION){
		lex();	//go to next symbol
		if (symb == FUNCTION){
			functionNode->f.b.n1 = currentFunction;	//put current functions as second value
			functionNode->f.b.n2 = functions();	//recurse as there are more functions
		}else {
			functionNode->f.b.n1 = currentFunction;
			functionNode->f.b.n2 = NULL;
		}
	} else{
		functionNode->f.b.n1 = currentFunction;
		functionNode->f.b.n2 = NULL; 
		return functionNode;
		//error(";", "Expected ; after Function Declaration");
	}
	return functionNode;
}


/**
* FUNCTION Reccursive Decent
* */
NODE * function(){
	extern NODE * name();
	extern NODE * arguments();
	extern NODE * argument();
	extern NODE * defs();
	extern NODE * commands();
	
	NODE * funcNode;
	NODE * funcName;
	NODE * funcLPAREN;
	NODE * funcRPAREN;
	NODE * funcRETURN;

	funcNode = newNode(FUNCTION);
	funcName = newNode(NAME);
	funcLPAREN = newNode(LPAREN);
	funcRPAREN = newNode(RPAREN);
	funcRETURN = newNode(RETURN);
		
	if(symb == FUNCTION){
		lex();
		funcNode->f.b.n1 = name();	//assign name node to n1 branches function(
		lex();
		if (symb == LPAREN){	//function arguments
			lex();
			if (symb != RPAREN){	//first argument exists
				funcLPAREN->f.b.n1 = arguments();	//start building function signature funciton (arg
				//lex();
				if (symb != RPAREN){
					error(")", "Expected ) after Function Arguments");
				}
			}
		}else {
			error("(", "Expected ( after Function Name");
		}
	}
	lex();
	if (symb == RETURN){
		lex();
		if (symb == LPAREN){
			lex();
			//create nodes to make return node
			NODE * retLPAREN; NODE * retRPAREN; 
			retLPAREN = newNode(LPAREN); 
			retRPAREN = newNode(RPAREN);

			retLPAREN->f.b.n1 = argument();	//assign args node to RETURN n1 branch 
			lex();
			//assign build return node
			funcRETURN->f.b.n1 = retLPAREN;
			funcRETURN->f.b.n2 = retRPAREN;
			//add return node to function Right paren node
			funcRPAREN->f.b.n1 = funcRETURN;	//)return(arg)
		}
	}
	if (symb == IS){
		NODE * isNode;
		isNode = newNode(IS);
		printf("%s %s\n", "test IS ", showSymb(symb));
		lex();
		isNode->f.b.n1 = defs();	//assign defs node to IS n1 branch

		funcRPAREN->f.b.n2 = isNode; //	)return(arg)is
	}
	/***********IMPORTANT************
	** STILL HAVE BEGIN, END
	*********************************/
	if (symb == TBEGIN){
		printf("%s %s\n", "test TBENGIN ", showSymb(symb));
		NODE * beginNode = newNode(BEGIN);
		//decide on positioning of BEGIN NODE
		//function signature looks like: function NAME(ARGS)
		if (RPAREN->f.b.n1 == RETURN && RPAREN->f.b.n2 == NULL){
			beginNode->f.b.n1 = commands()
		}
		//function signature looks like: function NAME(ARGS) RETURN (ARG)
		else if (RPAREN->f.b.n1.tag == RETURN && RPAREN->f.b.n2 == NULL){

		}
		//function signature looks like: function NAME(ARGS) IS DEFS
		else if (RPAREN->f.b.n1 == NULL && RPAREN->f.b.n2.tag == IS){

		}
		//function signature looks like: function NAME(ARGS) RETURN(ARG) IS DEFS
		else if (RPAREN->f.b.n1.tag == RETURN && RPAREN->f.b.n1.tag == IS){

		}

	}
	/*if(symb == END){

	}*/

	funcLPAREN->f.b.n2 = funcRPAREN;	//(args)return(arg) OR function(args)return(arg)is
	funcNode->f.b.n2 = funcLPAREN;		//function name (args)return(arg) OR function(args)return(arg)is
	return funcNode;

}

/**
* NAME Reccursive Decent
* */
NODE * name(){
	NODE * n;
	if(yytext == NULL){
		printf("Error: yytext is null");
		return n;
	}
	else{
		n = newName(yytext);
	}
	return n;
}

/**
* NAME Reccursive Decent
* */
NODE * num(){
	NODE * n;
	if(yytext == NULL){
		printf("Error: yytext is null");
		return n;
	}
	else{
		n = newNumber(yytext);
	}
	return n;
}

/**
* COMMAND Reccursive Decent
* */
NODE * command(){
	
}


/**
* COMMANDS Reccursive Decent
* */
NODE * commands(){

}


/**
* ARG Reccursive Decent
* */
NODE * argument(){
	NODE * argument;
	argument = newNode(COLON);
	if (symb == NAME){
		argument->f.b.n1 = name();
		lex(); lex(); lex();
	}
	return argument;
}

/**
* ARGS Reccursive Decent
* */
NODE * arguments(){
	NODE * args;
	args = newNode(COMMA);

	args->f.b.n1 = argument();
	if (symb == COMMA){
		lex();
		args->f.b.n2 = arguments();
	}else{return args;}
	return args;
}

/**
* TYPE Reccursive Decent
* */
NODE * Type(){
	printf("%s %s\n", "test enter Type, symb = ", showSymb(symb));

	NODE * dType;

	/*if types are needed for defs*/
	/*if (symb == INTEGER){
		//dType = newNode(INTEGER);
		lex();
	}*/
	if (symb == ARRAYOFSIZE){
		dType = newNode(ARRAYOFSIZE);
		lex();
		if (symb == NUMBER){
			dType->f.b.n1 = num(); /*still to implement*/
			lex();
		}	
	}
	printf("%s %s\n", "test before Type return node, symb = ", showSymb(symb));
	return dType;
}


/**
* DEF Reccursive Decent
* */
NODE * definit(){
	printf("%s %s\n", "test enter DEFINIT, symb = ", showSymb(symb));
	NODE * definition;
	definition = newNode(COLON);
	definition->f.b.n1 = name();
	lex(); lex();
	if (symb == INTEGER){
		lex();
		//definition->f.b.n2 = Type();	//if types are needed for defs
	}else if(symb == ARRAYOFSIZE){
		definition->f.b.n2 = Type();
	}else{
		error("type", "Expected Type after definition declaration");
	}
	return definition;
}

/**
* DEFS Reccursive Decent
* */
NODE * defs(){
	printf("%s %s\n", "test enter DEFS, symb = ", showSymb(symb));
	NODE * d;
	d = newNode(SEMI);

	d->f.b.n1 = definit();lex();
	printf("%s %s\n", "test DEFS def returned, symb = ", showSymb(symb));
	if (symb != TBEGIN){
		d->f.b.n2 = defs();
	}
	return d;
}


int main (int argc, char ** argv){
	if((yyin=fopen(argv[1],"r"))==NULL){  
		  printf("can't open %s\n",argv[1]);
		  exit(0);
	}
	
	/*symb = yylex();
	printSymb();
	
	while(symb != EOF){  
		symb = yylex();  
		printSymb();
	}*/

	symb = yylex();
	showTree(program(),1);
		
	fclose(yyin);
	return 0;
}

