#ifndef SEM_H
#define SEM_H

F_fragList SEM_transProg(A_exp exp);

struct expty {
	Tr_exp exp; 
	Ty_ty ty;
};


#endif
