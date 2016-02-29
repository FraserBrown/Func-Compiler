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
		case SIGNATURE: return "SIGNATURE";
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
		case FUNCNAME: return "FUNCNAME";
		case EXPR:	return "EXPRESSION";	
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
	printf("%s%s\n", "enter program(), symb = ", showSymb(symb));
	extern NODE * functions();

	NODE * p;
	p = newNode(SEMI);

	if(symb==FUNCTION){
		p->f.b.n1 = functions();
	}else{
		printf("%s \n", "exit program");
	}
	printf("%s%s\n", "exit program(), symb = ", showSymb(symb));
	return p;
}

/**
* FUNCTIONS Reccursive Decent
* */
NODE * functions(){
	extern NODE * function();
	//get function node
	NODE * fun;
	NODE * funs;
	fun = function();

	if (symb == SEMI){
		lex();
		if (symb == FUNCTION){
			funs = newNode(FUNCTION);
			funs->f.b.n1 = fun;		//store initial function in branch 1
			funs->f.b.n2 = functions();	//get additional functions via recursion
			return funs;
		}
		//return fun; 
	}
	return fun;
}

/**
* FUNCTION Reccursive Decent
* */
NODE * function(){
	extern NODE * name();
	extern NODE * args();
	extern NODE * arg();
	extern NODE * defs();
	extern NODE * commands();

	NODE * s;	//signature node
	NODE * n; 	//node for name of function
	NODE * r;	//return node if needed
	NODE * is; 	//is node if needed
	NODE * fun; //function node
	NODE * beg;	//begin node
	NODE * e; 	//end node

	lex();	//move to name of function
	if (symb == NAME){	//start building signature branch of function
		s = newNode(SIGNATURE);
		
		n = newNode(FUNCNAME);
		n->f.b.n1 = newName(yytext);
		lex();
		if (symb == LPAREN){
			lex();
			n->f.b.n2 = args();	//get arguments
			if (symb != RPAREN){
				error(")", "Expected ) after Function Arguments");
			}
			lex();
			s->f.b.n1 = n;	//attach function name to signature node
		}else{error("(", "Expected ( before Function Arguments");}
	}else {error("NAME", "Expected Name for Function");}

	if(symb == RETURN){
		r = newNode(RETURN);
		lex();
		if (symb == LPAREN){
			lex();
			r->f.b.n1 = arg();
			if (symb != RPAREN){
				error("RETURN", "Expected ) after Function return argument");
			}
			lex();
		}
		if (symb == IS){
			lex();
			is = newNode(IS);
			is->f.b.n1 = defs();
			if (symb == TBEGIN){
				r->f.b.n2 = is;	//attach is node to return right branch
			}
		}
		s->f.b.n2 = r;	//attach return node to signature
	}
	
	fun = newNode(FUNCTION);
	fun->f.b.n1 = s;	//attach signature to function node

	if(symb == TBEGIN){
		beg = newNode(TBEGIN);//create begin node beg
		lex();	
		beg->f.b.n1 = commands();	//attach commands to beg node
		if (symb == END){
			e = newNode(END);	//create end node e
			lex();	//move to name
			e->f.b.n1 = name();
			beg->f.b.n2 = e;		//attach e to beg node
			lex();
			if (symb != SEMI){
				error("END", "Expected ; after Function End");
			}
		}else{error("END", "Expected END after Commands");}
	}else{error("FUNCTION", "Expected BEGIN after Function Signature");}
	
	fun->f.b.n2 = beg;	
	return fun;
}

/**
* ARGS Reccursive Decent
* */
NODE * args(){
	extern NODE * arg();
	extern NODE * comma();

	NODE * a;
	a = arg();

	if (symb == COMMA){
		lex();
		NODE * c = newNode(COMMA);
		c->f.b.n1 = a;
		c->f.b.n2 = args();

		return c;
	}
	return a;
}

/**
* ARG seperated args Reccursive Decent
* */
NODE * arg(){
	extern NODE  * name();

	NODE * n = name();	//get name of argument
	lex();
	if (symb != COLON){
		error("ARG", "expected : after argument name");
	}
	lex();
	if (symb != INTEGER){
		error("ARG", "Expected Integer type for argument");
	}
	lex();
	return n;
}


/**
* DEFS Reccursive Decent
* */
NODE * defs(){
	extern NODE * def();
	//get function node
	NODE * d;	//original definition node
	NODE * ds;	//multiple defs node
	d = def();
	if (symb == SEMI){
		lex();
		if (symb == NAME){
			ds = newNode(SEMI);
			ds->f.b.n1 = d;		//store initial function in branch 1
			ds->f.b.n2 = defs();	//get additional functions via recursion
			return ds;
		}
		return d; 
	}
	return d;
}


/**
* DEF Reccursive Decent
* */
NODE * def(){
	extern NODE * type();
	extern NODE * name();

	NODE * n;	//node for def name
	NODE * t;	//node for type 
	NODE * d = newNode(COLON); 	//def node

	n = name();
	lex();	//move to colon
	if (symb != COLON){
		error("DEF", "Expected : after def name");
	}
	lex(); //move to type
	t = type();
	lex();	//move to semicolon

	d->f.b.n1 = n;
	d->f.b.n2 = t;
	return d;
}

/**
* TYPE Reccursive Decent
* */
NODE * type(){
	extern NODE * num();
	extern NODE * name();

		//type node

	if (symb == INTEGER){
		NODE * t = newNode(INTEGER);
		return t;
	}else if (symb == ARRAYOFSIZE){
		NODE * t = newNode(INTEGER);
		lex();
		t->f.b.n1 = num();
		return t;
	}else{
		error("TYPE", "Given Wrong Type for Def expected either Integer or Array of Size <number>");
	}
}


/**
* COMMANDS Reccursive Decent
* */
NODE * commands(){
	extern NODE * command();
	//get function node
	NODE * c;	//original command node
	NODE * cs;	//multiple commands node
	c = command();
	if (symb == SEMI){
		lex();
		if (symb == NAME){
			cs = newNode(SEMI);
			cs->f.b.n1 = c;		//store initial function in branch 1
			cs->f.b.n2 = commands();	//get additional functions via recursion
			return cs;
		}
		return c; 
	}
	return c;
}


/**
* COMMAND Reccursive Decent
* */
NODE * command(){
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
	extern NODE * name();
	extern NODE * expr();

	NODE * a = newNode(ASSIGN);
	NODE * n = name();

	lex();
	if (symb == LSQBRA){
		NODE * lsq = newNode(LSQBRA);
		lsq->f.b.n1 = n;	//assing name to left sq bracket
		lex();
		lsq->f.b.n2 = expr();
		lex();	//move to right square bracket
		if (symb != RSQBRA){
			error("ASSIGN", "Expected ] after assignment expression name[expr]");
		}
		a->f.b.n1 = lsq;
		lex();lex();
	}else{
		a->f.b.n1 = n;	//attach name to left branch of assign
	}
	//second half of assign 
	if (symb == ASSIGN){	//if token is assign symbol 
		lex();
		if(symb == NAME){
			a->f.b.n2 = expr();
			return a;
		}
	}

	return a;
}


/**
* EXPRS Reccursive Decent
* */
NODE * exprs(){
	extern NODE * expr();
	//get function node
	NODE * ex;	//original expr node
	NODE * exs;	//multiple exprs node
	ex = expr();
	if (symb == COMMA){
		lex();
		if (symb == NAME || NUMBER){
			exs = newNode(LPAREN);
			exs->f.b.n1 = ex;		//store initial expr in branch 1
			exs->f.b.n2 = exprs();	//get additional exprs via recursion
			return exs;
		}
		return ex; 
	}
	return ex;
}
/**
* EXPR Reccursive Decent
* */
NODE * expr(){
	extern NODE * exprs();
	extern NODE * name();
	extern NODE * num();

	NODE * na = name();
	NODE * e;

	if (symb == NAME){
		e = newNode(COMMA);
		e->f.b.n1 = na;
	}
	if (symb == NUMBER){
		NODE * nu = num();

		/*e->f.b.n1 = nu;*/
		return nu;
	}
	lex();
	if(symb == LPAREN){
		e = newNode(EXPR);
		e->f.b.n1 = na;
		lex();
		e->f.b.n2 = exprs();
		lex();
		if (symb != RPAREN){
		}
		lex();
		return e;
	}
	if (symb == LSQBRA){
		e = newNode(LSQBRA);
		e->f.b.n1 = name();
		lex();
		e->f.b.n2 = expr();
		lex();
		if (symb != RSQBRA){
			error("EXPR", "Expected ] after EXPRS name[expr]");
		}
		return e;
	}
	return e;

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
* NUM Reccursive Decent
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
