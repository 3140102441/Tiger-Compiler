#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "tree.h"

T_stm T_Seq(T_stm left, T_stm right)
{
	T_stm pointer = (T_stm)checked_malloc(sizeof *pointer);
	pointer->kind = T_SEQ;
	pointer->u.SEQ.left = left;
	pointer->u.SEQ.right = right;
	return pointer;
}

T_stm T_Label(Temp_label label)
{
	T_stm pointer = (T_stm)checked_malloc(sizeof *pointer);
	pointer->kind = T_LABEL;
	pointer->u.LABEL = label;
	return pointer;
}



T_stm T_Jump(T_exp exp, Temp_labelList labels)
{
	T_stm pointer = (T_stm)checked_malloc(sizeof *pointer);
	pointer->kind = T_JUMP;
	pointer->u.JUMP.exp = exp;
	pointer->u.JUMP.jumps = labels;
	return pointer;
}

T_stm T_Cjump(T_relOp op, T_exp left, T_exp right, Temp_label true, Temp_label false)
{
	T_stm pointer = (T_stm)checked_malloc(sizeof *pointer);
	pointer->kind = T_CJUMP;
	pointer->u.CJUMP.op = op;
	pointer->u.CJUMP.left = left;
	pointer->u.CJUMP.right = right;
	pointer->u.CJUMP.true = true;
	pointer->u.CJUMP.false = false;
	return pointer;
}

T_stm T_Move(T_exp dst, T_exp src)
{
	T_stm pointer = (T_stm)checked_malloc(sizeof *pointer);
	pointer->kind = T_MOVE;
	pointer->u.MOVE.dst = dst;
	pointer->u.MOVE.src = src;
	return pointer;
}

T_stm T_Exp(T_exp exp)
{
	T_stm pointer = (T_stm)checked_malloc(sizeof *pointer);
	pointer->kind = T_EXP;
	pointer->u.EXP = exp;
	return pointer;
}

T_exp T_Binop(T_binOp op, T_exp left, T_exp right)
{
	T_exp pointer = (T_exp)checked_malloc(sizeof *pointer);
	pointer->kind = T_BINOP;
	pointer->u.BINOP.op = op;
	pointer->u.BINOP.left = left;
	pointer->u.BINOP.right = right;
	return pointer;
}

T_exp T_Mem(T_exp exp)
{
	T_exp pointer = (T_exp)checked_malloc(sizeof *pointer);
	pointer->kind = T_MEM;
	pointer->u.MEM = exp;
	return pointer;
}

T_exp T_Temp(Temp_temp temp)
{
	T_exp pointer = (T_exp)checked_malloc(sizeof *pointer);
	pointer->kind = T_TEMP;
	pointer->u.TEMP = temp;
	return pointer;
}

T_exp T_Eseq(T_stm stm, T_exp exp)
{
	T_exp pointer = (T_exp)checked_malloc(sizeof *pointer);
	pointer->kind = T_ESEQ;
	pointer->u.ESEQ.stm = stm;
	pointer->u.ESEQ.exp = exp;
	return pointer;
}

T_exp T_Name(Temp_label name)
{
	T_exp pointer = (T_exp)checked_malloc(sizeof *pointer);
	pointer->kind = T_NAME;
	pointer->u.NAME = name;
	return pointer;
}

T_exp T_Const(int consti)
{
	T_exp pointer = (T_exp)checked_malloc(sizeof *pointer);
	pointer->kind = T_CONST;
	pointer->u.CONST = consti;
	return pointer;
}

T_exp T_Double(float f) {
	T_exp pointer = checked_malloc(sizeof(*pointer));
	pointer->kind = T_DOUBLE;
	pointer->u.DOUBLE = f;
	return pointer;
}

T_exp T_Call(T_exp fun, T_expList args)
{
	T_exp pointer = (T_exp)checked_malloc(sizeof *pointer);
	pointer->kind = T_CALL;
	pointer->u.CALL.fun = fun;
	pointer->u.CALL.args = args;
	return pointer;
}

T_expList T_ExpList(T_exp head, T_expList tail)
{
	T_expList pointer = (T_expList)checked_malloc(sizeof *pointer);
	pointer->head = head;
	pointer->tail = tail;
	return pointer;
}

T_stmList T_StmList(T_stm head, T_stmList tail)
{
	T_stmList pointer = (T_stmList)checked_malloc(sizeof *pointer);
	pointer->head = head;
	pointer->tail = tail;
	return pointer;
}

T_relOp T_notRel(T_relOp r)
{
	switch (r)
	{
		case T_le:
		return T_gt;
		case T_ult:
		return T_uge;
		case T_eq:
			return T_ne;
		case T_ge:
			return T_lt;
		case T_gt:
			return T_le;
		case T_ne:
			return T_eq;
		case T_ule:
			return T_ugt;
		case T_ugt:
			return T_ule;
		case T_lt:
			return T_ge;
		case T_uge:
			return T_ult;
	}
	assert(0);
	return 0;
}

T_relOp T_commute(T_relOp r)
{
	switch (r)
	{
		case T_lt:
			return T_gt;
		case T_ge:
			return T_le;
		case T_eq:
			return T_eq;
		case T_le:
			return T_ge;
		case T_ult:
			return T_ugt;
		case T_ne:
			return T_ne;
		case T_ule:
			return T_uge;
		case T_ugt:
			return T_ult;
		case T_gt:
			return T_lt;
		case T_uge:
			return T_ule;
	}
	assert(0);
	return 0;
}
