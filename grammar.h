#include "grammarDef.h"

symbol findSymbol(const char * c, int start, int end, grammar G);

grammar readGrammar(const char * grammarFile);

void readSemantics(const char * semanticsFile, grammar G);

void freeGrammar(grammar G);
