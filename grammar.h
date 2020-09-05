#include "grammarDef.h"

symbol findSymbol(const char * c, int start, int end, grammar G);

grammar readGrammar(const char * grammarFile);

void freeGrammar(grammar G);
