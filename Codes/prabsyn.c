#include <stdio.h>
#include "util.h"
#include "symbol.h" 
#include "absyn.h"  
#include "prabsyn.h" 
#include "string.h"

char result[MAX_LENGTH];

void changestr(string origin);

string  makeparent(string name, string parent){
	string s = malloc(sizeof(char) * 100);
	sprintf(s, "{ \"name\": \"%s\",\"parent\": \"%s\",\"children\":[", name, parent);


	return s;
}

string makeleaf(string name, string parent){
	string s = malloc(sizeof(char) * 100);
	sprintf(s, "{ \"name\": \"%s\",\"parent\": \"%s\"", name, parent);


	return s;

}


void changestr(string origin){
	int i = 0;
	while (*origin){
		if (*origin == '\n') {
			result[i++] = '\\';
			result[i++] = 'n';
			origin++;
		}
		else {
			result[i++] = *origin++;
		}
	}
	result[i++] = '\0';
}


/*static void print_blank(FILE *out, int distance) {
int i;
for (i = 0; i <= distance; i++)
fprintf(out, "   ");
}*/

static char str_oper[][12] = {
	"PLUS", "MINUS", "TIMES", "DIVIDE",
	"EQUAL", "NOTEQUAL", "LESSTHAN", "LESSEQ", "GREAT", "GREATEQ" };


static void print_oper(FILE *out, A_oper op, string parent) {
	string s = makeleaf(str_oper[op], parent);

	fprintf(out, "%s", s);
	fprintf(out, "}\n");
}


static void print_var(FILE *out, A_var var, string parent);
static void print_dec(FILE *out, A_dec dec, string parent);
static void print_ty(FILE *out, A_ty ty, string parent);
static void print_field(FILE *out, A_field field, string parent);
static void print_fieldList(FILE *out, A_fieldList flist, string parent);
static void print_expList(FILE *out, A_expList elist, string parent);
static void print_fundec(FILE *out, A_fundec fdec, string parent);
static void print_fundecList(FILE *out, A_fundecList fdeclist, string parent);
static void print_decList(FILE *out, A_decList dlist, string parent);
static void print_namety(FILE *out, A_namety namety, string parent);
static void print_nametyList(FILE *out, A_nametyList ntylist, string parent);
static void print_efield(FILE *out, A_efield efield, string parent);
static void print_efieldList(FILE *out, A_efieldList eflist, string parent);


static void print_var(FILE *out, A_var var, string parent) {
	string  s;	string name = malloc(sizeof(char) * 20);
	switch (var->kind) {
	case A_simpleVar:

		sprintf(name, "simpleVar(%s)", S_name(var->u.simple));

		s = makeleaf(name, parent);

		fprintf(out, "%s", s);
		fprintf(out, "\n");

		fprintf(out, "}\n");
		break;

	case A_subscriptVar:

		s = makeparent("subscriptVar()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_var(out, var->u.subscript.var, "subscriptVar()");
		fprintf(out, "%s\n", ",");
		print_exp(out, var->u.subscript.exp, "subscriptVar()");
		fprintf(out, "]}\n");
		break;

	case A_fieldVar:

		s = makeparent("fieldVar()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_var(out, var->u.field.var, "fieldVar()");
		fprintf(out, "]}\n");

		//fprintf(out, "%s)", S_name(var->u.field.sym));
		break;
	default:
		assert(0);
	}
}


static void print_dec(FILE *out, A_dec dec, string parent) {
	string  s;	string name = malloc(sizeof(char) * 20);
	switch (dec->kind) {
	case A_varDec:


		sprintf(name, "varDec(%s)", S_name(dec->u.var.var));

		s = makeparent(name, parent);

		fprintf(out, "%s", s);
		fprintf(out, "\n");
		if (dec->u.var.typ) {

			//fprintf(out, "%s,\n", S_name(dec->u.var.typ));
		}
		print_exp(out, dec->u.var.init, "varDec()");
		fprintf(out, "]}\n");

		//fprintf(out, "%s", dec->u.var.escape ? "TRUE)" : "FALSE)");
		break;
	case A_typeDec:
		s = makeparent("typeDec()", parent);

		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_nametyList(out, dec->u.type, "typeDec()");
		fprintf(out, "]}\n");
		break;
	case A_functionDec:
		s = makeparent("functionDec()", parent);

		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_fundecList(out, dec->u.function, "functionDec()");
		fprintf(out, "]}\n");
		break;
	default:
		assert(0);
	}
}

static void print_decList(FILE *out, A_decList dlist, string parent) {
	string s;	string name = malloc(sizeof(char) * 20);
	if (dlist) {
		s = makeparent("decList()", parent);

		fprintf(out, "%s", s);
		fprintf(out, "\n");
		print_dec(out, dlist->head, "decList()");
		fprintf(out, ",\n");
		print_decList(out, dlist->tail, "decList()");
		fprintf(out, "]}\n");
	}
	else {
		s = makeleaf("decList()", parent);

		fprintf(out, "%s", s);
		fprintf(out, "\n");

		fprintf(out, "}\n");
	}

}

void print_exp(FILE *out, A_exp exp, string parent) {
	string  s;	string name = malloc(sizeof(char) * 20);
	switch (exp->kind) {
	case A_letExp:
		s = makeparent("letExp", parent);

		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_decList(out, exp->u.let.decs, "letExp");
		fprintf(out, ",\n");
		print_exp(out, exp->u.let.body, "letExp");
		fprintf(out, "]}\n");

		break;

	case A_varExp:
		s = makeparent("VarExp", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_var(out, exp->u.var, "VarExp");

		fprintf(out, "]}\n");
		break;

	case A_nilExp:

		s = makeleaf("nilExp()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		fprintf(out, "}\n");
		break;

	case A_doubleExp:
		s = makeleaf("doubleExp()", parent);

		fprintf(out, "%s", s);
		fprintf(out, "\n");

		fprintf(out, "}\n");
		break;

	case A_intExp:

		sprintf(name, "intExp(%d)", exp->u.intt);
		s = makeleaf(name, parent);
		fprintf(out, "%s", s);

		fprintf(out, "\n");

		fprintf(out, "}\n");

		break;
	case A_stringExp:
		changestr(exp->u.stringg);

		sprintf(name, "stringExp(%s)", result);
		s = makeleaf(name, parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		fprintf(out, "}\n");
		break;

	case A_seqExp:
		s = makeparent("seqExp", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_expList(out, exp->u.seq, "seqExp");

		fprintf(out, "]}\n");
		break;
	case A_assignExp:
		s = makeparent("assignExp", parent);

		fprintf(out, "%s", s);
		fprintf(out, "\n");
		print_var(out, exp->u.assign.var, "assignExp");
		fprintf(out, ",\n");
		print_exp(out, exp->u.assign.exp, "assignExp");
		fprintf(out, "]}\n");
		break;

	case A_opExp:
		s = makeparent("opExp", parent);

		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_oper(out, exp->u.op.oper, "opExp");
		fprintf(out, ",\n");
		print_exp(out, exp->u.op.left, "opExp");
		fprintf(out, ",\n");
		print_exp(out, exp->u.op.right, "opExp");
		fprintf(out, "]}\n");
		break;

	case A_callExp:

		sprintf(name, "callExp(%s)", S_name(exp->u.call.func));
		s = makeparent(name, parent);

		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_expList(out, exp->u.call.args, "callExp");
		fprintf(out, "]}\n");
		break;

	case A_recordExp:

		sprintf(name, "recordExp(%s)", S_name(exp->u.record.typ));
		s = makeparent(name, parent);

		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_efieldList(out, exp->u.record.fields, "recordExp");
		fprintf(out, "]}\n");
		break;

	case A_arrayExp:


		sprintf(name, "arrayExp(%s)", S_name(exp->u.array.typ));
		s = makeparent(name, parent);

		fprintf(out, "%s", s);

		fprintf(out, "\n");

		print_exp(out, exp->u.array.size, "arrayExp");
		fprintf(out, ",\n");
		print_exp(out, exp->u.array.init, "arrayExp");
		fprintf(out, "]}\n");
		break;

	case A_ifExp:
		s = makeparent("iffExp()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");
		print_exp(out, exp->u.iff.test, "iffExp()");
		fprintf(out, ",\n");
		print_exp(out, exp->u.iff.then, "iffExp()");
		if (exp->u.iff.elsee) {
			fprintf(out, ",\n");
			print_exp(out, exp->u.iff.elsee, "iffExp()");
		}
		fprintf(out, "]}\n");
		break;

	case A_whileExp:
		s = makeparent("whileExp()", parent);

		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_exp(out, exp->u.whilee.test, "whileExp()");
		fprintf(out, ",\n");
		print_exp(out, exp->u.whilee.body, "whileExp()");
		fprintf(out, "]}\n");
		break;


	case A_breakExp:
		s = makeleaf("breakExp()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "}\n");
		break;

	case A_forExp:


		sprintf(name, "forExp(%s)", S_name(exp->u.forr.var));
		s = makeparent(name, parent);

		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_exp(out, exp->u.forr.lo, "forExp()");
		fprintf(out, ",\n");
		print_exp(out, exp->u.forr.hi, "forExp()");
		fprintf(out, ",\n");
		print_exp(out, exp->u.forr.body, "forExp()");
		fprintf(out, "]}\n");

		//fprintf(out, "%s", exp->u.forr.escape ? "TRUE)" : "FALSE)");
		break;
	default:
		assert(0);
	}
}

static void print_expList(FILE *out, A_expList elist, string parent) {
	string  s;	string name = malloc(sizeof(char) * 20);
	if (elist) {
		s = makeparent("expList()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_exp(out, elist->head, "expList()");

		fprintf(out, ",\n");
		print_expList(out, elist->tail, "expList()");
		fprintf(out, "]}\n");
	}
	else {
		s = makeleaf("expList()", parent);

		fprintf(out, "%s", s);
		fprintf(out, "\n");
		fprintf(out, "}\n");
	}

}



static void print_ty(FILE *out, A_ty ty, string parent) {
	string  s;	string name = malloc(sizeof(char) * 20);
	switch (ty->kind) {
	case A_nameTy:

		sprintf(name, "nameTy(%s)", S_name(ty->u.name));
		s = makeleaf(name, parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		fprintf(out, "}\n");
		break;
	case A_arrayTy:

		sprintf(name, "arrayTy(%s)", S_name(ty->u.array));
		s = makeleaf(name, parent);
		fprintf(out, "%s", s);

		fprintf(out, "\n");

		fprintf(out, "}\n");
		break;

	case A_recordTy:
		s = makeparent("recordTy()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_fieldList(out, ty->u.record, "recordTy()");
		fprintf(out, "]}\n");
		break;
	default:
		assert(0);
	}
}

static void print_field(FILE *out, A_field field, string parent) {
	string name = malloc(sizeof(char) * 20);
	sprintf(name, "field(%s)", S_name(field->name));
	string s = makeleaf(name, parent);
	fprintf(out, "%s", s);
	fprintf(out, "\n");
	fprintf(out, "}\n");


	/*fprintf(out, "%s,\n", S_name(field->typ));

	fprintf(out, "%s", field->escape ? "TRUE)" : "FALSE)");*/
}

static void print_fieldList(FILE *out, A_fieldList flist, string parent) {
	if (flist) {
		string s = makeparent("fieldList()", parent);

		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_field(out, flist->head, "fieldList()");
		fprintf(out, ",\n");

		print_fieldList(out, flist->tail, "fieldList()");
		fprintf(out, "]}");
	}
	else{
		string s = makeleaf("fieldList()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");
		fprintf(out, "}\n");
	}
}



static void print_fundec(FILE *out, A_fundec fdec, string parent) {
	string name = malloc(sizeof(char) * 20);
	sprintf(name, "fundec(%s)", S_name(fdec->name));
	string s = makeparent(name, parent);
	fprintf(out, "%s", s);
	fprintf(out, "\n");
	print_fieldList(out, fdec->params, "fundec()");
	fprintf(out, ",\n");
	if (fdec->result) {

		//fprintf(out, "%s,\n", S_name(fdec->result));
	}
	print_exp(out, fdec->body, "fundec()");
	fprintf(out, "]}\n");
}

static void print_fundecList(FILE *out, A_fundecList fdeclist, string parent) {
	if (fdeclist) {
		string s = makeparent("fundecList()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_fundec(out, fdeclist->head, "fundecList()");
		fprintf(out, ",\n");
		print_fundecList(out, fdeclist->tail, "fundecList()");
		fprintf(out, "]}\n");
	}
	else {
		string s = makeleaf("fundecList()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");


		fprintf(out, "}\n");
	}
}


static void print_namety(FILE *out, A_namety namety, string parent) {
	string name = malloc(sizeof(char) * 20);
	sprintf(name, "namety(%s)", S_name(namety->name));
	string s = makeparent(name, parent);
	fprintf(out, "%s", s);
	fprintf(out, "\n");

	print_ty(out, namety->ty, "namety()");
	fprintf(out, "]}\n");
}

static void print_nametyList(FILE *out, A_nametyList ntylist, string parent) {
	if (ntylist) {
		string s = makeparent("nametyList()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_namety(out, ntylist->head, "nametyList()");
		fprintf(out, ",\n");
		print_nametyList(out, ntylist->tail, "nametyList()");
		fprintf(out, "]}\n");
	}
	else
	{
		string s = makeleaf("nametyList()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");
		fprintf(out, "}\n");
	}
}

static void print_efield(FILE *out, A_efield efield, string parent) {
	if (efield) {
		string name = malloc(sizeof(char) * 20);
		sprintf(name, "efield(%s)", S_name(efield->name));
		string s = makeparent(name, parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");

		print_exp(out, efield->exp, "efield()");
		fprintf(out, "]}\n");
	}
	else {
		string s = makeleaf("efield()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");
		fprintf(out, "}\n");
	}
}

static void print_efieldList(FILE *out, A_efieldList eflist, string parent) {
	if (eflist) {
		string s = makeparent("efieldList()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");
		print_efield(out, eflist->head, "efieldList()");
		fprintf(out, ",\n");
		print_efieldList(out, eflist->tail, "efieldList()");
		fprintf(out, "]}\n");
	}
	else {
		string s = makeleaf("efieldList()", parent);
		fprintf(out, "%s", s);
		fprintf(out, "\n");
		fprintf(out, "}\n");
	}
}
