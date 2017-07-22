#include <assert.h>

typedef char * string;
typedef char bool;

#define F_P
#define MAX_LENGTH 512

#define TRUE (char)1
#define FALSE (char)0

typedef struct boolList_ * U_boolList;
struct boolList_ { 
	bool head; 
	U_boolList tail; 
};

U_boolList U_BoolList(bool head, U_boolList tail);


void * checked_malloc(int);
string String(char *);