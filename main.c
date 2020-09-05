#include <stdio.h>

#include "grammar.h"
#include "lexer.h"

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

int main(int argc, const char * argv[]){

	grammar G = readGrammar("grammar.txt");
	printTokens(argv[1], G);
	freeGrammar(G);

	return 0;

}
