#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"



U_boolList U_BoolList(bool head, U_boolList tail)
{
	U_boolList boollist = checked_malloc(sizeof(*boollist));
	boollist->head = head;
	boollist->tail = tail;
	return boollist;
}


void *checked_malloc(int lenth)
{
	void *pointer = malloc(lenth);
	if (!pointer) {
		fprintf(stderr, "\nThe memory is already full!\n");
		exit(1);
	}
	return pointer;
}

string String(char *str)
{
	string newstr = checked_malloc(strlen(str) + 1);
	strcpy(newstr,str);
	return newstr;
}

