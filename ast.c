#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "parserDef.h"
#include "astDef.h"

type createType(int basic){

	type _type = (type)malloc(sizeof(struct _type));
	_type->basic = basic;
	_type->input = NULL;
	_type->output = NULL;
	_type->start = 0;
	_type->end = 0;
	strcpy(_type->startLexeme, "");
	strcpy(_type->endLexeme, "");
	return _type;

}

void inheritedSemantics(ast _ast, semantic semantics){

	// Inherited attribute semantics, to be done before creating children
	// And, possibly passed down to children

	symTable table;
	while(semantics){
		switch(semantics->type){
			case 0:
				// construct semantics
				_ast->construct = semantics->instruction;
				break;
			case 1:
				// scope semantics
				// Creating empty symbol table
				table = (symTable)malloc(sizeof(struct _symTable));
				switch(_ast->construct){
					case CONSTRUCT_PROGRAM:
						strcpy(table->id, "<ROOT>");
						break;
					case CONSTRUCT_ENTRY:
						strcpy(table->id, "driver");
						break;
					default:
						// Will be filled after creating children
						strcpy(table->id, "");
						break;
				}
				for(int i = 0; i < SYMBOL_TABLE_ENTRIES; i++)
					table->entries[i] = NULL;
				table->width = 0;
				table->parent = _ast->table;
				_ast->table = table;
				break;
		}
		semantics = semantics->next;
	}
	return;

}

void synthesizedSemantics(ast _ast, ast parent, semantic semantics){

	// Synthesized attributes semantics, to be done after creating children
	// And, possibly taken from children

	int child = 0;
	ast temp = NULL;
	type tempType = NULL;

	while(semantics){
		switch(semantics->type){
			case 2:
				// list semantic
				switch(semantics->instruction){
					case 0:
						// head
						child = semantics->child;
						break;
					case 1:
						// next
						if(child < _ast->totalChildren && _ast->children[child]){
							temp = _ast->children[child];
							while(temp->next)
								temp = temp->next;
							temp->next = _ast->children[semantics->child];
						}else
							_ast->next = _ast->children[semantics->child];
						_ast->children[semantics->child] = NULL;
						break;
				}
				break;
			case 3:
				// type semantic
				switch(semantics->instruction){
					case 0:
						// set basic
						tempType = _ast->type;
						_ast->type = createType(semantics->child);
						break;
					case 1:
						// set input
						if(tempType)
							_ast->type->input = tempType;
						else if(_ast->children[semantics->child])
							_ast->type->input = _ast->children[semantics->child]->type;
						break;
					case 2:
						// set output
						if(tempType)
							_ast->type->output = tempType;
						else if(_ast->children[semantics->child])
							_ast->type->output = _ast->children[semantics->child]->type;
						break;
					case 3:
						// set range
						temp = _ast->children[semantics->child];

						// start
						if(temp->children[0]->type)
							// static
							_ast->type->start = atoi(temp->children[0]->lexeme);
						else
							// dynamic
							strcpy(_ast->type->startLexeme, temp->children[0]->lexeme);
						free(temp->children[0]);

						// end
						if(temp->children[1]->type)
							// static
							_ast->type->end = atoi(temp->children[1]->lexeme);
						else
							// dynamic
							strcpy(_ast->type->endLexeme, temp->children[1]->lexeme);

						// Copied information, now freeing
						free(temp->children[1]);
						free(temp->children);
						free(temp);
						_ast->children[semantics->child] = NULL;
						break;
				}
				break;
			case 4:
				// operator rearrangement semantic
				if(semantics->child){
					_ast->children[0]->construct = CONSTRUCT_OPERATOR;
					_ast->children[0]->totalChildren = 1;

					if(semantics->child == 9){
						// unary minus
						_ast->children[0]->children = (ast *)malloc(sizeof(ast));
						_ast->children[0]->children[0] = _ast->children[1];
					}else{
						_ast->children[0]->children = (ast *)malloc(sizeof(ast) * 2);
						_ast->children[0]->children[0] = NULL;
						_ast->children[0]->children[1] = _ast->children[1];
					}
					_ast->children[1] = NULL;
				}

				temp = _ast->children[_ast->totalChildren - 1];
				while(temp && temp->totalChildren == 2)
					temp = temp->children[0];
				if(!temp || temp->children[0])
					break;
				temp->children[0] = _ast->children[0];
				temp->totalChildren = 2;
				_ast->children[0] = NULL;
				break;
			case 5:
				// node pruning
				while(_ast->children[semantics->child]){

					if(_ast->construct == CONSTRUCT_IO && semantics->child == 0)
						// Copying IO type before pruning
						strcpy(_ast->lexeme, _ast->children[semantics->child]->lexeme);

					temp = _ast->children[semantics->child]->next;
					free(_ast->children[semantics->child]);
					_ast->children[semantics->child] = temp;
				}
				break;
		}
		semantics = semantics->next;
	}
	return;

}

ast createASTNode(parseTree tree, symTable table){

	ast _ast = (ast)malloc(sizeof(struct _ast));
	_ast->construct = CONSTRUCT_STATEMENTS;
	_ast->table = table; // Copying parent scope. If new needed, semantics will create it
	_ast->entry = NULL;
	_ast->next = NULL;
	_ast->totalChildren = tree->totalChildren;
	_ast->children = (ast *)malloc(sizeof(ast) * _ast->totalChildren);
	_ast->line = tree->line;
	_ast->type = NULL;
	strcpy(_ast->lexeme, tree->lexeme);
	return _ast;

}

ast createAST(parseTree tree, ast parent, symTable table){

	ast _ast = createASTNode(tree, table);
	int children = _ast->totalChildren;

	inheritedSemantics(_ast, tree->semantics);
	for(int i = 0; i < children; i++)
		_ast->children[i] = createAST(tree->children[i], _ast, _ast->table);
	synthesizedSemantics(_ast, parent, tree->semantics);

	// Removing pruned nodes
	for(int i = 0; i < children; i++){
		if(!_ast->children[i]){
			for(int j = i + 1; j < children; j++){
				_ast->children[j - 1] = _ast->children[j];
			}
			children--;
			i--;
		}
	}
	_ast->totalChildren = children;
	_ast->children = (ast *)realloc(_ast->children, sizeof(ast) * _ast->totalChildren);

	// Current node pruning
	if(tree->symbol->is_non_terminal){

		if(children == 0 && _ast->construct != CONSTRUCT_ENTRY){
			if(_ast->type && parent && !parent->type)
				parent->type = _ast->type;
			free(_ast->children);
			free(_ast);
			_ast = NULL;
		}

		// Possible single children nodes: program (empty code), entry (for starting point), operator (unary minus), IO
		if(children == 1 && _ast->construct != CONSTRUCT_PROGRAM && _ast->construct != CONSTRUCT_ENTRY && _ast->construct != CONSTRUCT_OPERATOR && _ast->construct != CONSTRUCT_IO){
			// Becoming children[0], and passing relevant information
			ast temp = _ast;
			_ast = _ast->children[0];
			ast current = _ast;

			if(temp->type){
				// Assignning type to children
				while(current){
					if(!current->type)
						current->type = temp->type;
					current = current->next;
				}
			}

			// Copying construct
			if(temp->construct != CONSTRUCT_STATEMENTS && temp->construct != CONSTRUCT_WHICH_ID)
				_ast->construct = temp->construct;

			free(temp);
		}
	}
	free(tree->children);
	free(tree);
	return _ast;

}

void recursivePrintAST(ast tree){

	// int to string conversion
	char symbols[14][14] = {
		"PROGRAM",
		"ENTRY",
		"MODULE",
		"VARIABLE",
		"WHICH_ID",
		"CONSTANT",
		"STATEMENTS",
		"DECLARE",
		"ASSIGN",
		"IO",
		"SWITCH",
		"LOOP",
		"REUSE_MODULE",
		"OPERATOR",
	};

	if(tree->next){
		// Printing linked list first
		int n = 0;
		while(tree){
			printf("List element %d: %d\t%s\t\t%s\t%d", n++, tree->line, symbols[tree->construct], tree->lexeme[0] ? tree->lexeme : "---", tree->totalChildren);
			for(int i = 0; i < tree->totalChildren; i++){
				// For each, printing sub tree
				printf("\nSubtree %d\n", i);
				recursivePrintAST(tree->children[i]);
			}
			printf("\n");
			tree = tree->next;
		}
	}else{
		// Pre-order, first this node, then children
		printf("%d\t%s\t\t%s\t%d\n", tree->line, symbols[tree->construct], tree->lexeme[0] ? tree->lexeme : "---", tree->totalChildren);
		for(int i = 0; i < tree->totalChildren; i++)
			recursivePrintAST(tree->children[i]);
	}
	return;

}

void freeAST(ast tree){

	for(int i = 0; i < tree->totalChildren; i++)
		freeAST(tree->children[i]);
	free(tree->children);
	if(tree->next && tree->construct != CONSTRUCT_DECLARE)
		freeAST(tree->next);

	type _type;

	switch(tree->construct){
		case CONSTRUCT_DECLARE:
			if(tree->type->output)
				free(tree->type->output);
			free(tree->type);
			break;

		case CONSTRUCT_OPERATOR:
			// Constructed type for relational operator, not in any symbol table
			if((tree->lexeme[0] == '>' || tree->lexeme[0] == '<' || tree->lexeme[0] == '!' || tree->lexeme[0] == '=') && tree->type)
				free(tree->type);
			break;

		case CONSTRUCT_MODULE:
			// Input
			_type = tree->type->input;
			while(_type){
				type temp = _type->input;
				if(_type->output)
					free(_type->output);
				free(_type);
				_type = temp;
			}

			// Output
			_type = tree->type->output;
			while(_type){
				type temp = _type->output;
				free(_type);
				_type = temp;
			}

			free(tree->type);
		case CONSTRUCT_PROGRAM:
		case CONSTRUCT_ENTRY:
		case CONSTRUCT_LOOP:
		case CONSTRUCT_SWITCH:
			// Clearing symbol tables only where new was created (hard-coded)
			for(int i = 0; i < SYMBOL_TABLE_ENTRIES; i++){
				tableEntry entry = tree->table->entries[i];
				while(entry){
					tableEntry temp = entry->next;
					free(entry);
					entry = temp;
				}
			}
			free(tree->table);
			break;
	}

	free(tree);
	return;

}
