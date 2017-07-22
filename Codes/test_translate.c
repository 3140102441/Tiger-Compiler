void print(Tr_exp exp, FILE * out, string parent) {
	if (exp->kind == Tr_ex) printExp(unEx(exp), out, parent);
	if (exp->kind == Tr_nx) printStm(unNx(exp), out, parent);
	if (exp->kind == Tr_cx) printStm(unCx(exp).stm, out, parent);
}

void print_frag(F_fragList flist, FILE * out) {
	if (!flist) {
		puts("There is no fragment!");
		return;
	}
	string s;
	string name = malloc(sizeof(char) * 20);

	s = makeparent("EXP", "null");
	fprintf(out, "%s", s);
	fprintf(out, "\n");

	while (flist) {
		F_frag fragment = flist->head;

		switch (fragment->kind) {
		case F_stringFrag:
			//print(Tr_Ex(T_Name(fragment->u.stringg.label)), out,"null");

			break;
		case F_procFrag:
			print(Tr_Nx(fragment->u.proc.body), out, "null");
			if (flist->tail)
				fprintf(out, ",\n");
			break;
		default: assert(0 && "The kind of fragment is not correct!");
		}
		flist = flist->tail;
	}
	fprintf(out, "]}\n");
}
