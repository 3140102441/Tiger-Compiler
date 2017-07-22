#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "types.h"


static struct Ty_ty_ ty_nil = {Ty_nil};
static struct Ty_ty_ ty_int = { Ty_int };
static struct Ty_ty_ ty_string = { Ty_string };
static struct Ty_ty_ ty_void = { Ty_void };
static struct Ty_ty_ ty_double = { Ty_double };

Ty_ty Ty_Nil(void) {
	return &ty_nil;
}


Ty_ty Ty_Int(void) {
	return &ty_int;
}


Ty_ty Ty_Double(void) {
	return &ty_double;
}



Ty_ty Ty_String(void) {
	return &ty_string;
}


Ty_ty Ty_Void(void) {
	return &ty_void;
}


Ty_ty Ty_Record(Ty_fieldList fields){
 Ty_ty pointer = checked_malloc(sizeof(*pointer));
 pointer->u.record = fields;
 pointer->kind = Ty_record;
 return pointer;
}

Ty_ty Ty_Array(Ty_ty ty){
 Ty_ty pointer = checked_malloc(sizeof(*pointer));
 pointer->u.array = ty;
 pointer->kind = Ty_array;
 return pointer;
}

Ty_ty Ty_Name(S_symbol sym, Ty_ty ty){
 Ty_ty pointer = checked_malloc(sizeof(*pointer));
 pointer->u.name.sym = sym;
 pointer->u.name.ty = ty;
 pointer->kind = Ty_name;
 return pointer;
}


Ty_tyList Ty_TyList(Ty_ty head, Ty_tyList tail){
 Ty_tyList pointer = checked_malloc(sizeof(*pointer));
 pointer->head = head;
 pointer->tail = tail;
 return pointer;
}

Ty_field Ty_Field(S_symbol name, Ty_ty ty){
 Ty_field pointer = checked_malloc(sizeof(*pointer));
 pointer->ty = ty;
 pointer->name = name;
 return pointer;
}

Ty_fieldList Ty_FieldList(Ty_field head, Ty_fieldList tail){
 Ty_fieldList pointer = checked_malloc(sizeof(*pointer));
 pointer->head = head;
 pointer->tail = tail;
 return pointer;
}




