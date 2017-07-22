#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"
#include "frame.h"
#include "printtree.h"

static const int F_MAX_REG = 6;
const int F_WORD_SIZE = 4; 
static Temp_temp fp = NULL;
 

struct F_frame_ {
	F_accessList formals;
	Temp_label name;
	int local_count;
	
};

struct F_access_ {
	enum {inFrame, inReg} kind;
	union {
		int offs;
		Temp_temp reg;
	} u;
};

static F_access InFrame(int offsets);
static F_access InFrame(int offsets) {
	F_access access = checked_malloc(sizeof(*access));
	access->kind = inFrame;
	access->u.offs = offsets;
	return access;
}

static F_access InReg(Temp_temp reg);
static F_access InReg(Temp_temp reg) {
	F_access access = checked_malloc(sizeof(*access));
	access->kind = inReg;
	access->u.reg = reg;
	return access;
}

static F_accessList F_AccessList(F_access, F_accessList);

static F_accessList makeFormalAccessList(F_frame, U_boolList);

static F_accessList makeFormalAccessList(F_frame f, U_boolList formals) {
	F_accessList head = NULL, tail = NULL;
	U_boolList formal = formals;

	int i = 0;
	for (; formal; formal = formal->tail, i++) {
		F_access access = NULL;
		if (i < F_MAX_REG && !formal->head) {
			access = InReg(Temp_newtemp());
		}
		else {
			access = InFrame(i * F_WORD_SIZE);
		}
		if (head) {
			tail->tail = F_AccessList(access, NULL);
			tail = tail->tail;
		}
		else {
			head = F_AccessList(access, NULL);
			tail = head;
		}
	}
	return head;
}




static F_accessList F_AccessList(F_access head, F_accessList tail) {
	F_accessList list = checked_malloc(sizeof(*list));
	list->head = head;
	list->tail = tail;
	return list;
}


F_frame F_newFrame(Temp_label name, U_boolList formals) {
	F_frame frame = checked_malloc(sizeof(*frame));
	frame->name = name;
	frame->formals = makeFormalAccessList(frame, formals);
	frame->local_count = 0;
	return frame;
}

F_accessList F_formals(F_frame f) {
	return f->formals;
}

F_access F_allocLocal(F_frame f, bool escape) {

	f->local_count++;
	if (escape) {
		return InFrame(-1 * f->local_count*F_WORD_SIZE);
	}
	return InReg(Temp_newtemp());
}


F_frag F_ProcFrag(T_stm body, F_frame frame) {
	F_frag procfrag = checked_malloc(sizeof(*procfrag));
	procfrag->kind = F_procFrag;
	procfrag->u.proc.frame = frame;
	procfrag->u.proc.body = body;
	return procfrag;
}

F_frag F_StringFrag(Temp_label label, string str) {
	F_frag stringfrag = checked_malloc(sizeof(*stringfrag));
	stringfrag->kind = F_stringFrag;
	stringfrag->u.stringg.str = str;
	stringfrag->u.stringg.label = label;
	return stringfrag;
}

F_fragList F_FragList(F_frag head, F_fragList tail) {
	F_fragList fraglist = checked_malloc(sizeof(*fraglist));
	fraglist->head = head;
	fraglist->tail = tail;
	return fraglist;
}

T_exp F_Exp(F_access access, T_exp framePtr){ 
	if (access->kind == inFrame) {
		T_exp exp = T_Mem(T_Binop(T_plus, framePtr, T_Const(access->u.offs)));
		return exp;
	} 
	else {
		return T_Temp(access->u.reg);
	}
}

T_exp F_externalCall(string str, T_expList args) {
	return T_Call(T_Name(Temp_namedlabel(str)), args);
}

T_stm F_procEntryExit1(F_frame frame, T_stm stm) {
	return stm;
}

Temp_temp F_FP(void) {
	if (!fp) {
		fp = Temp_newtemp();
	}
	return fp;
}


