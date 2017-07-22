/* table.h - generic hash table */

typedef struct TAB_table_ *TAB_table;

/* Make a new table */
TAB_table TAB_empty(void);

/* Enter key->value into table t, shadowing any previous binding for key. */
void TAB_enter(TAB_table t, void *key, void *value);

/* Look up key in table t */
void *TAB_look(TAB_table t, void *key);

/* Pop the most recent binding and return its key. This may expose another binding for the same key. */
void *TAB_pop(TAB_table t);

void TAB_dump(TAB_table t, void (*show)(void *key, void *value));
