#include "grammarDef.h"
#include "parserDef.h"

parseTree parseInputSourceCode(const char * fileName, grammar G);

void recursivePrintParseTree(parseTree tree);

void freeParseTree(parseTree tree);
