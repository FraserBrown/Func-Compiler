%{
#include "tokens.h"
%}

NUMBER [0-9]
NAME [a-zA-z][A-Za-z0-9]*

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
"["				{return LSQBRA;}
"]" 			{return RSQBRA;}
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
"LessThan" 		{return LT;}
"LessEqual"		{return LE;}
"Equal" 		{return EQ;}
"NotEqual" 		{return NEQ;} 
"GreaterThan" 	{return GT;}
"GreaterEqual" 	{return GEQ;}
"Main"			{return MAIN;}
{NUMBER}+ 		{return NUMBER;}
{NAME}			{return NAME;}
<<EOF>>			{return EOF;}

[ \t\n]+          /* eat up whitespace */
%%
int yywrap() { return EOF; }
