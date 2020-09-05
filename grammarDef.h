#ifndef _grammarDef
#define _grammarDef

typedef struct _symbol * symbol;
struct _symbol{
	char data[32];
	int is_non_terminal;
};

typedef struct _rule * rule;
struct _rule{
	rule next;
	symbol lhs;
	symbol * rhs;
	int rhsSize;
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
