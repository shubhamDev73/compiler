#include "astDef.h"

void createSymbolTable(ast tree, ast root);

tableEntry getInSymbolTable(ast tree, symTable table);

tableEntry getIdInSymbolTable(char * id, symTable table);
