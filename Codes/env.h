typedef struct E_enventry_ * E_enventry;

/* declaration of enventry */
struct E_enventry_ {
	enum {E_varEntry, E_funEntry} kind;
	union {
		struct {Tr_access access; Ty_ty ty;} var;
		struct {Tr_level level; Temp_label label; Ty_tyList formals; Ty_ty result;} fun;
	} u;
};

/* constructor of two types of entry */
E_enventry E_VarEntry(Tr_access access, Ty_ty ty);
E_enventry E_FunEntry(Tr_level level, Temp_label label, Ty_tyList, Ty_ty result);

/* constructor of two types of table */
S_table E_base_tenv(void);
S_table E_base_venv(void);
