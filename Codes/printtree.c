#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "printtree.h"



static char rel_operator[][20] = {
	"EQ", "NE", "LT", "GT", "LE", "GE", "ULT", "ULE", "UGT", "UGE" };

static char bin_operator[][20] = {
	"PLUS", "MINUS", "TIMES", "DIVIDE",
	"AND", "OR", "LSHIFT", "RSHIFT", "ARSHIFT", "XOR" };


static void print_node(FILE *out, T_exp exp, string parent);



static void print_stm(FILE *out, T_stm stm, string parent)
{
	string s;
	string name = malloc(sizeof(char) * 20);
	switch (stm->kind) {
	case T_EXP:
		s = makeparent("EXP()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_node(out, stm->u.EXP, "EXP()");
		fprintf(out, "]}\n");
		break;
	case T_SEQ:
		s = makeparent("SEQ()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_stm(out, stm->u.SEQ.left, "SEQ()");
		fprintf(out, ",\n");
		print_stm(out, stm->u.SEQ.right, "SEQ()");
		fprintf(out, "]}\n");
		break;
	case T_LABEL:
		sprintf(name, "LABEL(%s)", S_name(stm->u.LABEL));
		s = makeleaf(name, parent);
		fprintf(out, "%s", s);
		fprintf(out, "}\n");
		break;

	case T_MOVE:
		s = makeparent("MOVE()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_node(out, stm->u.MOVE.dst, "MOVE()");
		fprintf(out, ",\n");
		print_node(out, stm->u.MOVE.src, "MOVE()");
		fprintf(out, "]}\n");
		break;
	case T_JUMP:
		s = makeparent("JUMP()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_node(out, stm->u.JUMP.exp, "JUMP()");
		fprintf(out, "]}\n");
		break;
	case T_CJUMP:
		sprintf(name, "CJUMP(%s)", rel_operator[stm->u.CJUMP.op]);
		s = makeparent(name, parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_node(out, stm->u.CJUMP.left, name);
		fprintf(out, ",\n");
		print_node(out, stm->u.CJUMP.right, name);
		//fprintf(out, ",\n");

		/*if (stm->u.CJUMP.true) {
		sprintf(name, "%s", S_name(stm->u.CJUMP.true));
		s = makeleaf(name, parent);
		fprintf(out, "%s", s);
		fprintf(out, "}\n");
		}
		else
		{
		s = makeleaf("NULL,", parent);
		fprintf(out, "%s", s);
		fprintf(out, "}\n");
		}
		fprintf(out, ",\n");
		if (stm->u.CJUMP.false) {
		sprintf(name, "%s", S_name(stm->u.CJUMP.false));
		s = makeleaf(name, parent);
		fprintf(out, "%s", s);
		fprintf(out, "}\n");
		}
		else
		{
		s = makeleaf("NULL,", parent);
		fprintf(out, "%s", s);
		fprintf(out, "}\n");
		}*/
		fprintf(out, "]}\n");
		break;

	}
}

static void print_node(FILE *out, T_exp exp, string parent)
{
	string  s;
	string name = malloc(sizeof(char) * 20);
	switch (exp->kind) {
	case T_NAME:
		sprintf(name, "NAME %s", S_name(exp->u.NAME));
		s = makeleaf(name, parent);
		fprintf(out, "%s", s);
		fprintf(out, "}\n");

		break;
	case T_CONST:
		sprintf(name, "CONST %d", exp->u.CONST);
		s = makeleaf(name, parent);
		fprintf(out, "%s", s);
		fprintf(out, "}\n");

		break;
	case T_MEM:
		s = makeparent("MEM()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_node(out, exp->u.MEM, "MEM()");
		fprintf(out, "]}\n");
		break;
	case T_TEMP:
		sprintf(name, "TEMP t%s", Temp_look(Temp_name(), exp->u.TEMP));
		s = makeleaf(name, parent);
		fprintf(out, "%s", s);
		fprintf(out, "}\n");

		break;
	case T_BINOP:
		sprintf(name, "BINOP (%s)", bin_operator[exp->u.BINOP.op]);
		s = makeparent(name, parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_node(out, exp->u.BINOP.left, name);
		fprintf(out, ",\n");
		print_node(out, exp->u.BINOP.right, name);
		fprintf(out, "]}\n");
		break;
	case T_ESEQ:
		s = makeparent("ESEQ()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_stm(out, exp->u.ESEQ.stm, "ESEQ()");
		fprintf(out, ",\n");
		print_node(out, exp->u.ESEQ.exp, "ESEQ()");
		fprintf(out, "]}\n");
		break;
	case T_CALL:{
		T_expList explist = exp->u.CALL.args;

		s = makeparent("CALL()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_node(out, exp->u.CALL.fun, "CALL()");

		for (; explist; explist = explist->tail) {
			/*fprintf(out, ",\n");
			print_node(out, explist->head, parent);*/
		}
		fprintf(out, "]}\n");

		break;
	}
	}
}


void printExp(T_exp e, FILE * out, string parent) {
	print_node(out, e, parent);
}
void printStm(T_stm s, FILE * out, string parent) {
	print_stm(out, s, parent);
}


/*void printStmList(FILE *out, T_stmList stmList, string parent)
{
for (; stmList; stmList = stmList->tail) {
print_stm(out, stmList->head, parent);
fprintf(out, ",\n");
}
}*/


