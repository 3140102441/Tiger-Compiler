%{
#include <string.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "y.tab.h"
#include "errormsg.h"


int Char_Pos = 1;

void record(void)
{
	EM_tokPos=Char_Pos;
    Char_Pos+=yyleng;
}


int yywrap(void)
{
	Char_Pos=1;
    return 1;
}


static int comment_cnt = 0;             
static int left;
static char * str_ptr;
static char str[MAX_LENGTH]; 
static char ch; 


void string_start() {
	left = MAX_LENGTH - 1;
	str_ptr = str;
}

#define	OVER_MEM_ERR printf("%s (max_length: %d)", "usage: The memory is full for string!", MAX_LENGTH);\
					 exit(1)

void char_to_string(int ch) {
	if (!left) {
		OVER_MEM_ERR;
	}
	*str_ptr++ = ch;
	left--;
}

void string_end() {
	if (!left) {
		OVER_MEM_ERR;
	}
	*str_ptr++ = '\0';
}

void str_to_string(char * s) {
	int t = strlen(s);
	if (left < t) {
		OVER_MEM_ERR;
	}
	while((*str_ptr++ = *s++, *s));
	left -= t;
}



%}

%x comment
%x string
%s nocomment
id [A-Za-z][_A-Za-z0-9]*
digits [0-9]+
double [0-9]+\.[0-9]+
blank [ \t]+
 
%%
array	 {record(); return ARRAY;}
break    {record(); return BREAK;}
do       {record(); return DO;}
end      {record(); return END;}
else     {record(); return ELSE;}
function {record(); return FUNCTION;}
for      {record(); return FOR;}
if		 {record(); return IF;}
in	     {record(); return IN;}
let      {record(); return LET;}
of       {record(); return OF;}
nil      {record(); return NIL;}
then     {record(); return THEN;}
to		 {record(); return TO;}
type	 {record(); return TYPE;}
var		 {record(); return VAR;}
while    {record(); return WHILE;}
"+"      {record(); return PLUS;}
"-"		 {record(); return MINUS;}
"*"	     {record(); return TIMES;}
"/"	     {record(); return DIVIDE;}
"&"      {record(); return AND;}
"|"      {record(); return OR;}
"="	     {record(); return EQ;}
"<>"	 {record(); return NEQ;}
"<="     {record(); return LE;}
"<"      {record(); return LT;}
">="	 {record(); return GE;}
">"	     {record(); return GT;}
"("      {record(); return LPAREN;}
")"	     {record(); return RPAREN;}
"{"      {record(); return LBRACE;}
"}"      {record(); return RBRACE;}
"["      {record(); return LBRACK;}
"]"      {record(); return RBRACK;}
"."	     {record(); return DOT;}
","	     {record(); return COMMA;}
":="     {record(); return ASSIGN;}
":"      {record(); return COLON;}
";"      {record(); return SEMICOLON;}
{blank}	 {record(); continue;}
\n	     {record(); EM_newline(); continue;}

{id}	 {record(); yylval.sval = yytext; return ID;}	
{digits} {record(); yylval.ival=atoi(yytext); return INT;}
{double} {record(); yylval.dval=atof(yytext); return DOUBLE;}

"/*"                 {record(); comment_cnt++; BEGIN comment;}
<comment>"/*"        {record(); comment_cnt++; BEGIN comment;}
<comment>"*/"        {record(); comment_cnt--; if (!comment_cnt) BEGIN nocomment;}
<comment>\n			 {record(); EM_newline();}	
<comment>(.)         {record(); continue;}


\"      {record(); string_start(); BEGIN string;}
<string>{
\\n				{record(); char_to_string(0x0A);}
\\t				{record(); char_to_string(0x09);}
\\				{record(); char_to_string(0x5c);}
"\\\""			{record(); char_to_string(0x22);}
		
\\[0-9]{3}		{record(); char_to_string(atoi(yytext));}
\"				{record(); string_end();  yylval.sval = strdup(str); BEGIN (0); return STRING;}
\n				{record(); }
{blank}	        {record(); str_to_string(yytext);}	
[^\\" \t\n]+    {record(); str_to_string(yytext);}
}

.	     {record(); EM_error(EM_tokPos,"The token is illegal!!");}
%%
