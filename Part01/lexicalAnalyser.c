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
	NODE * beginNode;

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
		lex();
		isNode->f.b.n1 = defs();	//assign defs node to IS n1 branch

		funcRPAREN->f.b.n2 = isNode; //	)return(arg)is
	}
	/***********IMPORTANT************
	** STILL HAVE BEGIN, END
	*********************************/
	if (symb == TBEGIN){
		printf("%s %s\n", "test TBENGIN ", showSymb(symb));
		beginNode = newNode(TBEGIN);
		lex();
		beginNode->f.b.n1 = commands();
		printf("%s %s\n", "before END , symb = ", showSymb(symb));	
	}
	if(symb == END){
		lex();
		NODE * e = newNode(END);
		e->f.b.n1 = newName(yytext);
		beginNode->f.b.n2 = e;	//add END node to second Branch of BEGIN node
		lex();
		printf("%s %s\n", "end END , symb = ", showSymb(symb));	
	}

	//decide on positioning of BEGIN NODE
	//function signature looks like: function NAME(ARGS)
		if ((funcRPAREN->f.b.n1 == NULL) && (funcRPAREN->f.b.n2 == NULL)){ //beginNode->f.b.n1 = commands();
			printf("%s\n", "function NAME(ARGS)");
			funcRPAREN->f.b.n1 = beginNode;

		}else if (funcRPAREN->f.b.n1 == NULL && funcRPAREN->f.b.n2->tag == IS){ //function NAME(ARGS) IS DEFS
			printf("%s\n", "function NAME(ARGS) IS DEFS");
			funcRPAREN->f.b.n2->f.b.n2 = beginNode;

		}else if (funcRPAREN->f.b.n1->tag == RETURN && funcRPAREN->f.b.n2 == NULL){  //function NAME(ARGS) RETURN (ARG)
			printf("%s\n", "function NAME(ARGS) RETURN (ARG)");
			funcRPAREN->f.b.n2 = beginNode;

		}else if (funcRPAREN->f.b.n1->tag == RETURN && funcRPAREN->f.b.n2->tag == IS){ 	//function NAME(ARGS) RETURN(ARG) IS DEFS
			printf("%s\n", "function NAME(ARGS) RETURN(ARG) IS DEFS");
			funcRPAREN->f.b.n2->f.b.n2 = beginNode;

		}else{
			printf("%s\n", "BEGIN IF ELSE FAIL" );
		}

	funcLPAREN->f.b.n2 = funcRPAREN;	//build function node
	funcNode->f.b.n2 = funcLPAREN;		
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
* COMMANDS Reccursive Decent
* */
NODE * commands(){
	printf("%s%s\n", "enter COMMANDS, symb = ", showSymb(symb));
	extern NODE * command();
	NODE * com;
	com = command();
	lex();
	printf("FUUUUUUUUK %s\n", showSymb(symb) );
	if(symb != END){  
		printf("%s\n","YOLOWSWEG" );
		NODE * c1; 
		c1 = com;
		com = newNode(SEMI);
		com->f.b.n1 = c1;
		printf("%s%s\n", "mid YOLOWSWEG, symb = ", showSymb(symb));
		com->f.b.n2 = commands();
		printf("%s%s\n", "after YOLOWSWEG, symb = ", showSymb(symb));
		return com;
	}
	return com;
}

/**
* COMMAND Reccursive Decent
* */
NODE * command(){
	printf("%s%s\n", "enter COMMAND, symb = ", showSymb(symb));
	extern NODE * assign();
	extern NODE * ifComm();
	extern NODE * writeComm();
	extern NODE * whileComm();
	extern NODE * block();

	switch(symb){  
		case NAME: 	return assign();
		/*case IF: 	lex(); return ifComm();
		case WHILE: lex(); return whileComm();
		case TBEGIN: lex(); return block();
		case WRITE: lex(); return writeComm();*/
		default:   
				error("command","BEGIN/IF/INPUT/PRINT/WHILE/identifier expected \n");
	}
}


/**
* ASSIGN Reccursive Decent
* */
NODE * assign(){
	printf("%s%s\n", "enter ASSIGN, symb = ", showSymb(symb));
	extern NODE * expr();

	NODE * a = newNode(ASSIGN);
	if (symb == LSQBRA){	//left of assign is expression
		lex();
		a->f.b.n1 = expr();
		lex();
	}else{	//left of assign is a name
		printf("%s%s\n", "is name ASSIGN, symb = ", showSymb(symb));
		NODE * i = newName(yytext);
		a->f.b.n1 = i;
	}
	lex();
	if (symb != ASSIGN){
		error("assign","name identifier expected \n");
	}
	lex();
	printf("%s%s\n", "after := symb ASSIGN, symb = ", showSymb(symb));
	a->f.b.n2 = expr();
	return a;
}


/**
* EXPRS Reccursive Decent
* */
NODE * exprs(){
	printf("%s%s\n", "enter EXPRS, symb = ", showSymb(symb));
	extern NODE * expr();
	NODE * exs; NODE * ex1;
	ex1 = expr();
	if (symb == COMMA){	//**********FIX ME
		printf("%s%s\n", "FRASER test: ", showSymb(symb));
		exs = newNode(COMMA);
		lex();
		exs->f.b.n1 = ex1;
		exs->f.b.n2 = exprs();
	}else{
		return ex1;
	}
	return exs;
}
/**
* EXPR Reccursive Decent
* */
NODE * expr(){
	extern NODE * name();
	printf("%s%s\n", "enter EXPR, symb = ", showSymb(symb));
	NODE * ex;
	NODE * n;

	n = name();

	if (symb == NUMBER){
		n = newNumber(yytext); 
		lex();
		printf("BALLS %s\n", n->f.id);
		return n;
	}else{lex();}
	
	if (symb == LPAREN){
		printf("%s%s\n", "inside LPAREN if EXPR, symb = ", showSymb(symb));
		ex = newNode(SEMI);
		lex();
		ex->f.b.n1 = n;
		ex->f.b.n2 = exprs();
		lex();	// move past RPAREN
		return ex;
	}
	if (symb == LSQBRA){
		printf("%s%s\n", "inside LSQBRA if EXPR, symb = ", showSymb(symb));
		ex = expr();
		lex(); //move past RSQBRA
		return ex;
	}

	return n;

}

/**
* ARG Reccursive Decent
* */
NODE * argument(){
	extern NODE * name();
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
	extern NODE * argument();

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
	extern NODE * num();
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
	return dType;
}


/**
* DEF Reccursive Decent
* */
NODE * definit(){
	extern NODE * Type();
	extern NODE * name();

	NODE * definition;
	definition = newNode(COLON);

	definition->f.b.n1 = name();	//assign 1st branch to name of definition
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
	NODE * d;
	d = newNode(SEMI);

	d->f.b.n1 = definit();lex();
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

