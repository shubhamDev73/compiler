#include <stdio.h>

#include "grammar.h"

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

int main(int argc, const char * argv[]){

	grammar G = readGrammar("grammar.txt");
	printGrammar(G);
	freeGrammar(G);

	return 0;

}
