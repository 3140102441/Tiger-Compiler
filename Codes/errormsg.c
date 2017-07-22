#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "util.h"
#include "errormsg.h"

extern FILE *yyin;

static int lineNum = 1;

static string fileName = "";

int EM_tokPos=0;

bool Exist_Error = FALSE;



typedef struct int_list {
	int i; 
	struct intlist *next;
} *IntList;

static IntList line_Pos = NULL;

static IntList intlist(int i, IntList next)
{
	IntList list = checked_malloc(sizeof *list);
	list->i = i; 
	list->next = next;
	return list;
}

void EM_newline(void)
{
	line_Pos = intlist(EM_tokPos, line_Pos);
	lineNum++;
}


void EM_error(int position, char *message,...)
{
	int linenum = lineNum;
	va_list list;
	IntList line = line_Pos;
 

	Exist_Error = TRUE;
	while (line && line->i >= position) {
		 linenum--;
		 line = line->next;
	}

	if (fileName) 
		fprintf(stderr,"error : file : %s:",fileName);
	if (line) 
		fprintf(stderr, "%d.%d: ", linenum, position - line->i);
	va_start(list, message);
	vfprintf(stderr, message, list);
	va_end(list);
	fprintf(stderr,"\n");
}

void EM_reset(string filename)
{
	fileName = filename;
	lineNum = 1;
	line_Pos = intlist(0, NULL);
	Exist_Error = FALSE; 
	yyin = fopen(filename, "r");
	if (!yyin) {
		EM_error(0,"cannot open the file ,please check!"); 
		exit(1);
	}
}


