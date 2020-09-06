#include "languageDef.h"
#include "symbolTableDef.h"

#ifndef _astDef
#define _astDef

typedef struct _ast * ast;
struct _ast{
	int construct;
	symTable table;
	tableEntry entry;
	ast next;
	ast * children;
	int totalChildren;
	int line;
	type type;
	char lexeme[SIZE_ID + 1];
};

#endif
