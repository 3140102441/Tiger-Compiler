#ifndef TIGER_PRINTTREE_H_
#define TIGER_PRINTTREE_H_


void printExp(T_exp, FILE * out);
void printStm(T_stm, FILE * out);
void printStmList(FILE * out, T_stmList stmList,string parent);

#endif


