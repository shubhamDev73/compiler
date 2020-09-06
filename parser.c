#include <stdlib.h>
#include <string.h>

#include "parserDef.h"
#include "lexer.h"

void insertInFF(firstAndFollow F, symbol non_terminal, set item, int first){

	// Finding insert location
	while(F->non_terminal != non_terminal){
		if(!F->next)
			break;
		F = F->next;
	}

	if(!F->non_terminal)
		// For empty F
		F->non_terminal = non_terminal;
	else if(F->non_terminal != non_terminal){
		// Reached last node
		firstAndFollow new = (firstAndFollow)malloc(sizeof(struct _firstAndFollow));
		new->non_terminal = non_terminal;
		new->first = NULL;
		new->follow = NULL;
		new->next = NULL;
		F->next = new;
		F = new;
	}

	// Inserting
	if(first)
		F->first = item;
	else
		F->follow = item;

	return;

}

int presentInSet(symbol terminal, set item){

	while(item){
		if(item->symbol == terminal)
			return 1;
		item = item->next;
	}
	return 0;

}

set createSet(symbol sym){

	set new = (set)malloc(sizeof(struct _set));
	new->symbol = sym;
	new->next = NULL;
	return new;

}

set computeFirstSet(grammar G, firstAndFollow F, symbol non_terminal){

	// If already computed, return it
	firstAndFollow ff = F;
	while(ff){
		if(ff->non_terminal == non_terminal){
			if(ff->first)
				return ff->first;
			else
				break;
		}
		ff = ff->next;
	}

	set first = NULL;
	set temp = first;

	rule _rule = G->rules;
	while(_rule){
		if(_rule->lhs == non_terminal){

			// If terminal found, break. Else non-terminal's first. If e present, next non-terminal
			for(int i = 0; i < _rule->rhsSize; i++)
				if(_rule->rhs[i]->is_non_terminal){
					set received = computeFirstSet(G, F, _rule->rhs[i]);
					int ePresent = 0;
					while(received){
						if(received->symbol == G->null)
							ePresent = 1;
						else if(!presentInSet(received->symbol, first)){
							first = createSet(received->symbol);
							first->next = temp;
							temp = first;
						}
						received = received->next;
					}
					if(!ePresent)
						break;
				}else{
					if(!presentInSet(_rule->rhs[i], first)){
						first = createSet(_rule->rhs[i]);
						first->next = temp;
						temp = first;
					}
					break;
				}
		}
		_rule = _rule->next;
	}

	insertInFF(ff ? ff : F, non_terminal, first, 1);

	return first;

}

set computeFollowSet(grammar G, firstAndFollow F, symbol non_terminal) {

	// If already computed, return it
	firstAndFollow ff = F;
	while(ff){
		if(ff->non_terminal == non_terminal){
			if(ff->follow)
				return ff->follow;
			else
				break;
		}
		ff = ff->next;
	}

	set follow = NULL;
	set temp = follow;

	rule _rule = G->rules;
	while(_rule){
		int i;

		for(i = 0; i < _rule->rhsSize - 1; i++){
			// follow = first(next_symbol), while e in first of (next_symbol)
			if(_rule->rhs[i] == non_terminal){
				int j = 1;
				while(j > 0 && j < _rule->rhsSize - i){
					int initJ = j;
					if(_rule->rhs[i + j]->is_non_terminal){
						set received = computeFirstSet(G, F, _rule->rhs[i + j]);
						while(received){
							if(received->symbol != G->null){
								if(!presentInSet(received->symbol, follow)){
									follow = createSet(received->symbol);
									follow->next = temp;
									temp = follow;
								}
							}else if(j == initJ)
								j++;
							received = received->next;
						}
					}else{
						if(!presentInSet(_rule->rhs[i + j], follow)){
							follow = createSet(_rule->rhs[i + j]);
							follow->next = temp;
							temp = follow;
						}
						j = 0;
					}
					if(j == initJ)
						break;
				}
			}
		}

		// If reached end, follow = follow(lhs)
		if(_rule->rhs[i] == non_terminal && _rule->lhs != non_terminal){
			set received = computeFollowSet(G, F, _rule->lhs);
			while(received){
				if(!presentInSet(received->symbol, follow)){
					follow = createSet(received->symbol);
					follow->next = temp;
					temp = follow;
				}
				received = received->next;
			}
		}
		_rule = _rule->next;
	}

	insertInFF(ff ? ff : F, non_terminal, follow, 0);

	return follow;

}

firstAndFollow computeFirstAndFollowSets(grammar G){

	firstAndFollow ff = (firstAndFollow)malloc(sizeof(struct _firstAndFollow));
	ff->non_terminal = NULL;
	ff->first = NULL;
	ff->follow = NULL;
	ff->next = NULL;

	for(int i = 0; i < G->non_terminalsSize; i++)
		computeFirstSet(G, ff, G->non_terminals[i]);
	for(int i = 0; i < G->non_terminalsSize; i++)
		computeFollowSet(G, ff, G->non_terminals[i]);
	return ff;

}

void freeFF(firstAndFollow ff){

	while(ff){
		firstAndFollow temp = ff->next;
		set first = ff->first;
		while(first){
			set temp = first->next;
			free(first);
			first = temp;
		}
		set follow = ff->follow;
		while(follow){
			set temp = follow->next;
			free(follow);
			follow = temp;
		}
		free(ff);
		ff = temp;
	}
	return;

}

rule findRuleInTable(parseTable T, grammar G, symbol terminal, symbol non_terminal){

	while(T){
		if(T->non_terminal == non_terminal && T->terminal == terminal)
			break;
		T = T->next;
	}

	if(T)
		return T->rule;
	else
		return NULL;

}

parseTable createTableNode(symbol terminal, symbol non_terminal, rule rule){

	parseTable pt = (parseTable)malloc(sizeof(struct _parseTable));
	pt->terminal = terminal;
	pt->non_terminal = non_terminal;
	pt->rule = rule;
	pt->next = NULL;
	return pt;

}

parseTable createParseTable(grammar G, firstAndFollow F){

	parseTable pt = NULL;
	parseTable temp = pt;

	rule _rule = G->rules;
	while(_rule){
		int j = 0;
		while(j < _rule->rhsSize){
			int initJ = j;
			int ePresent = 0;
			if(_rule->rhs[j]->is_non_terminal){
				firstAndFollow tempF = F;
				while(tempF->non_terminal != _rule->rhs[j])
					tempF = tempF->next;
				set first = tempF->first;
				while(first){
					if(first->symbol == G->null){
						if(!findRuleInTable(pt, G, first->symbol, _rule->lhs)){
							pt = createTableNode(first->symbol, _rule->lhs, _rule);
							pt->next = temp;
							temp = pt;
						}
						ePresent = 1;
						firstAndFollow tempF = F;
						while(tempF->non_terminal != _rule->lhs)
							tempF = tempF->next;
						set follow = tempF->follow;
						while(follow){
							if(!findRuleInTable(pt, G, follow->symbol, _rule->lhs)){
								pt = createTableNode(follow->symbol, _rule->lhs, _rule);
								pt->next = temp;
								temp = pt;
							}
							follow = follow->next;
						}
						if(j == initJ)
							j++;
					}else if(!findRuleInTable(pt, G, first->symbol, _rule->lhs)){
						pt = createTableNode(first->symbol, _rule->lhs, _rule);
						pt->next = temp;
						temp = pt;
					}
					first = first->next;
				}
			}else{
				if(_rule->rhs[j] == G->null){
					if(!findRuleInTable(pt, G, _rule->rhs[j], _rule->lhs)){
						pt = createTableNode(_rule->rhs[j], _rule->lhs, _rule);
						pt->next = temp;
						temp = pt;
					}
					ePresent = 1;
					j++;
					firstAndFollow tempF = F;
					while(tempF->non_terminal != _rule->lhs)
						tempF = tempF->next;
					set follow = tempF->follow;
					while(follow){
						if(!findRuleInTable(pt, G, follow->symbol, _rule->lhs)){
							pt = createTableNode(follow->symbol, _rule->lhs, _rule);
							pt->next = temp;
							temp = pt;
						}
						follow = follow->next;
					}
				}else if(!findRuleInTable(pt, G, _rule->rhs[j], _rule->lhs)){
					pt = createTableNode(_rule->rhs[j], _rule->lhs, _rule);
					pt->next = temp;
					temp = pt;
				}
			}
			if(!ePresent)
				break;
		}
		_rule = _rule->next;
	}
	return pt;

}

void freeParseTable(parseTable T){

	while(T){
		parseTable temp = T->next;
		free(T);
		T = temp;
	}
	return;

}

parseTree createParseTreeNode(parseTree parent, symbol sym, int childNum, int line){

	parseTree tree = (parseTree)malloc(sizeof(struct _parseTree));
	tree->parent = parent;
	tree->symbol = sym;
	tree->children = NULL;
	tree->totalChildren = 0;
	tree->childNum = childNum;
	tree->line = line;
	tree->filled = 0;
	strcpy(tree->lexeme, "");
	return tree;

}

stack createStack(symbol sym, stack next){

	stack _stack = (stack)malloc(sizeof(struct _stack));
	_stack->symbol = sym;
	_stack->next = next;
	return _stack;

}

parseTree parseInputSourceCode(const char * fileName, grammar G){

	firstAndFollow F = computeFirstAndFollowSets(G);
	parseTable T = createParseTable(G, F);

	char errorSymbol[SIZE_ID + 1] = "";

	openStream(fileName);

	// Start parsing with start symbol

	parseTree tree = createParseTreeNode(NULL, G->start, 0, 0);
	parseTree current = tree;

	stack top = createStack(G->start, NULL);
	token input = getNextToken(G);

	while(top && input){
		symbol pop = top->symbol;
		stack temp = top->next;
		free(top);
		top = temp;
		if(pop->is_non_terminal){
			// Find rule
			rule _rule = findRuleInTable(T, G, input->terminal, pop);
			if(!_rule){
				_rule = findRuleInTable(T, G, G->null, pop);
				if(!_rule){
					// Syntax error
					if(strcmp(errorSymbol, input->lexeme)){
						printf("Line: %d, Syntax error: input: %s\n", input->line, input->lexeme);
						tree->line++;
					}
					strcpy(errorSymbol, "");
					firstAndFollow ff = F;
					while(ff){
						if(ff->non_terminal == pop)
							break;
						ff = ff->next;
					}
					set follow = ff->follow;
					set first = ff->first;

 					// Panic recovery, find token in either first or follow of top
					while(input && !presentInSet(input->terminal, follow) && !presentInSet(input->terminal, first))
						input = getNextToken(G);

					if(input){
						_rule = findRuleInTable(T, G, input->terminal, pop);
						if(!_rule)
							continue;
					}else
						break;
				}
			}

			// Create children based on the rule

			current->totalChildren = _rule->rhsSize;
			current->children = (parseTree *)malloc(sizeof(parseTree) * _rule->rhsSize);
			for(int i = 0; i < _rule->rhsSize; i++){
				// Insert in stack in reverse order
				top = createStack(_rule->rhs[_rule->rhsSize - 1 - i], top);
				current->children[i] = createParseTreeNode(current, _rule->rhs[i], i, input->line);
			}

			// Assign next node to be evaluated
			int i;
			for(i = 0; i < _rule->rhsSize; i++){
				if(current->children[i]->symbol->is_non_terminal){
					current = current->children[i];
					break;
				}
			}

			// If not in children, find in parent(s)
			if(i == _rule->rhsSize){
				int j;
				do{
					int start = current->childNum + 1;
					current = current->parent;
					for(j = start; j < current->totalChildren; j++){
						if(current->children[j]->symbol->is_non_terminal){
							current = current->children[j];
							break;
						}
					}
				}while(j == current->totalChildren);
			}

		}else{
			if(pop == G->null){
				// Fill null productions in parse tree, will free later
				rule _rule = findRuleInTable(T, G, pop, current->symbol);
				if(_rule){
					current->totalChildren = _rule->rhsSize;
					current->children = (parseTree *)malloc(sizeof(parseTree) * _rule->rhsSize);
					for(int i = 0; i < _rule->rhsSize; i++)
						current->children[i] = createParseTreeNode(current, _rule->rhs[i], i, input->line);
				}
				continue;
			}
			if(pop == input->terminal){
				// Assign the terminal to first non-filled node
				int i, loop;
				parseTree temp = current;
				do{
					loop = 0;
					temp = temp->parent;

					for(i = 0; i < temp->totalChildren; i++){
						if(!temp->children[i]->filled && temp->children[i]->symbol != G->null){
							if(temp->children[i]->symbol->is_non_terminal){
								// Non terminal is empty, hence it's child needs to be filled
								temp = temp->children[i]->children[0];
								loop = 1;
							}else{
								// Fill terminal
								temp->children[i]->line = input->line;
								strcpy(temp->children[i]->lexeme, input->lexeme);
								temp->children[i]->filled = 1;
							}
							break;
						}
					}

					// Couldn't find any empty node, hence it must be filled
					if(!loop && i == temp->totalChildren)
						temp->filled = 1;
				}while(loop || i == temp->totalChildren);
			}else{
				// Invalid token. Syntax error
				printf("Line: %d, Syntax error: input: %s\n", input->line, input->lexeme);
				tree->line++;
				strcpy(errorSymbol, input->lexeme);

				// Removing terminals from stack
				while(top){
					pop = top->symbol;
					if(pop->is_non_terminal)
						break;
					temp = top->next;
					free(top);
					top = temp;
				}
				continue;
			}
			input = getNextToken(G);
		}
	}

	if(!input){
		while(top){
			rule _rule = findRuleInTable(T, G, G->null, top->symbol);
			if(!_rule){
				printf("Line: EOF, Syntax error: stack top: %s\n", top->symbol->data);
				tree->line++;
				break;
			}
			stack temp = top->next;
			free(top);
			top = temp;
		}
	}else{
		printf("Line: %d, Syntax error: input: %s\n", input->line, input->terminal->data);
		tree->line++;
	}

	closeStream();
	freeParseTable(T);
	freeFF(F);
	return tree;

}

void recursivePrintParseTree(parseTree tree){

	// Pre-order traversal, first current then children
	printf("%d\t%s\t%s\t%s\t%d\n",
		tree->line,
		tree->symbol->data,
		tree->parent ? tree->parent->symbol->data : "ROOT",
		tree->lexeme,
		tree->totalChildren
	);
	for(int i = 0; i < tree->totalChildren; i++)
		recursivePrintParseTree(tree->children[i]);
	return;

}

void freeParseTree(parseTree tree){

	for(int i = 0; i < tree->totalChildren; i++)
		freeParseTree(tree->children[i]);
	free(tree->children);
	free(tree);
	return;

}
