#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "types.h"
#include "temp.h"
#include "tree.h"
#include "frame.h"
#include "translate.h"
#include "printtree.h"


static F_fragList fragmentList = NULL;
static F_fragList stringFragmentList = NULL;
static Temp_temp nil_Temp = NULL;


struct Tr_level_ {
	F_frame frame;
	Tr_accessList formals;
	Tr_level parent;
	Temp_label name;
};

static Tr_level outer_level = NULL;

struct Tr_access_ {
	F_access access;
	Tr_level level;
};

struct Cx {
	patchList trues;
	patchList falses;
	T_stm stm;
};

struct Tr_exp_ {
	enum { Tr_ex, Tr_nx, Tr_cx } kind;
	union {
		T_exp ex;    
		T_stm nx;    
		struct Cx cx;
	} u;
};

struct Tr_expList_ {
	Tr_exp head;
	Tr_expList tail;
};

struct patchList_ {
	Temp_label * head; 
	patchList tail;
};

static patchList PatchList(Temp_label *, patchList);
static void doPatch(patchList, Temp_label);
static patchList joinPatch(patchList, patchList);
static Tr_exp Tr_StaticLink(Tr_level, Tr_level);

static Tr_access Tr_Access(Tr_level, F_access);
static Tr_accessList makeFormalAccessList(Tr_level);


static Tr_exp Tr_Ex(T_exp);
static Tr_exp Tr_Nx(T_stm);
static Tr_exp Tr_Cx(patchList, patchList, T_stm);


static T_exp unEx(Tr_exp);
static T_stm unNx(Tr_exp);
static struct Cx unCx(Tr_exp);



static Tr_exp Tr_Ex(T_exp e) {
	
	Tr_exp tr_exp = checked_malloc(sizeof(*tr_exp));
	tr_exp->kind = Tr_ex;
	tr_exp->u.ex = e;
	return tr_exp;
}

static Tr_exp Tr_Nx(T_stm s) {
	
	Tr_exp tr_exp = checked_malloc(sizeof(*tr_exp));
	tr_exp->kind = Tr_nx;
	tr_exp->u.nx = s;
	return tr_exp;
}

static Tr_exp Tr_Cx(patchList t, patchList f, T_stm s) {
	Tr_exp tr_exp = checked_malloc(sizeof(*tr_exp));
	tr_exp->kind = Tr_cx;
	tr_exp->u.cx.trues = t;
	tr_exp->u.cx.falses = f;
	tr_exp->u.cx.stm = s;
	return tr_exp;
}

static T_exp unEx(Tr_exp exp) {
	
	switch (exp->kind) {
	case Tr_ex:
		return exp->u.ex;
	case Tr_nx:
		return T_Eseq(exp->u.nx, T_Const(0));
	case Tr_cx: {
		Temp_temp reg = Temp_newtemp();
		Temp_label truetemp = Temp_newlabel(), falsetemp = Temp_newlabel();
		doPatch(exp->u.cx.trues, truetemp);
		doPatch(exp->u.cx.falses, falsetemp);
		return T_Eseq(T_Move(T_Temp(reg), T_Const(1)),
				      T_Eseq(exp->u.cx.stm,
						     T_Eseq(T_Label(falsetemp),
								    T_Eseq(T_Move(T_Temp(reg), T_Const(0)),
										   T_Eseq(T_Label(truetemp), T_Temp(reg))))));
		}
	}
	assert(0 && "The expression has only 3 condition");
}

static T_stm unNx(Tr_exp exp) {
	
	switch (exp->kind) {
    case Tr_ex:
		return T_Exp(exp->u.ex);
	case Tr_nx:
		return exp->u.nx;
	case Tr_cx: {
		Temp_temp reg = Temp_newtemp(); 
		Temp_label truelabel = Temp_newlabel(), falselabel = Temp_newlabel(); 
		doPatch(exp->u.cx.trues, truelabel);
		doPatch(exp->u.cx.falses, falselabel);
		return T_Exp(T_Eseq(T_Move(T_Temp(reg), T_Const(1)),
				            T_Eseq(exp->u.cx.stm,
								T_Eseq(T_Label(falselabel),
									T_Eseq(T_Move(T_Temp(reg), T_Const(0)),T_Eseq(T_Label(truelabel), T_Temp(reg)))))));

		}
	}
	assert(0);
}

static struct Cx unCx(Tr_exp exp){
	switch (exp->kind) {
	case Tr_cx:
		return exp->u.cx;
	case Tr_ex: {
		struct Cx exp_cx;
	
		exp_cx.stm = T_Cjump(T_eq, exp->u.ex, T_Const(1), NULL, NULL);
		exp_cx.trues = PatchList(&(exp_cx.stm->u.CJUMP.true), NULL);
		exp_cx.falses = PatchList(&(exp_cx.stm->u.CJUMP.false), NULL);
		return exp_cx;
	}
	case Tr_nx:
		assert(0); 
	}
	assert(0);
}


Tr_expList Tr_ExpList(Tr_exp head, Tr_expList tail) {
	Tr_expList list = checked_malloc(sizeof(*list));
	list->head = head;
	list->tail = tail;
	return list;
}

void Tr_expList_prepend(Tr_exp head, Tr_expList * tail) {
	Tr_expList newlist = Tr_ExpList(head, NULL);
	newlist->tail = *tail;
	*tail = newlist;
}

Tr_exp Tr_simpleVar(Tr_access access, Tr_level level) {
	T_exp address = T_Temp(F_FP()); 
	while (level != access->level->parent) { 
		F_access s = F_formals(level->frame)->head;
		address = F_Exp(s, address);
		level = level->parent;
	}
	return Tr_Ex(F_Exp(access->access, address)); 
}

Tr_exp Tr_fieldVar(Tr_exp base_addr, int offsets) {
	
	return Tr_Ex(T_Mem(T_Binop(T_plus, unEx(base_addr), T_Const(offsets * F_WORD_SIZE))));
}

Tr_exp Tr_subscriptVar(Tr_exp base_addr, Tr_exp index) {

	return Tr_Ex(T_Mem(T_Binop(T_plus,unEx(base_addr),T_Binop(T_mul, unEx(index), T_Const(F_WORD_SIZE)))));
}


Tr_exp Tr_stringExp(string str) { 
	Temp_label stringlabel = Temp_newlabel();
	F_frag fragment = F_StringFrag(stringlabel, str);
	stringFragmentList = F_FragList(fragment, stringFragmentList);
	return Tr_Ex(T_Name(stringlabel));
}

Tr_exp Tr_intExp(int i) {
	return Tr_Ex(T_Const(i));
}

Tr_exp Tr_doubleExp(float f) {
	return Tr_Ex(T_Double(f));
}


Tr_exp Tr_noExp() {
	return Tr_Ex(T_Const(0));
}

Tr_exp Tr_nilExp() {
	if (!nil_Temp) {
		nil_Temp = Temp_newtemp(); 
		T_stm allocate = T_Move(T_Temp(nil_Temp),F_externalCall(String("initRecord"), T_ExpList(T_Const(0), NULL)));
		return Tr_Ex(T_Eseq(allocate, T_Temp(nil_Temp)));
	}
	return Tr_Ex(T_Temp(nil_Temp));
}

Tr_exp Tr_recordExp(int num, Tr_expList list) {
	Temp_temp reg = Temp_newtemp();
	
	T_stm allocate = T_Move(T_Temp(reg),F_externalCall(String("initRecord"), T_ExpList(T_Const(num * F_WORD_SIZE), NULL)));

	int i = num - 1;
	T_stm seq = T_Move(T_Mem(T_Binop(T_plus, T_Temp(reg), T_Const(i-- * F_WORD_SIZE))), unEx(list->head));

	
	for (list = list->tail; list; list = list->tail, i--) {
		seq = T_Seq(T_Move(T_Mem(T_Binop(T_plus, T_Temp(reg), T_Const(i * F_WORD_SIZE))), 
						   unEx(list->head)),seq);
	}

	return Tr_Ex(T_Eseq(T_Seq(allocate, seq), T_Temp(reg)));
}

Tr_exp Tr_arrayExp(Tr_exp arr_size, Tr_exp exp_init) {
	return Tr_Ex(F_externalCall(String("initArray"), T_ExpList(unEx(arr_size), T_ExpList(unEx(exp_init), NULL))));
}

Tr_exp Tr_seqExp(Tr_expList list) {
	T_exp resultlist = unEx(list->head); 
	for (list = list->tail; list; list = list->tail) {
		resultlist = T_Eseq(T_Exp(unEx(list->head)), resultlist);
	}
	return Tr_Ex(resultlist);
}

Tr_exp Tr_assignExp(Tr_exp level_val, Tr_exp exp) {
	return Tr_Nx(T_Move(unEx(level_val), unEx(exp)));
}



Tr_exp Tr_doneExp() { 
	return Tr_Ex(T_Name(Temp_newlabel())); 
} 

Tr_exp Tr_ifExp(Tr_exp exp_test, Tr_exp exp_then, Tr_exp exp_elsee) {

	Tr_exp result = NULL;
	Temp_label truelabel = Temp_newlabel(), falselabel = Temp_newlabel(), joinlabel = Temp_newlabel();
	struct Cx condition = unCx(exp_test);
	doPatch(condition.trues, truelabel);
	doPatch(condition.falses, falselabel);

	if (!exp_elsee) {
		if (exp_then->kind == Tr_nx) {
			result = Tr_Nx(T_Seq(condition.stm, T_Seq(T_Label(truelabel), T_Seq(exp_then->u.nx, T_Label(falselabel)))));
		}
		else if (exp_then->kind == Tr_cx) {
			result = Tr_Nx(T_Seq(condition.stm, T_Seq(T_Label(truelabel), T_Seq(exp_then->u.cx.stm, T_Label(falselabel)))));
		}
		else {
			result = Tr_Nx(T_Seq(condition.stm, T_Seq(T_Label(truelabel), T_Seq(T_Exp(unEx(exp_then)), T_Label(falselabel)))));
		}
	}
	else {
		Temp_temp reg = Temp_newtemp();
		T_stm stm_Jump = T_Jump(T_Name(joinlabel), Temp_LabelList(joinlabel, NULL));
		T_stm stm_then;
		if (exp_then->kind == Tr_ex)
			stm_then = T_Exp(exp_then->u.ex);
		else
			stm_then = (exp_then->kind == Tr_nx) ? exp_then->u.nx : exp_then->u.cx.stm;
		T_stm stm_else;
		if (exp_elsee->kind == Tr_ex)
			stm_else = T_Exp(exp_elsee->u.ex);
		else
			stm_else = (exp_elsee->kind == Tr_nx) ? exp_elsee->u.nx : exp_elsee->u.cx.stm;
		result = Tr_Nx(T_Seq(condition.stm,
			T_Seq(T_Label(truelabel),
			T_Seq(stm_then, T_Seq(stm_Jump,
			T_Seq(T_Label(falselabel),
			T_Seq(stm_else, T_Seq(stm_Jump, T_Label(joinlabel)))))))));
	}
	return result;
}

Tr_exp Tr_whileExp(Tr_exp exp_test, Tr_exp exp_loop, Tr_exp exp_done) {
	Temp_label test = Temp_newlabel(), body = Temp_newlabel();
	return Tr_Ex(T_Eseq(T_Jump(T_Name(test), Temp_LabelList(test, NULL)), 
				        T_Eseq(T_Label(body),
							T_Eseq(unNx(exp_loop),T_Eseq(T_Label(test),
										T_Eseq(T_Cjump(T_eq, unEx(exp_test), T_Const(0), unEx(exp_done)->u.NAME, body),
											T_Eseq(T_Label(unEx(exp_done)->u.NAME), T_Const(0))))))));
}

Tr_exp Tr_breakExp(Tr_exp exp) { 
	return Tr_Nx(T_Jump(T_Name(unEx(exp)->u.NAME), Temp_LabelList(unEx(exp)->u.NAME, NULL))); 
}

Tr_exp Tr_arithExp(A_oper op, Tr_exp exp_left, Tr_exp exp_right) { 
	T_binOp binop;
	switch(op) {
	case A_timesOp:  binop = T_mul; break;
	case A_divideOp: binop = T_div; break;
	case A_plusOp  : binop = T_plus; break;
	case A_minusOp:  binop = T_minus; break;
	
	default: assert(0);
	}
	return Tr_Ex(T_Binop(binop, unEx(exp_left), unEx(exp_right)));
}

Tr_exp Tr_eqExp(A_oper op, Tr_exp exp_left, Tr_exp exp_right) {
	
    T_relOp relop;
	if (op == A_eqOp) 
		relop = T_eq; 
	else 
		relop = T_ne;
	T_stm condition = T_Cjump(relop, unEx(exp_left), unEx(exp_right), NULL, NULL);
	patchList trues = PatchList(&condition->u.CJUMP.true, NULL);
	patchList falses = PatchList(&condition->u.CJUMP.false, NULL);
	return Tr_Cx(trues, falses, condition);
}

Tr_exp Tr_eqStringExp(A_oper op, Tr_exp exp_left, Tr_exp exp_right) {
	
	T_exp result = F_externalCall(String("stringEqual"), T_ExpList(unEx(exp_left), T_ExpList(unEx(exp_right), NULL)));
	if (op == A_eqOp) 
		return Tr_Ex(result);
	else if (op == A_neqOp){
		T_exp exp = (result->kind == T_CONST && result->u.CONST != 0) ? T_Const(0) : T_Const(1);
		return Tr_Ex(exp);
	} else {
		if (op == A_ltOp) 
			return (result->u.CONST < 0) ? Tr_Ex(T_Const(0)) : Tr_Ex(T_Const(1));
		else 
			return (result->u.CONST > 0) ? Tr_Ex(T_Const(0)) : Tr_Ex(T_Const(1));
	}
}

Tr_exp Tr_eqRef(A_oper op, Tr_exp exp_left, Tr_exp exp_right) {

	T_relOp relop;
	if (op == A_eqOp) 
		relop = T_eq; 
	else 
		relop = T_ne;
	T_stm condition = T_Cjump(relop, unEx(exp_left), unEx(exp_right), NULL, NULL);
	patchList trues = PatchList(&condition->u.CJUMP.true, NULL);
	patchList falses = PatchList(&condition->u.CJUMP.false, NULL);
	return Tr_Cx(trues, falses, condition);
}

Tr_exp Tr_relExp(A_oper op, Tr_exp exp_left, Tr_exp exp_right) {
	T_relOp relop;
	switch(op) {
		case A_gtOp: relop = T_gt; break;
		case A_geOp: relop = T_ge; break;
		case A_ltOp: relop = T_lt; break;
		case A_leOp: relop = T_le; break;
		default: assert(0); 
	}
	T_stm condition = T_Cjump(relop, unEx(exp_left), unEx(exp_right), NULL, NULL);
	patchList trues = PatchList(&condition->u.CJUMP.true, NULL);
	patchList falses = PatchList(&condition->u.CJUMP.false, NULL);
	return Tr_Cx(trues, falses, condition);
}



static Tr_exp Tr_StaticLink(Tr_level present, Tr_level definition) {
	
	T_exp address = T_Temp(F_FP());
	while (present && (present != definition->parent)) {
		F_access s = F_formals(present->frame)->head;
		address = F_Exp(s, address);
		present = present->parent;
	}
	return Tr_Ex(address);
}

static T_expList Tr_expList_convert(Tr_expList list) {
	
	T_expList head = NULL, tail = NULL;
	for (; list; list = list->tail) {
		T_exp temp = unEx(list->head);
		if (head) {
			tail->tail =  T_ExpList(temp, NULL);
			tail = tail->tail;
		}
		else {
			head = T_ExpList(temp, NULL);
			tail = head;
		}	
	}
	return head;
}

Tr_exp Tr_callExp(Temp_label label, Tr_level fun_level, Tr_level call_level , Tr_expList * list) {
	T_expList argslist = NULL;
	Tr_expList_prepend(Tr_StaticLink(call_level, fun_level), list);
	argslist = Tr_expList_convert(*list);
	return Tr_Ex(T_Call(T_Name(label), argslist));
}

static patchList PatchList(Temp_label * head, patchList tail) {
	patchList pointer = checked_malloc(sizeof(*pointer));
	pointer->head = head;
	pointer->tail = tail;
	return pointer;
}

static void doPatch(patchList tail, Temp_label label) {
	
	for (; tail; tail = tail->tail) {
		*(tail->head) = label;
	}
}

static patchList joinPatch(patchList first, patchList second) {
	
	if (!first) return second;
	for (; first->tail; first = first->tail);
	first->tail = second;
	return first;
}


Tr_access Tr_allocLocal(Tr_level level, bool esc) {
	Tr_access access = checked_malloc(sizeof(*access));
	access->level = level;
	access->access = F_allocLocal(level->frame, esc);

	return access;
}


Tr_level Tr_newLevel(Tr_level parent, Temp_label label, U_boolList frame) {
	Tr_level level = checked_malloc(sizeof(*level));
	level->name = label;
	level->parent = parent;
	level->frame = F_newFrame(label , U_BoolList(TRUE, frame));
	level->formals = makeFormalAccessList(level);
	
	return level;
}


Tr_accessList Tr_AccessList(Tr_access head, Tr_accessList tail) {
	Tr_accessList accesslist = checked_malloc(sizeof(*accesslist));
	accesslist->head = head;
	accesslist->tail = tail;
	return accesslist;
}

static Tr_access Tr_Access(Tr_level level, F_access access) {
	Tr_access tr_access = checked_malloc(sizeof(*tr_access));
	tr_access->level = level;
	tr_access->access = access;
	return tr_access;
}

Tr_accessList Tr_formals(Tr_level level) {
	return level->formals;
}

static Tr_accessList makeFormalAccessList(Tr_level level) {
	Tr_accessList head = NULL, tail = NULL;
	F_accessList  accesslist = F_formals(level->frame)->tail; 
	for (; accesslist; accesslist = accesslist->tail) {
		Tr_access access = Tr_Access(level, accesslist->head);
		if (head) {
			tail->tail = Tr_AccessList(access, NULL);
			tail = tail->tail;
		} 
		else {
			head = Tr_AccessList(access, NULL);
			tail = head;
		}
	}
	return head;
}

Tr_level Tr_outermost(void) {
	if (!outer_level)
		outer_level = Tr_newLevel(NULL, Temp_newlabel(), NULL);
	return outer_level;
}


void Tr_procEntryExit(Tr_level level, Tr_exp exp, Tr_accessList formal) {
	F_frag procfragment = F_ProcFrag(unNx(exp), level->frame);
	fragmentList = F_FragList(procfragment, fragmentList);
}

F_fragList Tr_getResult() {
	F_fragList current = NULL, previous = NULL;
	for (current = stringFragmentList; current; current = current->tail)
		previous = current;
	if (previous) previous->tail = fragmentList;
	return stringFragmentList ? stringFragmentList : fragmentList;
}

#include "test_translate.c"
