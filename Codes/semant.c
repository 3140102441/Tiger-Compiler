#include <stdio.h>
#include "util.h"
#include "errormsg.h"
#include "symbol.h"
#include "absyn.h"
#include "types.h"
#include "temp.h"
#include "tree.h"
#include "frame.h"
#include "translate.h"
#include "env.h"
#include "semant.h"
#include "printtree.h"


struct expty expTy(Tr_exp e, Ty_ty t) {
	struct expty et;
	et.exp = e;
	et.ty  = t;
	return et;
}

static struct expty transVar(Tr_level, Tr_exp, S_table, S_table, A_var);
static struct expty transExp(Tr_level, Tr_exp, S_table, S_table, A_exp);
static Tr_exp transDec(Tr_level, Tr_exp, S_table, S_table, A_dec);
static Ty_ty transTy (S_table, A_ty);
static Ty_tyList makeFormalTyList(S_table, A_fieldList); 
static Ty_ty actual_ty(Ty_ty);
static bool args_match(Tr_level, Tr_exp, S_table, S_table, A_expList, Ty_tyList, A_exp); 
static bool ty_match(Ty_ty, Ty_ty);
static bool efields_match(Tr_level, Tr_exp, S_table, S_table, Ty_ty, A_exp);
static Ty_fieldList makeFieldTys(S_table, A_fieldList); 
static U_boolList makeFormals(A_fieldList); 
extern bool anyErrors; 

F_fragList SEM_transProg(A_exp exp) {
	struct expty et;
	S_table t = E_base_tenv();
	S_table v = E_base_venv();
	et = transExp(Tr_outermost(), NULL, v, t, exp);
	F_fragList fraglist = Tr_getResult();
	return fraglist;
}

static struct expty transVar(Tr_level level, Tr_exp lbreak, S_table venv, S_table tenv, A_var v) {
	if (!v) {
		return expTy(Tr_noExp(), Ty_Void());
	}

	switch (v->kind) {
	// token id
	case A_simpleVar:
	{
		Tr_exp idTrans;
		E_enventry idx;
		idx = S_look(venv, v->u.simple); 
		idTrans = Tr_noExp();

		if (idx && idx->kind == E_varEntry) {
			idTrans = Tr_simpleVar(idx->u.var.access, level);
			return expTy(idTrans, actual_ty(idx->u.var.ty));
		} 

		else {
			EM_error(v->pos, "!!!Undefined var %s", S_name(v->u.simple));
			return expTy(idTrans, Ty_Int());
		}
		break;
	}

	// lvalue DOT id 
	case A_fieldVar:
	{
		struct expty fieldet;
		Tr_exp fieldTrans;

		fieldet = transVar(level, lbreak, venv, tenv, v->u.field.var);
		fieldTrans = Tr_noExp();

		if (fieldet.ty->kind != Ty_record) 
		{
			EM_error(v->pos, "!!!Not a record type");
		} 
		else {
			Ty_fieldList fieldList;
			int k = 0;
			for (fieldList = fieldet.ty->u.record; fieldList; fieldList = fieldList->tail, k++) {
				if (fieldList->head->name == v->u.field.sym) {
					fieldTrans = Tr_fieldVar(fieldet.exp, k);
					return expTy(fieldTrans, actual_ty(fieldList->head->ty));
				}
			}
			EM_error(v->pos, "!!!No such field in record %s", S_name(v->u.field.sym));
		}
		return expTy(fieldTrans, Ty_Int());
		break;
	}

	// id LBRACK exp RBRACK
	case A_subscriptVar: 
	{
		struct expty subset, subset2;
		Tr_exp subsTrans;

		subset  = transVar(level, lbreak, venv, tenv, v->u.subscript.var);
		subsTrans = Tr_noExp();

		if (subset.ty->kind != Ty_array) {
			EM_error(v->pos, "!!!Not a array type");
		} 
		else {
			subset2 = transExp(level, lbreak, venv, tenv, v->u.subscript.exp);

			if (subset2.ty->kind != Ty_int) {
				EM_error(v->pos, "!!!Int needed");
			} 
			else {
				subsTrans = Tr_subscriptVar(subset.exp, subset2.exp);
				return expTy(subsTrans, actual_ty(subset.ty->u.array));
			}

		}
		return expTy(subsTrans, Ty_Int());
	}

	default:
		printf("wrong entry.");
	}
}



static struct expty transExp(Tr_level level, Tr_exp lbreak, S_table v, S_table t, A_exp e){
	if (!e) 
		{ 
			return expTy(Tr_noExp(), Ty_Void()); 
		}
	switch (e->kind) {
	// lvalue
	case A_varExp:
	{
		return transVar(level, lbreak, v, t, e->u.var);
	}
	// lvalue ASSIGN exp
	case A_assignExp: 
	{
		struct expty expf4 = transVar(level, lbreak, v, t, e->u.assign.var);
		struct expty expf5 = transExp(level, lbreak, v, t, e->u.assign.exp);
		if (!ty_match(expf4.ty, expf5.ty)) 
		{
			EM_error(e->pos, "!!!Unmatched assign exp");
		}
		return expTy(Tr_assignExp(expf4.exp, expf5.exp), Ty_Void());
	}
	// while
	case A_whileExp: 
	{
		struct expty expf = transExp(level, lbreak, v, t, e->u.whilee.test);
		if (expf.ty->kind != Ty_int) {
			EM_error(e->pos, "!!!Int required");
		}
		Tr_exp done = Tr_doneExp();
		struct expty body = transExp(level, done, v, t, e->u.whilee.body);
		return expTy(Tr_whileExp(expf.exp, body.exp, done), Ty_Void());
	}

	// break
	case A_breakExp:
	{
		if (!lbreak) return expTy(Tr_noExp(), Ty_Void());
		return expTy(Tr_breakExp(lbreak), Ty_Void());
	}

	// NIL
	case A_nilExp:
	{
		return expTy(Tr_nilExp(), Ty_Nil());
	}
	// string
	case A_stringExp:
	{
		return expTy(Tr_stringExp(e->u.stringg), Ty_String());
	}
	// int
	case A_intExp:
	{
		return expTy(Tr_intExp(e->u.intt), Ty_Int());
	}
	// double
	case A_doubleExp:
	{
		return expTy(Tr_doubleExp(e->u.doublee), Ty_Double());
	}
	// funcall
	case A_callExp: 
	{
		E_enventry callinfo = S_look(v, e->u.call.func); 
		A_expList args = NULL;
		Tr_expList argList = NULL;
		for (args = e->u.call.args; args; args = args->tail) 
		{ 
			struct expty arg = transExp(level, lbreak, v, t, args->head);

			Tr_expList_prepend(arg.exp, &argList);
		}

		Tr_exp trans = Tr_noExp();

		if (callinfo && callinfo->kind == E_funEntry)
		{
			trans = Tr_callExp(callinfo->u.fun.label, callinfo->u.fun.level, level, &argList);
			if (args_match(level, lbreak, v, t, e->u.call.args, callinfo->u.fun.formals, e)) 
			{
				if (callinfo->u.fun.result) 
				{
					return expTy(trans, actual_ty(callinfo->u.fun.result));
				}
			}
		}
		else 
		{
			EM_error(e->pos, "!!!Undefined function %s\n", S_name(e->u.call.func));
		}
		return expTy(trans, Ty_Void());
	}
	// record
	case A_recordExp: 
	{
		Ty_ty recty = actual_ty(S_look(t, e->u.record.typ));
		if (!recty) 
		{
			EM_error(e->pos, "!!!Undefined type %s (debug recordExp)", S_name(e->u.record.typ));
		}
		else 
		{
			if (recty->kind != Ty_record)
			{
				EM_error(e->pos, "%s is not a record type", S_name(e->u.record.typ));
				return expTy(Tr_noExp(), Ty_Record(NULL));
			}
			if (efields_match(level, lbreak, v, t, recty, e)) 
			{
				Tr_expList l = NULL;
				int n = 0;
				A_efieldList el;
				for (el = e->u.record.fields; el; el = el->tail, n++) 
				{
					struct expty val = transExp(level, lbreak, v, t, el->head->exp);
					Tr_expList_prepend(val.exp, &l);
				}
				return expTy(Tr_recordExp(n, l), recty);
			}
		}
		return expTy(Tr_noExp(), Ty_Record(NULL));
	}

	// array
	case A_arrayExp: 
	{
		Ty_ty arrayty = actual_ty(S_look(t, e->u.array.typ));
		if (!arrayty) 
		{
			EM_error(e->pos, "!!!Undeined array type %s", S_name(e->u.array.typ));
			return expTy(Tr_noExp(), Ty_Int());
		}
		if (arrayty->kind != Ty_array) 
		{
			EM_error(e->pos, "%s is not a array type", S_name(e->u.array.typ));
			return expTy(Tr_noExp(), Ty_Int());
		}

		struct expty expf2 = transExp(level, lbreak, v, t, e->u.array.size);
		struct expty expf3 = transExp(level, lbreak, v, t, e->u.array.init);

		if (expf2.ty->kind != Ty_int) 
		{
			EM_error(e->pos, "!!!Array size should be int %s", S_name(e->u.array.typ));
		}
		else if (!ty_match(expf3.ty, arrayty->u.array))
		{
			EM_error(e->pos, "!!!Unmatched array type in %s", S_name(e->u.array.typ));
		}
		else {
			return expTy(Tr_arrayExp(expf2.exp, expf3.exp), arrayty);
		}
		return expTy(Tr_noExp(), Ty_Int());
	}
	//seq
	case A_seqExp: 
	{
		Tr_expList l = NULL;
		A_expList list = e->u.seq;
		struct expty seqone;
		if (!list) 
		{
			return expTy(Tr_noExp(), Ty_Void());
		}
		for (; list; list = list->tail) 
		{
			seqone = transExp(level, lbreak, v, t, list->head);
			Tr_expList_prepend(seqone.exp, &l);
		}
		return expTy(Tr_seqExp(l), seqone.ty);
	}

	// FOR id ASSIGN exp TO exp DO exp
	case A_forExp: 
	{
		EM_error(e->pos, "!!!For error");
		return expTy(Tr_noExp(), Ty_Int());
	}
	// LET decs IN explist END
	case A_letExp: 
	{
		A_decList decs;
		Tr_expList l = NULL;
		S_beginScope(v);
		S_beginScope(t);
		for (decs = e->u.let.decs; decs; decs = decs->tail) 
		{
			Tr_expList_prepend(transDec(level, lbreak, v, t, decs->head), &l);
		}

		struct expty expf = transExp(level, lbreak, v, t, e->u.let.body);
		Tr_expList_prepend(expf.exp, &l);
		S_endScope(v);
		S_endScope(t);

		return expTy(Tr_seqExp(l), expf.ty);
	}
	// OP
	case A_opExp: 
	{
		A_oper oper = e->u.op.oper;
		struct expty left = transExp(level, lbreak, v, t, e->u.op.left);
		struct expty right = transExp(level, lbreak, v, t, e->u.op.right);
		if (0 <= oper && oper < 4) {
			if (left.ty->kind != Ty_int && left.ty->kind != Ty_double){
				EM_error(e->u.op.left->pos, "int or double required(op)");
			}
			else if (right.ty->kind != Ty_int && right.ty->kind != Ty_double) {
				EM_error(e->u.op.right->pos, "int or double required(op)");
			}
			else if (left.ty->kind == Ty_int && right.ty->kind == Ty_int) {
				return expTy(Tr_arithExp(oper, left.exp, right.exp), Ty_Int());
			}
			else {
				return expTy(Tr_arithExp(oper, left.exp, right.exp), Ty_Double());
			}
			return expTy(Tr_noExp(), Ty_Int());
		}
		else if (3 < oper && oper < 10) 
		{
			Tr_exp translation = Tr_noExp();
			if (oper == 4 || oper == 5) 
			{
				switch (left.ty->kind) 
				{
				case Ty_int:
				case Ty_double:
					if (right.ty->kind == Ty_int || right.ty->kind == Ty_double) 
						translation = Tr_eqExp(oper, left.exp, right.exp);
					else 
						{ 
							EM_error(e->u.op.right->pos, "!!!Unexpected type in comparsion"); 
						}
					break;
				case Ty_string:

					if (ty_match(right.ty, left.ty)) 
						translation = Tr_eqStringExp(oper, left.exp, right.exp);
					else 
						{ 
							EM_error(e->u.op.right->pos, "!!!Unexpected type in comparsion"); 
						}
					break;
				case Ty_array:
					if (ty_match(right.ty, left.ty)) translation = Tr_eqRef(oper, left.exp, right.exp);
					else { EM_error(e->u.op.right->pos, "!!!Unexpected type in comparsion"); }
					break;
				case Ty_record:
					if (ty_match(right.ty, left.ty) || right.ty->kind == Ty_nil) 
						translation = Tr_eqRef(oper, left.exp, right.exp);
					else 
					{ 
						EM_error(e->u.op.right->pos, "!!!Unexpected type in comparsion"); 
					}
					break;
				default:
					EM_error(e->u.op.right->pos, "!!!Unexpected expression in comparsion");
				}
				return expTy(translation, Ty_Int());
			}
			else {
				switch (left.ty->kind) 
				{
				case Ty_double:
				case Ty_int:
					if (right.ty->kind == Ty_double || right.ty->kind == Ty_int) 
						translation = Tr_relExp(oper, left.exp, right.exp);
					else 
						{ 
							EM_error(e->u.op.right->pos, "!!!Unexpected type in comparsion"); 
						}
					break;
				case Ty_string:
					if (right.ty->kind == Ty_string) 
						translation = Tr_eqStringExp(oper, left.exp, right.exp);
					else 
						{ 
							EM_error(e->u.op.right->pos, "!!!Unexpected type in comparsion"); 
						}
					break;
				default:
					EM_error(e->u.op.right->pos, "!!!Unexpected type in comparsion");
				}
				return expTy(translation, Ty_Int());
			}
		}
		else {
			printf("wrong entry.");
		}
	}
	// IF exp THEN exp
	case A_ifExp: 
	{
		struct expty expf = transExp(level, lbreak, v, t, e->u.iff.test);
		struct expty expf2 = transExp(level, lbreak, v, t, e->u.iff.then);
		struct expty expf3 = { NULL, NULL };
		if (e->u.iff.elsee) 
		{ 
			expf3 = transExp(level, lbreak, v, t, e->u.iff.elsee);
			if (expf.ty->kind != Ty_int)
			{
				EM_error(e->u.iff.test->pos, "!!!Int required");
			}
			if (!ty_match(expf2.ty, expf3.ty)) 
			{
				EM_error(e->pos, "!!!if-else sentence must return same type");
			}
		}
		return expTy(Tr_ifExp(expf.exp, expf2.exp, expf3.exp), expf2.ty);
	}
	default:
		printf("wrong entry.");
	}
}

static Tr_exp transDec(Tr_level level, Tr_exp lbreak, S_table v, S_table t, A_dec d)
 {
	struct expty expf;
	A_fundec f;
	Ty_ty resTy, namety, isname;
	Ty_tyList formalTys, s;
	A_fieldList l;
	A_nametyList nl;
	A_fundecList fcl;
	E_enventry fun;
	int iscyl, isset;
	Tr_access ac;

	// dec
	switch (d->kind) {
	// tydeclist, tydec
	case A_typeDec:
	{
		for (nl = d->u.type; nl; nl = nl->tail) {
			S_enter(t, nl->head->name, Ty_Name(nl->head->name, NULL));
		} 
		iscyl = TRUE;
		for (nl = d->u.type; nl; nl = nl->tail) 
		{
			resTy = transTy(t, nl->head->ty);
			if (iscyl) 
			{
				if (resTy->kind != Ty_name) 
				{
					iscyl = FALSE;
				}
			}

			if (!nl->tail && resTy->kind != Ty_name && isset) 
			{
				EM_error(d->pos, "!!!Warning: actual type should declare brefore nameTy type");
			}
			namety = S_look(t, nl->head->name);
			namety->u.name.ty = resTy;
		}
		if (iscyl) 
			EM_error(d->pos, "!!!Illegal type cycle: cycle must contain record, array");
		return Tr_noExp();
	}

	// vardec
	case A_varDec:
	{
		expf = transExp(level, lbreak, v, t, d->u.var.init);
		ac = Tr_allocLocal(level, d->u.var.escape);
		if (!d->u.var.typ) 
		{
			if (expf.ty->kind == Ty_nil || expf.ty->kind == Ty_void) 
			{
				
				EM_error(d->pos, "!!!init should not be nil without type in %s", S_name(d->u.var.var));
				S_enter(v, d->u.var.var, E_VarEntry(ac, Ty_Int()));
			}
			else {
				S_enter(v, d->u.var.var, E_VarEntry(ac, expf.ty));
			}
		}
		else {
			resTy = S_look(t, d->u.var.typ);
			if (!resTy) 
			{
				EM_error(d->pos, "!!!Undifined type %s", S_name(d->u.var.typ));
			}
			else 
			{
				if (!ty_match(resTy, expf.ty)) 
				{
					EM_error(d->pos, "!!!Unmatched type in %s", S_name(d->u.var.typ));
					S_enter(v, d->u.var.var, E_VarEntry(ac, resTy));
				}
				else {
					S_enter(v, d->u.var.var, E_VarEntry(ac, resTy));
				}
			}
		}
		return Tr_assignExp(Tr_simpleVar(ac, level), expf.exp);
	}
	// fundeclist fundec
	case A_functionDec:
	{
		for (fcl = d->u.function; fcl; fcl = fcl->tail) 
		{
			if (fcl->head->result) 
			{
				resTy = S_look(t, fcl->head->result);
				if (!resTy) 
				{
					EM_error(fcl->head->pos, "!!!Undefined type for return type");
					resTy = Ty_Void();
				}
			}
			else 
			{
				resTy = Ty_Void();
			}
			
			formalTys = makeFormalTyList(t, fcl->head->params);
			{
				Temp_label funLabel = Temp_newlabel();
				Tr_level l = Tr_newLevel(level, funLabel, makeFormals(fcl->head->params));
				S_enter(v, fcl->head->name, E_FunEntry(l, funLabel, formalTys, resTy));
			}
		}
		
		for (fcl = d->u.function; fcl; fcl = fcl->tail) {
			f = fcl->head;

			E_enventry funEntry = S_look(v, f->name);
			S_beginScope(v);
			formalTys = funEntry->u.fun.formals;

			Tr_accessList acls = Tr_formals(funEntry->u.fun.level);
			for (l = f->params, s = formalTys; l && s && acls; l = l->tail, s = s->tail, acls = acls->tail) {
				S_enter(v, l->head->name, E_VarEntry(acls->head, s->head));
			}

			expf = transExp(funEntry->u.fun.level, lbreak, v, t, f->body);
			fun = S_look(v, f->name);
			if (!ty_match(fun->u.fun.result, expf.ty)) 
			{
				EM_error(f->pos, "!!!Incorrect return type in function '%s'", S_name(f->name));
			}
			Tr_procEntryExit(funEntry->u.fun.level, expf.exp, acls);
			S_endScope(v);
		}
		return Tr_noExp();
	}

	default:
		printf("wrong entry.");
	}
}

static U_boolList makeFormals(A_fieldList fieldList) {
	U_boolList head, tail;
	A_fieldList p;
	head = NULL;
	tail = NULL;
	p = fieldList;
	for (; p; p = p->tail) 
	{
		if (head) 
		{
			tail->tail = U_BoolList(TRUE, NULL);
			tail = tail->tail;
		} 
		else {
			head = U_BoolList(TRUE, NULL);
			tail = head;
		}
	}
	return head;
}

static Ty_ty transTy(S_table tb, A_ty ty) {
	Ty_fieldList fieldTys;

	switch (ty->kind) 
	{

	// id
	case A_nameTy:
	{
		Ty_ty nameTy = NULL;
		nameTy = S_look(tb, ty->u.name);
		if (!nameTy) 
		{
			EM_error(ty->pos, "!!!Undefined type %s", S_name(ty->u.name));
			return Ty_Int();
		}
		return nameTy;
	}
	// LBRACE typefields RBRACE
	case A_recordTy:
	{
		Ty_fieldList fieldList;
		fieldList = makeFieldTys(tb, ty->u.record);
		return Ty_Record(fieldList);
	}
	// ARRAY OF id
	case A_arrayTy:
	{
		Ty_ty arrayTy = NULL;
		arrayTy = S_look(tb, ty->u.array);
		if (!arrayTy) 
		{
			EM_error(ty->pos, "!!!Undefined type %s", S_name(ty->u.array));
		}
		return Ty_Array(arrayTy);
	}

	default:
		printf("wrong entry.");
	}
}


static bool args_match(Tr_level level, Tr_exp lbreak, S_table v, S_table tt, A_expList ell, Ty_tyList fll, A_exp fun) {
	A_expList expList = ell;
	Ty_tyList fl = fll;

	while (expList && fl) 
	{
		struct expty t;
		t = transExp(level, lbreak, v, tt, expList->head);
		if (!ty_match(t.ty, fl->head))
		{
			EM_error(fun->pos, "!!!Unmatched params in function %s\n", S_name(fun->u.call.func));
			return FALSE;
		}
		expList = expList->tail;
		fl = fl->tail;
	}

	if (expList && !fl) 
	{
		EM_error(fun->pos,"!!!Too many parameters in function %s\n", S_name(fun->u.call.func));
		return FALSE;
	} 
	else if (!expList && fl) 
	{
		EM_error(fun->pos, "!!!Less parameters in function %s\n", S_name(fun->u.call.func));
		return FALSE;
	} 
	else {
		return TRUE;
	}
}

static bool ty_match(Ty_ty tt, Ty_ty ee) {
	Ty_ty t, e;
	int tk, ek;
	bool result;

	t = actual_ty(tt);
	e = actual_ty(ee);
	tk = t->kind;
	ek = e->kind;

	result = (((tk == Ty_record || tk == Ty_array) && t == e) ||(tk == Ty_record && ek == Ty_nil) ||
			 (ek == Ty_record && tk == Ty_nil) || (tk != Ty_record && tk != Ty_array && tk == ek));

	return result;
}

static bool efields_match(Tr_level level, Tr_exp lbreak, S_table v, S_table t, Ty_ty tyy, A_exp e) {
	Ty_fieldList ty;
	A_efieldList fl;

	ty = tyy->u.record;
	fl = e->u.record.fields;
	while (ty && fl) 
	{
		struct expty et;
		et = transExp(level, lbreak, v, t, fl->head->exp);
		if (!(ty->head->name == fl->head->name) || !ty_match(ty->head->ty, et.ty))
		{
			EM_error(e->pos, "!!!Unmatched name: type in %s", S_name(e->u.record.typ));
			return FALSE;
		}
		ty = ty->tail;
		fl = fl->tail;
	}

	if (ty && !fl) 
	{
		EM_error(e->pos, "!!!Less fields in %s", S_name(e->u.record.typ));
		return FALSE;
	} 
	else if (!ty && fl) 
	{
		EM_error(e->pos, "!!!Too many field in %s", S_name(e->u.record.typ));
		return FALSE;
	}
	return TRUE;
}


static Ty_fieldList makeFieldTys(S_table t, A_fieldList fs) {
	Ty_fieldList tys = NULL;
	Ty_fieldList head;
	Ty_ty ty;
	Ty_field temp;
	A_fieldList f;

	for (f = fs; f; f = f->tail) 
	{
		ty = S_look(t, f->head->typ);

		if (!ty) {
			EM_error(f->head->pos, "!!!Undefined type %s", S_name(f->head->typ));
		} 
		else 
		{
			temp = Ty_Field(f->head->name, ty);
			if (tys) 
			{
				tys->tail = Ty_FieldList(temp, NULL);
				tys = tys->tail;
			} 
			else 
			{
				tys = Ty_FieldList(temp, NULL);
				head = tys;
			}
		}
	}
	return head;
}

static Ty_tyList makeFormalTyList(S_table t, A_fieldList fieldlist) {
	A_fieldList l = fieldlist;
	Ty_ty ty;
	Ty_tyList tyList = NULL; 
	Ty_tyList head = NULL;

	for (; l; l = l->tail) 
	{
		ty = S_look(t, l->head->typ);
		if(!ty) 
		{
			EM_error(l->head->pos, "!!!Undefined type %s", S_name(l->head->typ));
			ty = Ty_Int();
		}
		if (!tyList) 
		{
			tyList = Ty_TyList(ty, NULL);
			head = tyList;
		} 
		else 
		{
			tyList->tail = Ty_TyList(ty, NULL);
			tyList = tyList->tail;
		}
	}
	return head;
}

static Ty_ty actual_ty(Ty_ty ty){
	if (!ty)
	{
		return ty;
	}
	if (ty->kind == Ty_name) 
	{
		actual_ty(ty->u.name.ty);
	}
	else 
	{
		return ty;
	}
}


