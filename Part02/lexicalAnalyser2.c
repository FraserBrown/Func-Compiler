#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tokens.h"

/**
* Name: Fraser M Brown
* Matric Number: H00155918
* Title: Language Processors Coursework Part 1 Func-Compiler
**/
int symb;
//#define DEBUG

extern int yylex(void);	/* returns the next token */
extern FILE * yyin;		/* the input file given */
extern char * yytext;	/* the last match is stored in yytext string */
extern void printSymb(void);

/**
* CODE GENERATION DEFINITIONS
**/
#define MAXREG 16
char * registers[MAXREG];		//stack for variable ids (holds variables in free registers)
int rp;	//pointer to next free funciton regitster.
char * funcName;
#define E1 16	//number for variable "$t8"
#define E2 17	//number for variable "$t9"
#define A0 18	//number for variable "$a0"
#define A1 19	//number for variable "$a1"
#define A2 20	//number for variable "$a2"
#define A3 21	//number for variable "$a3"


/**
 * NODE DEFINITIONS
 * */
#define NODE struct node
struct branches {NODE * n1; NODE * n2; };
union fields {struct branches b; int value; char * id;};
struct node {int tag; union fields f;};


/*
* Node for Name of functions and variables
*		 i
*		/ 
*	  <id>
*/
NODE * newName(char * i){
	NODE * n;
	char * cur = strdup(i);
	n = (NODE *)malloc(sizeof(NODE));
	n->tag = NAME;
	n->f.id = cur;
	return n;
}

/*
*	Node for numbers
*		 i
*		/ 
*	  <id>
*/
NODE * newNumber(char * i){
	NODE * n;
	char * cur = strdup(i);
	n = (NODE *)malloc(sizeof(NODE));
	n->tag = NUMBER;
	n->f.id = cur;
	return n;
}

/*
*	Main inner nodes of AST
*		  tag
*		/     \
*	 <null>  <null>
*/
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
		case  LT: return "Less";
		case  LE: return "LessEq";
		case  EQ: return "Eq";
		case  NEQ: return "NEq"; 
		case GT: return "GreaterThan";
		case GEQ: return "GreaterEqual";
		case NAME: return "NAME";
		case NUMBER: return "NUMBER";	
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
 * get next symbol
 * */
void lex(){  
	symb = yylex();
}

/**
 * prints error messages
 * */
void error(char * rule,char * message){  
	printf("%s: found %s\n",rule,showSymb(symb));
	printf("%s: %s\n",rule,message); exit(0); 
}

/***Recursive decent functions for FUNC***/

/**
* PROGRAM Reccursive Decent
*		 <;>
*		 /    
*	  <functions> 
**/
NODE * program(){
	extern NODE * functions();

	NODE * p;
	p = newNode(SEMI);

	if(symb==FUNCTION){		//check if function exists in program 
		p->f.b.n1 = functions();	//build functions node, add to program node
	}else{
		error("PROGRAM", "Expected FUNCTION In program");
	}
	return p;
}

/**
* FUNCTIONS Reccursive Decent
*		 <FUNCTIONS>
*		 /    	  \
*	<FUNCTION>  <FUNCTIONS>
* */
NODE * functions(){
	extern NODE * function();
	//get function node
	NODE * fun;
	NODE * funs;
	fun = function();
	//if more than one functions recursivly append to left branch of functions
	if (symb == SEMI){
		lex();
		if (symb == FUNCTION){
			funs = newNode(FUNCTION);
			funs->f.b.n1 = fun;		//store initial function in branch 1
			funs->f.b.n2 = functions();	//get additional functions via recursion
			return funs;
		}
	}
	return fun;
}

/**
* FUNCTION Reccursive Decent
* possible tree if function had RETURN & IS 
*		           <FUNCTION>
*               /             \
*	   <SIGNATURE>            <BEGIN>
*	  /           \          /       \
*   <NAME>       <RETURN> <COMMANDS> <END>
*	/	 \	      /    \
* <NAME> <ARGS> <ARG>  <IS>
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
	//start building signature branch of function
	if (symb == NAME){	
		s = newNode(LPAREN);	//signature node
		n = newNode(COMMA);		//name node
		n->f.b.n1 = newName(yytext);		//name of function
		lex();
		if (symb == LPAREN){
			lex();
			n->f.b.n2 = args();	//get arguments of function
			if (symb != RPAREN){error(")", "Expected ) after Function Arguments");}
			lex();
			s->f.b.n1 = n;	//attach function name to signature node
		}else{error("(", "Expected ( before Function Arguments");}
	}else {error("NAME", "Expected Name for Function");}

	if(symb == RETURN){		//signature has RETURN then IS
		r = newNode(RETURN);	//create return node
		lex();
		if (symb == LPAREN){
			lex();
			r->f.b.n1 = arg();	//append return arg to RETURN node
			if (symb != RPAREN){error("RETURN", "Expected ) after Function return argument");}
			lex();
		}
		//check if it has IS after RETURN
		if (symb == IS){
			lex();
			is = newNode(IS);
			is->f.b.n1 = defs();
			if (symb == TBEGIN){
				r->f.b.n2 = is;	//attach is node to return right branch
			}
		}
		s->f.b.n2 = r;	//attach return node to signature
	}else if (symb == IS){	//signature only has IS e.g. function Main() is
		lex();
		is = newNode(IS);
		is->f.b.n1 = defs();
		if (symb == TBEGIN){
			s->f.b.n2 = is;	//attach return node to signature
		}
	}
	
	//create function node, append signature node to left branch
	fun = newNode(FUNCTION);
	fun->f.b.n1 = s;	

	//body of function
	if(symb == TBEGIN){			//create left hand branch of function node
		beg = newNode(TBEGIN);	//create begin node beg
		lex();
		beg->f.b.n1 = commands();
		if (symb == END){		//build end of function
			e = newNode(END);	//create end node e
			lex();				
			e->f.b.n1 = name();		//append name of function to end node
			beg->f.b.n2 = e;		//append end node to beg node
			lex();
			if (symb != SEMI){
				error("END FUNCTION", "Expected ; after Function End");
			}
		}else{error("END FUNCTION", "Expected ; after Function End");} 
	}else{error("FUNCTION", "Expected BEGIN after Function Signature");}
	
	fun->f.b.n2 = beg;	//append BEGIN node to function
	return fun;
}

/**
* ARGS Reccursive Decent
*	      <ARGS>			    <ARGS>		
*         /    \		OR      /    \
*	  <ARG>  <ARGS>		     <ARG>  <ARG>
**/
NODE * args(){
	extern NODE * arg();
	extern NODE * comma();

	if (symb == RPAREN){
		return NULL;
	}
	NODE * a;
	a = arg();
	
	//multiple arguments
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
*		  <ARG>
*         /     
*	  <NAME>    
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
*	      <DEFS>			   <DEFS>		
*         /    \		OR     /    \
*	  <DEF>  <DEFS>		    <DEF>  <DEF>
* */
NODE * defs(){
	extern NODE * def();
	NODE * d;	//original definition node
	NODE * ds;	//multiple defs node
	d = def();

	//multiple defs
	if (symb == SEMI){
		lex();
		if (symb == NAME){
			ds = newNode(SEMI);
			ds->f.b.n1 = d;			//store initial def in branch 1
			ds->f.b.n2 = defs();	//get additional defs via recursion
			return ds;
		}
		return d; 
	}
	return d;
}


/**
* DEF Reccursive Decent
*	      <DEF>	
*         /    \
*	  <NAME>  <TYPE>		
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
**	      <TYPE>			    <TYPE>		
*         /    		OR         /      
*	  <INTEGER>        <ARRAYOFSIZE> 
*							/
*						<NUMBER>
* */
NODE * type(){
	extern NODE * num();
	extern NODE * name();

	if (symb == INTEGER){
		NODE * t = newNode(INTEGER);
		return t;
	}else if (symb == ARRAYOFSIZE){
		NODE * t = newNode(ARRAYOFSIZE);
		lex();
		t->f.b.n1 = name();
		return t;
	}else{
		error("TYPE", "Given Wrong Type for Def expected either Integer or Array of Size <number>");
	}
}


/**
* COMMANDS Reccursive Decent
*	      <COMMANDS>			    <COMMANDS>		
*         /        \		OR      /      
*	  <COMAND>   <COMMANDS>		<COMMAND>   
* */
NODE * commands(){
	extern NODE * command();
	//get function node
	NODE * c;	//original command node
	NODE * cs;	//multiple commands node
	c = command();
	lex();

	//check if next statement is a command
	if (symb == IF || symb ==  NAME || symb ==  WHILE || symb == READ || symb == WRITE){
			cs = newNode(SEMI);
			cs->f.b.n1 = c;				//store initial command in branch 1
			cs->f.b.n2 = commands();	//get additional commands via recursion
			return cs;
	}
	return c;
}


/**
* COMMAND Reccursive Decent
* returns constructed command nodes: 
* <ASSIGN> OR <IF> OR <WRITE> OR <READ> OR <WHILE>		
* */
NODE * command(){
	extern NODE * assign();
	extern NODE * ifComm();
	extern NODE * writeComm();
	extern NODE * whileComm();
	extern NODE * readComm();

	switch(symb){  
		case NAME: 	return assign();
		case IF: 	return ifComm();
		case WRITE: return writeComm();
		case READ: return readComm();
		case WHILE: return whileComm();
		default:   
				error("command","BEGIN/IF/INPUT/PRINT/WHILE/identifier expected \n");
	}
}

/**
* WHILECOMM Reccursive Decent
**	      <WHLIE>	
*         /      \	
*	<CONDEXPR>  <COIMMANDS>	
**/
NODE * whileComm(){
	extern NODE * condExpr();
	extern NODE * commands();

	NODE * w;	//while node
	NODE * l;	//node for inner loop commands

	w = newNode(WHILE); 
	lex();
	w->f.b.n1 = condExpr();	//get conditional expression append to while node
	if (symb == LOOP){
		lex();
		l = commands();		//append commands to while node
	}
	w->f.b.n2 = l;		//set right of while node to then commands
	
	//check while block is ended correctly 
	if (symb == END){
		lex(); 
		if (symb == LOOP){
			lex();
		}else{error("WHILE","expected LOOP after END\n");}
	}else{error("WHILE","expected END LOOP after while block \n");}

	return w;

}

/**
* WRITECOMM Reccursive Decent
*	      <WRITE>	
*         / 
*	  <INTEGER>
* */
NODE * writeComm(){
	extern NODE * expr();
	lex();
	NODE * w = newNode(WRITE);
	w->f.b.n1 =  expr();
	return w;
}

/**
* READCOMM Reccursive Decent
*	      <READ>
*         / 
*	  <NAME>
* */
NODE * readComm(){
	extern NODE * name();
	lex();
	NODE * r = newNode(READ);
	r->f.b.n1 = name();
	lex();
	return r;
}

/**
* IFCOMM Reccursive Decent
*	       <IF>			                <IF>		
*         /    \		OR          /          \
*	  <BOP>  <COMANDS>		    <BOP>          <ELSE>
*     /   \                     /   \         /      \ 
* <NAME> <EXPRS>            <NAME> <EXPRS> <COMANDS> <COMANDS>
* */
NODE * ifComm(){
	extern NODE * condExpr();
	extern NODE * commands();

	NODE * i;
	NODE * t;

	i = newNode(IF); lex();
	i->f.b.n1 = condExpr();
	if (symb == THEN){
		lex();
		t = commands();
		//check if there is ELSE in IF block
		if (symb == ELSE){
			lex();
			NODE * e = newNode(ELSE);	//construct else node
			e->f.b.n1 = t;
			e->f.b.n2 = commands();

			i->f.b.n2 = e;	//add else node to if commands node
		}else{
			i->f.b.n2 = t;	//else set left of if command to then commands
		}
	}else{error("IF","expected THEN in IF block\n");}

	//check end of if block is correct
	if (symb == END){
		lex(); 
		if (symb == IF){
			lex();
		}else{error("IF","expected closing IF after END of IF block block\n");}
	}else{error("IF","expected END IF to close IF block\n");}

	return i;

}

/**
* CONDEXPR Reccursive Decent
*	       <CONDEXPR>
*         /         \
*	   <BOP>      <EXPRS>
* */
NODE * condExpr(){
	extern NODE * bop();
	extern NODE * exprs();
	//lex();
	NODE * b = bop();
	if (symb == LPAREN){
		NODE * cond = newNode(LPAREN);
		cond->f.b.n1 = b;
		lex();	
		cond->f.b.n2 = exprs();
		if (symb != RPAREN){error("CondEXPR",") expected after expressions \n");}
		lex();
		return cond;
	}else{error("CondEXPR","( expected after binary operator \n");}
	

}

/**
* BOP Reccursive Decent
*	   <BOP>	
*       / 
*	  <OP> 
* */
NODE * bop(){
	NODE * b;
	switch(symb){
		case LT:  lex(); return newNode(LT); 
		case LE:  lex(); return newNode(LE); 
		case EQ:  lex(); return newNode(EQ); 
		case NEQ: lex(); return newNode(NEQ); 
		default: 
			error("Binary Operator","Less/LessEq/Eq/NEq binary operator expected \n");
	}
	lex();
	return b;

}

/**
* ASSIGN Reccursive Decent
**	      <ASSIGN>			    <ASSIGN>		
*         /    	 \	    OR     /        \
*	  <NAME>    <EXPR>     <NAME>  <NAME/NUMBER>
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
		//lex();	//move to right square bracket
		if (symb != RSQBRA){
			printf("%s\n", yytext);

			error("ASSIGN", "Expected ] after assignment expression name[expr]");
		}
		lex();
		a->f.b.n1 = lsq;
		//lex(); lex();
	}else{	//NAME
		a->f.b.n1 = n;	//attach name to left branch of assign
	}
	//second half of assign 
	if (symb == ASSIGN){	//if token is assign symbol ]
		lex();
		if(symb == NAME || symb == NUMBER){
			a->f.b.n2 = expr();
		}
	}else{error("ASSIGN", "Expected := after name in assignment command ");}
	return a;
}

/**
* EXPRS Reccursive Decent
**	      <EXPRS>			    <EXPRS>		
*         /    		 OR         /      \
*	  <EXPR>                <EXPR>   <EXPRS>
* */
NODE * exprs(){
	extern NODE * expr();
	//get function node
	NODE * ex;	//original expr node
	NODE * exs;	//multiple exprs node
	ex = expr();
	if (symb == COMMA){
		lex();
		if (symb == NAME || symb == NUMBER){
			exs = newNode(COMMA);
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
*	      <EXPR>			    <EXPR>		
*         /    		OR         /      \
*	  <NAME>          		<NAME>  <EXPRS>
* */
NODE * expr(){
	extern NODE * exprs();
	extern NODE * name();
	extern NODE * num();

	NODE * na = name();	// get name of expression
	NODE * e;

	lex();
	if(symb == LPAREN){
		e = newNode(LPAREN);
		e->f.b.n1 = na;			//name of left hand sied of method call
		lex();					//move to expression in ()
		e->f.b.n2 = exprs();	//get expression args values
		if (symb != RPAREN){
			error("EXPR", "Expected ) after EXPRS name(exprs)");
		}
		lex();
		return e;
	} else if (symb == LSQBRA){
		e = newNode(LSQBRA);
		e->f.b.n1 = na;
		lex();
		e->f.b.n2 = expr();
		if (symb != RSQBRA){
			error("EXPR", "Expected ] after EXPRS name[expr]");
		}
		lex();
		return e;
	}
	return na;

}

/**
* NAME Reccursive Decent
*	      <NAME>
*         /    	
*	  <YYTEXT>       
* */
NODE * name(){
	NODE * n;
	if(yytext == NULL){
		printf("Error: yytext is null");
		return n;
	}else{
		n = newName(yytext);
	}
	return n;
}

/**
* NUM Reccursive Decent
*	      <NUMBER>
*         /    	
*	  <YYTEXT>  
* */
NODE * num(){
	NODE * n;
	if(yytext == NULL){
		printf("Error: yytext is null");
		return n;
	}else{
		n = newNumber(yytext);
	}
	return n;
}

/**
* Get correct index for function variable
**/
int checkVar(char * id){ 
	int i;
	for(i=0;i<rp;i++){
		if(strcmp(id,registers[i])==0){
			return i;
		}
	}
	return -1;
}

/**
* Builds the register name string
**/
char* concat(char *s1, char *s2){
	char *result = malloc(strlen(s1) + strlen(s2)+1);
	strcpy(result, s1);
	strcat(result, s2);
	return result;
}

/**
* returns the register name given the position in regitsters array
**/
char * regname(int r){
	extern char * concat();

	char rnum[2];
	switch(r){
		case E1: return "$t8";
		case E2: return "$t9";
		case A0: return "$a0";
		case A1: return "$a1";
		case A2: return "$a2";
		case A3: return "$a3";
		default:
				if (r < 8){	//$s0-$s7
					sprintf(rnum, "%d", r);
					return concat("$s",rnum);
				}else{	//$t0-$t7
					sprintf(rnum, "%d", r-8);
					return concat("$t",rnum);
				}
	}
}

void codeerror(NODE * t, char * msg){
	fprintf(stderr, "%s: at tag: %s, id: %s\n", msg, showSymb(t->tag), t->f.id );
	exit(0);
}

/**
* check if variable is already defined, if there is enough space to store them
* stores the variable value in the next free register point in registers array
**/
void codeVar(NODE * t){ 
	//printf("\n enter codeVar node: %s\n",showSymb(t->tag));

	extern void codeerror();
	extern void checkvar();

	if(rp==MAXREG){
		codeerror(NULL,"too many variables");
	}
	if(checkVar((char * )t)!=-1){
		codeerror(t,"declared already");
	}
	registers[rp] = (char * )t;
	rp++;
}

/**
* Traverses the AST tree and produces MIPS code in a .asm file
* perameters: AST Tree t
**/
void codeTree(NODE * t){
	extern void codeTree();
	extern void codeSignature();
	extern void codeCommand();
	extern void codeEnd();

	if(t==NULL){
		return;
	}

	switch(t->tag){
		case SEMI: 		codeTree(t->f.b.n1);	//check right branch for functions
						return;
		case FUNCTION: 	codeTree(t->f.b.n1);	//check for more functions or start of a function signatures
						codeTree(t->f.b.n2); 	//check for more functions or start of begin blocks
						return;
		case LPAREN: 	codeSignature(t->f.b.n1); 	//build function signature code
						//printf("TRAVERSE RETURN/IS BRANCH\n");
						codeSignature(t->f.b.n2);	//build return or is code in signature
						return;
		case TBEGIN:	//printf("TRAVERSE TBEGIN BRANCH\n");
						codeCommand(t->f.b.n1);		//generate code for begin block
						codeEnd(t->f.b.n2);
						return;
		default: 	printf("unknown node in codeTree: %s\n",
					showSymb(t->tag));
					exit(0);
	}
}

/**
* Starts to build code for the signature of the function
**/
void codeSignature(NODE * t){
	#ifdef DEBUG
		printf("\n enter codeSignature node: %s\n",showSymb(t->tag));
	#endif
	extern void codeName();
	extern void codeReturn();
	extern void codeIs();

	if (t == NULL){
		return;
	}

	switch (t->tag){
		case COMMA:	codeName(t->f.b.n1);
					if (t->f.b.n2 != NULL){codeName(t->f.b.n2);}
					return;
		case RETURN: codeReturn(t->f.b.n1);
					 codeReturn(t->f.b.n2);
					 return;
		case IS:	 codeIs(t->f.b.n1);
					 printf("\t .text \n");
					 printf("%s \n", funcName);
					 return;
		default: 	printf("unknown node in codeSignature: %s\n",
					showSymb(t->tag));
					exit(0);
	}
}

/**
*	deals with names of variables / functions and calls to build any arguments if needed for return statements
**/
void codeName(NODE * t){
	extern void codeArgs();

	if (t == NULL){
		return;
	}
	#ifdef DEBUG
		printf("\n enter codeName node: %s\n",showSymb(t->tag));
	#endif

	switch(t->tag){
		case NAME:	if (strcmp(t->f.id,"Main") == 0){
						//printf("\n\t .text\n");
						//printf("main: \n");
						funcName = "main:";
					}
					return;
					
		case COMMA:	//codeArgs(t->f.b.n1);
					//codeArgs(t->f.b.n1);
					return;
		default: 	printf("unknown node in codeName: %s\n",
					showSymb(t->tag));
					exit(0);
	}
}


/**
*	generates code for return section of a function and initialises IS code generation
**/
void codeReturn(NODE * t){
	extern void codeName();
	extern void codeIs();

	if (t == NULL){
		return;
	}

	switch(t->tag){
		case NAME:	codeName(t);
					return;
		case IS:	codeIs(t->f.b.n1);
					return;
		default: 	printf("unknown node in codeReturn: %s\n",
					showSymb(t->tag));
					exit(0);
	}
}

/**
*	generates code for IS section of .fun code AST TREE
**/
void codeIs(NODE * t){
	#ifdef DEBUG
		printf("\n enter codeIs node: %s\n",showSymb(t->tag));
	#endif

	extern void codeDefs();
	extern void codeDef();
	if (t == NULL){
		return;
	}

	switch(t->tag){
		case COLON:	if (t->f.b.n2->tag == ARRAYOFSIZE){printf("\t.data\n%s:",t->f.b.n1->f.id);}
					codeDef(t->f.b.n1);
					codeDef(t->f.b.n2);
					return;
		case SEMI:	codeDefs(t->f.b.n1);
					codeDefs(t->f.b.n2);
					return;
		default: 	printf("unknown node in codeIs: %s\n",
					showSymb(t->tag));
					exit(0);
	}
}


/**
*	traverses tree if multiple definitions were declared after IS section of .fun code AST TREE
**/
void codeDefs(NODE * t){
	#ifdef DEBUG
		printf("\n enter codeDefs() node: %s nodeLeft: %s rightNode: %s\n",showSymb(t->tag), showSymb(t->f.b.n1->tag), showSymb(t->f.b.n2->tag));
	#endif
	extern void codeDefs();
	extern void codeDef();

	if (t == NULL){
		return;
	}

	switch(t->tag){
		case COLON:	if (t->f.b.n2->tag == ARRAYOFSIZE){printf("\t.data\n%s:",t->f.b.n1->f.id);}
					codeDef(t->f.b.n1);
					codeDef(t->f.b.n2);
					return;
		case SEMI:	codeDefs(t->f.b.n1);
					codeDefs(t->f.b.n2);
					return;
		default: 	printf("unknown node in codeDefs: %s\n",
					showSymb(t->tag));
					exit(0);
	}
}

/**
*	traverses tree if a definition was declared after IS section of .fun code AST TREE
**/
void codeDef(NODE * t){
	extern void codeAOSDef();
	#ifdef DEBUG
		printf("\n enter codeDef node: %s\n",showSymb(t->tag));
	#endif
	extern void codeVar();

	if (t == NULL){
		return;
	}

	switch(t->tag){
		case NAME:		codeVar(t->f.b.n1);
						return;
		case INTEGER:	return;
		case ARRAYOFSIZE:	codeAOSDef(t);//array stuff return;
							return;
		default: 	printf("unknown node in codeDef: %s\n",
					showSymb(t->tag));
					exit(0);
	}
}



void codeAOSAssign(NODE * t){
	extern void codeMathFunc();
	extern void popFromSP();

	#ifdef DEBUG
		printf("\n enter codeAOSAssign() node: %s nodeLeft: %s rightNode: %s\n",showSymb(t->tag), showSymb(t->f.b.n1->tag), showSymb(t->f.b.n2->tag));
	#endif

	if(t == NULL){
		return;
	} 

	if (t->f.b.n2->tag == LPAREN){
		codeMathFunc(t->f.b.n2);
		int accessNumber = atoi(t->f.b.n1->f.b.n2->f.id);	//get access number for array from AST

		printf("\tli $t8,%i \n", accessNumber);	//element to access from array
		printf("\tli $t9,4 \n");		//word size loaded into temp register
		printf("\tmul $t8, $t8, $t9 \n");	//muliply access point by word size to get correct point in memory
		popFromSP("$s0");
		return;
	}
	//access Array of Size at that value
	char * nameofArr = t->f.b.n1->f.b.n1->f.id;	//get name of array from AST
	int accessNumber = atoi(t->f.b.n1->f.b.n2->f.id);	//get access number for array from AST

	printf("\tli $t8,%i \n", accessNumber);	//element to access from array
	printf("\tli $t9,4 \n");		//word size loaded into temp register
	printf("\tmul $t8, $t8, $t9 \n");	//muliply access point by word size to get correct point in memory
	printf("\tli $t9,%s \n", t->f.b.n2->f.id);

	//store Right assign value in array
	printf("\tsw $t9, %s($t8)\n", nameofArr);

	return;
}


/**
*	Defines .data segment in mips code when an array is defined 
**/
void codeAOSDef(NODE * t){
	#ifdef DEBUG
		printf("\n enter codeAOSDef node: %s nodeLeft: %s ID: %s\n",showSymb(t->tag), showSymb(t->f.b.n1->tag), t->f.b.n1->f.id);
	#endif
	if (t == NULL){
		return;
	}
	int arrsize = atoi(t->f.b.n1->f.id) * 4;	//times array size ginven in AST by a word size
	printf("\t .space %i\n", arrsize);
	return;

}

/**
*	traverses tree if a TBEGIN therefore traverses commands was in .fun code AST TREE
**/
void codeCommand(NODE * t){
	#ifdef DEBUG
	printf("\n enter codeCommand node: %s nodeLeft: %s nodeRight: %s\n",showSymb(t->tag), showSymb(t->f.b.n1->tag), showSymb(t->f.b.n2->tag));
	#endif

	extern void codeVar();
	extern void codeAssign();
	extern void codeCommands();
	extern void codeWrite();
	extern void codeWhile();


	if (t == NULL){
		return;
	}
	switch(t->tag){
		case SEMI:		codeCommands(t->f.b.n1);
						codeCommand(t->f.b.n2);
						return;
		case ASSIGN:	codeAssign(t);
						return;
		case INTEGER:	return;
		case WRITE:		codeWrite(t->f.b.n1);
						return;
		case WHILE:		codeWhile(t);
						return;
		default: 	printf("unknown node in codeCommand tag: %s id: %s\n",showSymb(t->tag), t->f.id);
					exit(0);
	}
}

/**
*	traverses tree if a multple commands was in .fun code AST TREE
**/
void codeCommands(NODE * t){
	#ifdef DEBUG
		char*nr;
		if(t->f.b.n2 != NULL){
			nr = showSymb(t->f.b.n2->tag);
		}else {
			nr = "NULL";
		}
		printf("\n enter codeCommandS node: %s nodeLeft: %s nodeRight: %s\n",showSymb(t->tag), showSymb(t->f.b.n1->tag), nr);
	#endif
	extern void codeVar();
	extern void codeIf();
	extern void codeAssign();
	extern void codeWrite();
	extern void codeWhile();

	if (t == NULL){
		return;
	}

	switch(t->tag){
		case IF:		codeIf(t);
						printf("END: \n"); //print end label for ifstatement
						return;
		case ASSIGN:	codeAssign(t);	//get left branch assign statement
						return;
		case INTEGER:	return;
		case WRITE:		codeWrite(t->f.b.n1);
						return;
		case WHILE:		codeWhile(t);
						return;
		default: 	printf("unknown node in codeCommands tag: %s id: %s\n",showSymb(t->tag), t->f.id);
					exit(0);
	}
}


/**
*	
**/
void codeWhile(NODE * t){
	#ifdef DEBUG
		printf("\n enter codeWhile node: %s nodeLeft: %s nodeRight: %s\n",showSymb(t->tag), showSymb(t->f.b.n1->tag), showSymb(t->f.b.n2->tag));
	#endif

	extern void codeBop();
	extern void codeAssign();

	if (t == NULL){
		return;
	}
	switch(t->tag){
		case WHILE: 	printf("START:\n");
						codeWhile(t->f.b.n1);
						codeCommand(t->f.b.n2);

						printf("\tj START\n");
						printf("END:\n");
						return;
		case LPAREN:	codeBop(t);	//left node is name of bop right is expressions
						return;
		case ASSIGN:	codeAssign(t);
						return;

	}
}


/**
*	Gets predefined variable and sets it to tval
**/
void codeMove(NODE * t, char * tval){
	#ifdef DEBUG
		printf("\n enter codeMove() node: %s id: %s\n",showSymb(t->tag), t->f.id);
	#endif

	extern int check_string_isDigit();
	extern void codeMathFunc();


	char * val;
	int cv;

	cv = check_string_isDigit(t->f.id);
	if (cv == 0){
		if (t->tag == LPAREN){
			codeMathFunc(t);
			return;
		}else{
			//is variables
			int reg = checkVar(t->f.id);
			if (reg == -1){
				codeerror(t,"not declared");
			}
			val = regname(reg);
			printf("\tmove %s,%s\n", val,tval);	//print MIPS command to terminal
			return;
		}
	}

}

/**
*	
**/
void codeIf(NODE * t){
	#ifdef DEBUG
		//printf("\n enter codeif node: %s\n",showSymb(t->tag)); 
	#endif

	extern void codeBop();
	extern void codeAssign();
	if (t == NULL){
		return;
	}
	switch(t->tag){
		case IF: 	codeIf(t->f.b.n1);
					codeIf(t->f.b.n2);
					return;
		case LPAREN:	codeBop(t);	//left node is name of bop right is expressions
						return;
		case ASSIGN:	codeAssign(t);
						return;

	}
}


/**
*	
**/
void codeBop(NODE * t){
	#ifdef DEBUG
		printf("\n enter codeBop node: %s\n",showSymb(t->tag)); 
	#endif
	extern void codeLess();

	if (t == NULL){
		return;
	}

	//extract name of binary operator
	char * bopName;
	switch(t->f.b.n1->tag){
		case LT:	codeLess(t->f.b.n2);	//pass arguments to less machine code generator
					return;
		default:	printf("unknown bop type in codeBop: %s id: %s\n",showSymb(t->f.b.n1->tag));
					exit(0);
	}

}


/**
* generates code for the built in function Less(x,y)
**/
void codeLess(NODE * t){
	#ifdef DEBUG
		printf("\n enter codeLess node: %s\n",showSymb(t->tag));
	#endif

	extern int checkVar();
	extern void commaCheck();
	int reg;
	int cv;


	if (t == NULL){
		return;
	}

	switch(t->tag){
		case LPAREN:	//todo: more binary operators therfore call codeBop
						codeBop(t);
						return;
		case COMMA:		//todo: get left value
						commaCheck("$t8", t->f.b.n1);
						//printf("\tli $t8,%s\n", LVal);	//print MIPS command to terminal

						//todo: get right value
						commaCheck("$t9", t->f.b.n2);
						//printf("\tli $t9,%s\n", RVal);	//print MIPS command to terminal

						//todo: generate less than machine code
						printf("\tbge $t8,$t9, END\n");	//print MIPS command to terminal
						//printf("\tli %s,%s\n", LVal,RVal);	//print MIPS command to terminal
						return;
	}

}

void pushToSP(char * reg){
	printf ("\taddi $sp,$sp,-4\n");
	printf("\tsw %s,0($sp) \n", reg); // move register value to stack
}

void popFromSP(char * reg){
	printf ("\tlw %s,0($sp)\n", reg);
	printf("\taddi $sp,$sp,4 \n");
}

/**
*	given one side of a bop command it will return wheather it is a number of a pre defined variable
**/
void commaCheck(char * tval, NODE * t){
	#ifdef DEBUG
		printf("\n enter commaCheck() node: %s id: %s\n",showSymb(t->tag), t->f.id);
	#endif

	extern int check_string_isDigit();
	extern void codeMathFunc();


	char * val;
	int cv;

	if (t->tag == LSQBRA){
		char * nameofArr = t->f.b.n1->f.id;	//get name of array from AST
		int accessNumber = atoi(t->f.b.n2->f.id);	//get access number for array from AST

		//load section of the array
		printf("\tli $t8,%i \n", accessNumber);	//element to access from array
		printf("\tli $t9,4 \n");		//word size loaded into temp register
		printf("\tmul $t8, $t8, $t9 \n");	//muliply access point by word size to get correct point in memory
		printf("\tlw $t9, %s($t8)\n", nameofArr);

		//push contents of array to stack 
		pushToSP("$t9");
		return;
	}

	cv = check_string_isDigit(t->f.id);
	if (cv == 0){
		if (t->tag == LPAREN){
			codeMathFunc(t);
			return;
		}else{
			//is variables
			int reg = checkVar(t->f.id);
			if (reg == -1){
				codeerror(t,"not declared");
			}
			val = regname(reg);
			printf("\tmove %s,%s\n", tval, val);	//print MIPS command to terminal
			return;
		}
	}else{ //is digit 
		val = t->f.id;
		printf("\tli %s,%s\n", tval, val);	//print MIPS command to terminal
		return;
	}
}

/**
* Generate MIPS code for write function
**/
void codeWrite(NODE * t){
	#ifdef DEBUG
		printf("\n enter codeWrite() node: %s \n",showSymb(t->tag));
	#endif
	extern void codeExp();
	extern void codeMove();

	if (t->tag == LSQBRA){
		char * nameofArr = t->f.b.n1->f.id;	//get name of array from AST
		int accessNumber = atoi(t->f.b.n2->f.id);	//get access number for array from AST

		//load section of the array
		printf("\tli $t8,%i \n", accessNumber);	//element to access from array
		printf("\tli $t9,4 \n");		//word size loaded into temp register
		printf("\tmul $t8, $t8, $t9 \n");	//muliply access point by word size to get correct point in memory
		printf("\tlw $t9, %s($t8)\n", nameofArr);
		
		//write value
		printf("\tli $v0, 1\n");
		printf("\tmove $a0, $t9\n");
		printf("\tsyscall\n");
		return;
	}else{
		printf("\tli $v0, 1\n");
		codeExp(A0,t);
		printf("\tsyscall\n");
		return;
	}
	
}

/**
*	traverses tree further if ASSIGN node was in .fun code AST TREE 
*	peramters:   :=				 :=				 :=
*				/  \	 OR		/  \	 OR		/  \
*			<NAME> <NUM>	<NAME> <NAME>   <NAME> <EXPR>
**/
void codeAssign(NODE * t){
	#ifdef DEBUG
		printf("\n enter codeAssign node: %s nodeLeft: %s ID: %s nodeRight: %s\n",showSymb(t->tag), showSymb(t->f.b.n1->tag), t->f.b.n1->f.id, showSymb(t->f.b.n2->tag));
	#endif

	extern int checkVar();
	extern void codeerror();
	extern void codeExp();
	extern void codeMathFunc();
	extern void codeAOSAssign();

	int reg;
	
	if (t->f.b.n1->tag == LSQBRA){
		codeAOSAssign(t);
		return;
	}
	else if (t->f.b.n2->tag == LPAREN){
			codeMathFunc(t->f.b.n2);
			return; 
	}else{//??????????????????
		reg = checkVar(t->f.b.n1->f.id);
		if(reg == -1){
			codeerror(t->f.b.n1,"not declared");
		}
		codeExp(reg,t->f.b.n2); 
		return;
	}
}


/**
*	decend built in expression functions in the form
*		<LPAREN>
*		 /   \
*	 <NAME>  <COMMA>
*	 		  /    \
*	   <NUM/NAME> <NUM/NAME>
**/
void codeMathFunc(NODE * t){
	#ifdef DEBUG
		printf("\n enter codeMathFunc node: %s nodeLeft: %s ID: %s nodeRight: %s\n",showSymb(t->tag), showSymb(t->f.b.n1->tag), t->f.b.n1->f.id, showSymb(t->f.b.n2->tag));
	#endif

	extern void codeerror();
	extern void codePlus();
	extern void codeMinus();
	extern void codeTimes();
	extern void codeDivide();

	if (t == NULL){
		return;
	}

	//depending on math built in function name decend tree and create different MIPS code
	if(strcmp(t->f.b.n1->f.id, "Plus") == 0){
		codePlus(t->f.b.n2);			//generate addtion MIPS CODE from arguments
		return;
	}else if (strcmp(t->f.b.n1->f.id, "Minus") == 0){
		codeMinus(t->f.b.n2);			//generate subtraction MIPS CODE from arguments
		return;
	}else if(strcmp(t->f.b.n1->f.id, "Times") == 0){
		codeTimes(t->f.b.n2);			//generate mutiply MIPS CODE from arguments
		return;
	}else if(strcmp(t->f.b.n1->f.id, "Divide") == 0){
		codeDivide(t->f.b.n2);			//generate divide MIPS CODE from arguments
		return;
	}else{
		codeerror(t,"math expression not recognised");
		exit(0);
	}

}

/**
*	generate addition MIPS code from expressions;
*			  <COMMA>
*	 	     /       \
* 	  <NUM/NAME>   <NUM/NAME>
**/
void codePlus(NODE * t){
	extern void codeMove();
	#ifdef DEBUG
		printf("\n enter codePlus node: %s\n",showSymb(t->tag)); 
	#endif
	extern void commaCheck();

	commaCheck("$t8", t->f.b.n1);
	commaCheck("$t9", t->f.b.n2);
	if (t->f.b.n1->tag == LSQBRA){
		//pop answer from stack
		popFromSP("$t8");
		printf("\tadd $t8, $t8, $t9\n");
		pushToSP("$t8");
		return;
	}
	printf("\tadd $t8, $t8, $t9\n");
	codeMove(t->f.b.n1, "$t8");
	return;
}

/**
*	generate Minus MIPS code from expressions;
*			  <COMMA>
*	 	     /       \
* 	  <NUM/NAME>   <NUM/NAME>
**/
void codeMinus(NODE * t){
	extern void codeMove();

	#ifdef DEBUG
		printf("\n enter codeMinus node: %s\n",showSymb(t->tag)); 
	#endif

	commaCheck("$t8", t->f.b.n1);
	commaCheck("$t9", t->f.b.n2);
	if (t->f.b.n1->tag == LSQBRA){
		//pop answer from stack
		popFromSP("$t8");
		printf("\tadd $t8, $t8, $t9\n");
		printf("\tmove $t9, $t8\n");
		pushToSP("$t9");	//push answer to calculation to stack

		return;
	}
	printf("\tsub $t8, $t8, $t9\n");
	codeMove(t->f.b.n1, "$t8");
	return;
}

/**
*	generate Times MIPS code from expressions;
*			  <COMMA>
*	 	     /       \
* 	  <NUM/NAME>   <NUM/NAME>
**/
void codeTimes(NODE * t){
	extern void codeMove();

	#ifdef DEBUG
		printf("\n enter codeTimes node: %s\n",showSymb(t->tag)); 
	#endif


	commaCheck("$t8", t->f.b.n1);
	commaCheck("$t9", t->f.b.n2);
	if (t->f.b.n1->tag == LSQBRA){
		//pop answer from stack
		popFromSP("$t8");
		printf("\tadd $t8, $t8, $t9\n");
		printf("\tmove $t9, $t8\n");
		pushToSP("t9");	//push answer to calculation to stack

		return;
	}
	printf("\tmul $t8, $t8, $t9\n");
	codeMove(t->f.b.n1, "$t8");
	return;

}

/**
*	generate Divide MIPS code from expressions;
*			  <COMMA>
*	 	     /       \
* 	  <NUM/NAME>   <NUM/NAME>
**/
void codeDivide(NODE * t){
	#ifdef DEBUG
		printf("\n enter codeDivide node: %s\n",showSymb(t->tag)); 
	#endif

	commaCheck("$t8", t->f.b.n1);
	commaCheck("$t9", t->f.b.n2);
	if (t->f.b.n1->tag == LSQBRA){
		//pop answer from stack
		popFromSP("$t8");
		printf("\tadd $t8, $t8, $t9\n");
		printf("\tmove $t9, $t8\n");
		pushToSP("t9");	//push answer to calculation to stack

		return;
	}
	printf("\tdiv $t8, $t8, $t9\n");
	codeMove(t->f.b.n1, "$t8");
	return;
}

/**
*	
**/
void codeExp(int RD,NODE * e){
	#ifdef DEBUG
		printf("\n enter codeExp node: %s\n",showSymb(e->tag));
	#endif
	extern char * regname();
	extern int check_string_isDigit();
	extern void codeerror();
	
	int reg;

	if (check_string_isDigit(e->f.id)){		//right value of assign is a number
		printf("\tli %s,%s\n", regname(RD),e->f.id);	//print MIPS command to terminal
		return;

	}else{	//right value is a predefined variable
		reg = checkVar(e->f.id);	//check variable hasnt been already declared
		if(reg == -1){
			codeerror(e,"not declared");
		}
		printf("\tmove %s,%s\n", regname(RD),regname(reg));	//print MIPS command to terminal
		return;
	}
}

void codeEnd(NODE * t){
	if (strcmp(t->f.b.n1->f.id,"Main") == 0){
		return; //end of program
	}else{
		//jump back to $ra
	}
}

/**
*	used for variable type checking
**/
int check_string_isDigit(char* s) {
	#ifdef DEBUG
		printf("enter check_string_isDigit with s: %s\n",s );
  	#endif
  	int string_len = strlen(s);

	  for(int i = 0; i < string_len; ++i) {
	    //character is not a digit therfore is string
	    if(!isdigit(s[i])){
	      	return 0;
	    }
	  }
	  return 1;	//character is digit(s)
}


/**
* Main Method
**/
int main (int argc, char ** argv){
	//open file
	if((yyin=fopen(argv[1],"r"))==NULL){  
		  printf("can't open %s\n",argv[1]);
		  exit(0);
	}

	rp=0;

	symb = yylex();			//read in first symbol
	NODE * ast_tree = program();
	//showTree(ast_tree,1);	//build AST, the print tree to terminal

	codeTree(ast_tree);
		
	fclose(yyin);	//close file
	return 0;
}
