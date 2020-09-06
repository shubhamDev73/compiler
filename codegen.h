#include "astDef.h"
#include "codegenDef.h"

void newCode();

code generateCode(ast tree, int currentLabel);

void outputCode(code _code, const char * outFile);
