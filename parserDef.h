#include "languageDef.h"
#include "grammarDef.h"

#ifndef _parserDef
#define _parserDef

typedef struct _set * set;
struct _set{
	set next;
	symbol symbol;
};

typedef struct _firstAndFollow * firstAndFollow;
struct _firstAndFollow{
	firstAndFollow next;
	symbol non_terminal;
	set first;
	set follow;
};

typedef struct _parseTable * parseTable;
struct _parseTable{
	parseTable next;
	symbol terminal;
	symbol non_terminal;
	rule rule;
};

typedef struct _parseTree * parseTree;
struct _parseTree{
	parseTree parent;
	symbol symbol;
	parseTree * children;
	int totalChildren;
	int childNum;
	int line;
	int filled;
	semantic semantics;
	char lexeme[SIZE_ID + 1];
};

typedef struct _stack * stack;
struct _stack{
	stack next;
	symbol symbol;
};

#endif
