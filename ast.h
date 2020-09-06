#include "parserDef.h"
#include "astDef.h"

type createType(int basic);

ast createAST(parseTree tree, ast parent, symTable table);

void recursivePrintAST(ast tree);

void freeAST(ast tree);
