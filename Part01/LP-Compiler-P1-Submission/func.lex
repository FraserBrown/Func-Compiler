%{
#include "tokens.h"
%}

NUMBER [0-9]
NAME [a-zA-Z][A-Za-z0-9]*

%%
"function"		{return FUNCTION;}
"returns"		{return RETURN;}
"is"			{return IS;}
"begin"			{return TBEGIN;}
"end"			{return END;}
"Integer"		{return INTEGER;}
"Array of Size"	{return ARRAYOFSIZE;}
"read"			{return READ;}
"write" 		{return WRITE;}
":="			{return ASSIGN;}
":" 			{return COLON;}
";"				{return SEMI;}
"," 			{return COMMA;}
"if" 			{return IF;}
"then" 			{return THEN;}
"else" 			{return ELSE;}
"while" 		{return WHILE;}
"loop" 			{return LOOP;}
"(" 			{return LPAREN;}
")" 			{return RPAREN;}
"["				{return LSQBRA;}
"]" 			{return RSQBRA;}
"Less" 			{return LT;}
"LessEq"		{return LE;}
"Eq" 			{return EQ;}
"NEq" 			{return NEQ;} 
{NUMBER}+ 		{return NUMBER;}
{NAME}			{return NAME;}
<<EOF>>			{return EOF;}

[ \t\n]+          /* eat up whitespace */
%%
int yywrap() { return EOF; }
