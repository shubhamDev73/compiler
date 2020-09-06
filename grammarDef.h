#ifndef _grammarDef
#define _grammarDef

typedef struct _symbol * symbol;
struct _symbol{
	char data[32];
	int is_non_terminal;
};

typedef struct _semantic * semantic;
struct _semantic{
	semantic next;
	int type; // 0: construct, 1: scope, 2: list, 3: type, 4: operator rearrangement, 5: prune
	int instruction; // 2-0: head, 2-1: next, 3-0: set basic, 3-1: set input, 3-2: set output, 3-3: set range
	int child;
};

typedef struct _rule * rule;
struct _rule{
	rule next;
	symbol lhs;
	symbol * rhs;
	int rhsSize;
	semantic semantics;
};

typedef struct _grammar * grammar;
struct _grammar{
	symbol start;
	symbol null;
	rule rules;
	symbol * terminals;
	symbol * non_terminals;
	int terminalsSize;
	int non_terminalsSize;
};

#endif
