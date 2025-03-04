#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokens.h"

/**
* Name: Fraser M Brown
* Matric Number: H00155918
* Title: Language Processors Coursework Part 1 Func-Compiler
**/
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
	n->tag = NAME;
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
*	  <DEF>  <DEFS>		    <DEF>  <NAME>
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
**	      <TYPE>			    <DEFS>		
*         /    		OR         /      \
*	  <INTEGER>        <ARRAYOFSIZE>  <NUMBER>
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
		t->f.b.n1 = num();
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
	NODE * w = expr();
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
	NODE * w = name();
	lex();
	return w;
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
		lex();	//move to right square bracket
		if (symb != RSQBRA){
			error("ASSIGN", "Expected ] after assignment expression name[expr]");
		}
		a->f.b.n1 = lsq;
		lex();lex();
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
* Main Method
**/
int main (int argc, char ** argv){
	//open file
	if((yyin=fopen(argv[1],"r"))==NULL){  
		  printf("can't open %s\n",argv[1]);
		  exit(0);
	}

	symb = yylex();			//read in first symbol
	showTree(program(),1);	//build AST, the print tree to terminal
		
	fclose(yyin);	//close file
	return 0;
}
