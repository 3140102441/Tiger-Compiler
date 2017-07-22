#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "symbol.h"
#include "temp.h"
#include "table.h"

static int label_cnt = 0;
static int temp_cnt = 100;
static FILE *out_file;

struct Temp_temp_ {
	int num;
};

int Temp_tempint(Temp_temp t) {
	return t->num;
}

string Temp_labelstring(Temp_label s){
	return S_name(s);
}


Temp_label Temp_newlabel(void){
	char buffer[100];
	sprintf(buffer, "L%d", label_cnt++);
	return Temp_namedlabel(String(buffer));
}

Temp_label Temp_namedlabel(string s){
	return S_Symbol(s);
}


Temp_temp Temp_newtemp(void){
 Temp_temp pointer = (Temp_temp) checked_malloc(sizeof (*pointer));
 pointer->num = temp_cnt++;
 {
  char reg[16];
  sprintf(reg, "%d", pointer->num);
  Temp_enter(Temp_name(), pointer, String(reg));
 }
 return pointer;
}



struct Temp_map_ {
	TAB_table tab; 
	Temp_map under;
};


Temp_map Temp_name(void) {
 static Temp_map map = NULL;
 if (!map) 
	 map=Temp_empty();
 return map;
}

Temp_map newMap(TAB_table tab, Temp_map under) {
  Temp_map map = checked_malloc(sizeof(*map));
  map->under = under;
  map->tab=tab;
  return map;
}

Temp_map Temp_empty(void) {
  return newMap(TAB_empty(), NULL);
}

Temp_map Temp_layerMap(Temp_map over, Temp_map under) {
  if (over==NULL)
      return under;
  else 
	  return newMap(over->tab, Temp_layerMap(over->under, under));
}

void Temp_enter(Temp_map m, Temp_temp t, string s) {
  assert(m && m->tab);
  TAB_enter(m->tab,t,s);
}

string Temp_look(Temp_map m, Temp_temp t) {
  string str;
  assert(m && m->tab);
  str = TAB_look(m->tab, t);
  if (str) 
	  return str;
  else if (m->under) 
	  return Temp_look(m->under, t);
  else 
	  return NULL;
}

Temp_tempList Temp_TempList(Temp_temp h, Temp_tempList t) {
 Temp_tempList pointer = (Temp_tempList)checked_malloc(sizeof(*pointer));
 pointer->head = h; 
 pointer->tail = t;
 return pointer;
}

Temp_labelList Temp_LabelList(Temp_label h, Temp_labelList t){
 Temp_labelList pointer = (Temp_labelList)checked_malloc(sizeof(*pointer));
 pointer->head = h; 
 pointer->tail = t;
 return pointer;
}

void show(Temp_temp t, string r) {
  fprintf(out_file, "t%d -> %s\n", t->num, r);
}

void Temp_dumpMap(FILE *out, Temp_map m) {
  out_file=out;
  TAB_dump(m->tab,(void (*)(void *, void*))show);
  if (m->under) {
     fprintf(out,"*************\n");
     Temp_dumpMap(out,m->under);
  }
}
