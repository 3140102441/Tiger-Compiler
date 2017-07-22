#include <stdio.h>
#include "util.h"
#include "table.h"

#define TABSIZE 109

typedef struct bucket_ *bucket;
struct bucket_ { void *key; void *value; bucket next; };
struct TAB_table_ {
	bucket table[TABSIZE];
	bucket stack;
};

TAB_table TAB_empty(void)
{
	TAB_table t = checked_malloc(sizeof(*t));
	int i;
	t->stack = NULL;
	for (i = 0; i < TABSIZE; i++)
		t->table[i] = NULL;
	return t;
}

void TAB_enter(TAB_table t, void *key, void *value)
{
	if (t == NULL || key == NULL)
	{
		printf("null pointer error.");
		return;
	}
	int index;
	index = ((unsigned)key) % TABSIZE;

	bucket b = checked_malloc(sizeof(*b));
	b->key = key; b->value = value; b->next = t->table[index];
	t->table[index] = b;

	bucket s = checked_malloc(sizeof(*s));
	s->key = key; s->next = t->stack;
	t->stack = s;
}

void *TAB_look(TAB_table t, void *key)
{
	if (t == NULL || key == NULL)
	{
		printf("null pointer error.");
		return;
	}
	int index;
	bucket b;
	index = ((unsigned)key) % TABSIZE;
	for (b = t->table[index]; b; b = b->next)
		if (b->key == key) return b->value;
	return NULL;
}

void *TAB_pop(TAB_table t) {
	if (t == NULL)
	{
		printf("null pointer error.");
		return NULL;
	}
	bucket s, b; int index;
	s = t->stack;
	if (s == NULL)
	{
		printf("the table is empty.");
		return NULL;
	}
	index = ((unsigned)s->key) % TABSIZE;
	b = t->table[index];
	if (b == NULL)
	{
		printf("unexpected pop error.");
		return NULL;
	}
	t->table[index] = b->next;
	t->stack = s->next;
	return b->key;
}

void TAB_dump(TAB_table t, void(*show)(void *key, void *value)) {
	bucket s = t->stack;
	int index = ((unsigned)s->key) % TABSIZE;
	bucket b = t->table[index];
	t->table[index] = b->next;
	t->stack = s->next;
	show(b->key, b->value);
	TAB_dump(t, show);
	t->stack = s;
	t->table[index] = b;
}
