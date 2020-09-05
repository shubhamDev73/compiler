#include <stdio.h>

#include "languageDef.h"
#include "grammarDef.h"

#ifndef _lexerDef
#define _lexerDef

#define SIZE_BUFFER 1024
#define SIZE_STRING 128

FILE * stream;

typedef struct _token * token;
struct _token {
	int line;
	symbol terminal;
	char lexeme[SIZE_ID + 1];
};

#endif
