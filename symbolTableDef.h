#include "languageDef.h"

#ifndef _symbolTableDef
#define _symbolTableDef

#define USE_DEFAULT 0
#define USE_INPUT 1
#define USE_OUTPUT 2
#define USE_FOR 3
#define USE_WHILE 4

typedef struct _type * type;
struct _type{
	int basic;
	type input, output;
	int start, end;
	char startLexeme[SIZE_ID + 1], endLexeme[SIZE_ID + 1];
};

typedef struct _tableEntry * tableEntry;
struct _tableEntry{
	tableEntry next;
	char id[SIZE_ID + 1];
	int width;
	int offset;
	type type;
	int useType;
	int used;
};

typedef struct _symTable * symTable;
struct _symTable{
	symTable parent;
	char id[SIZE_ID + 1];
	tableEntry entries[SYMBOL_TABLE_ENTRIES];
	int width;
};

#endif
