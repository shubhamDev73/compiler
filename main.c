#include <stdio.h>

#include "grammar.h"
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "symbolTable.h"

void printGrammar(grammar G){

	rule _rule = G->rules;
	while(_rule){
		printf("%s: ", _rule->lhs->data);
		for(int i = 0; i < _rule->rhsSize; i++)
			printf("%s ", _rule->rhs[i]->data);
		_rule = _rule->next;
		printf("\n");
	}

}

void printTokens(const char * fileName, grammar G){

	printf("Line\tLexeme\t\tName\n");
	printf("-----------------------------\n");

	openStream(fileName);
	token _token;
	while(_token = getNextToken(G))
		printf("%d\t%s\t\t%s\n", _token->line, _token->lexeme, _token->terminal->data);
	closeStream();

}

void printParseTree(parseTree pt){

	printf("Line\tSymbol\tParent\tLexeme\tNumber of children\n");
	printf("----------------------------------------------------\n");
	recursivePrintParseTree(pt);

}

void printAST(ast _ast){

	printf("Line\tConstruct\tLexeme\t\tNumber of children\n");
	printf("-----------------------------------------------------------\n");
	recursivePrintAST(_ast);

}

int main(int argc, const char * argv[]){

	grammar G = readGrammar("grammar.txt");
	readSemantics("semantics.txt", G);
	parseTree pt = parseInputSourceCode(argv[1], G);

	if(pt->line == 0){
		ast _ast = createAST(pt, NULL, NULL);
		freeGrammar(G);
		printAST(_ast);
		createSymbolTable(_ast, _ast);
		freeAST(_ast);
	}else{
		freeParseTree(pt);
		freeGrammar(G);
	}

	return 0;

}
