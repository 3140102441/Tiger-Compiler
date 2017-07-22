#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "errormsg.h"
#include "parse.h"
#include "prabsyn.h"
#include "types.h"
#include "temp.h"
#include "tree.h"
#include "frame.h"
#include "translate.h"
#include "semant.h"


extern int yyparse(void);
extern A_exp absyn_root;




A_exp parse(string filename) 
{
	EM_reset(filename);
    if (yyparse() == 0) 
		return absyn_root;
	else 
		printf("!!!Syntax error\n");
		return NULL;
}


F_fragList newAFragment()
{
	F_fragList flist = 0;
	return 	flist;
}


int main(int argc, char **argv) {
    if (argc != 2){
		fprintf(stderr, "pleace input a.out and filename like this: a.out filename\n");
		exit(1);
	}
	A_exp tempexp = parse(argv[1]);
	FILE * f1 = fopen("absyn.json", "w");
	FILE * f2 = fopen("IR,.json", "w");

	if (tempexp) {
		print_exp(f1, tempexp, "null");
		F_fragList flist = SEM_transProg(tempexp);
		print_frag(flist, f2);
	}
	fclose(f1);
	fclose(f2);
	return 0;
}
