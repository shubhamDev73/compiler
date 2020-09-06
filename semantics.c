#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "astDef.h"
#include "ast.h"
#include "symbolTable.h"

int compareTypes(type lhs, type rhs){

	// Deep compare, for array types too

	if(lhs && rhs)
		if(lhs->basic == rhs->basic){
			if(lhs->basic == TYPE_ARRAY || rhs->basic == TYPE_ARRAY){
				if(lhs->output && rhs->output){
					if(!lhs->startLexeme[0] && !rhs->startLexeme[0] && !lhs->endLexeme[0] && !rhs->endLexeme[0])
						return lhs->start == rhs->start && lhs->end == rhs->end;
					return lhs->output->basic == rhs->output->basic;
				}
			}else
				return 1;
		}
	return 0;

}

void assignUseType(ast tree, int useType){

	for(int i = 0; i < tree->totalChildren; i++)
		assignUseType(tree->children[i], useType);

	if(tree->construct == CONSTRUCT_VARIABLE && tree->entry){
		tree->entry->useType = useType;
		if(useType != USE_DEFAULT)
			// Resetting used
			tree->entry->used = 0;
	}

	return;

}

int checkUsed(ast tree){

	for(int i = 0; i < tree->totalChildren; i++)
		if(checkUsed(tree->children[i]))
			return 1;

	return tree->entry ? tree->entry->used : 0;

}

void semanticCheck(ast tree, ast root){

	// Assigning useType first, then traversing children, then checking useType
	if(tree->construct == CONSTRUCT_LOOP){
		if(tree->totalChildren == 2)
			assignUseType(tree->children[0], USE_WHILE);
		else
			assignUseType(tree->children[0], USE_FOR);
	}

	for(int i = 0; i < tree->totalChildren; i++)
		if(tree->construct != CONSTRUCT_SWITCH)
			semanticCheck(tree->children[i], root);

	type lhs, rhs;
	ast temp;
	int line = tree->line;

	switch(tree->construct){
		case CONSTRUCT_OPERATOR:
			lhs = tree->children[0]->type;
			if(!lhs)
				break;
			if(tree->totalChildren == 1){
				// Unary minus
				tree->type = lhs;
				if(lhs->basic != TYPE_INTEGER && lhs->basic != TYPE_REAL){
					printf("Line: %d, Error: Invalid type for operator '%s'. Expecting INTEGER or REAL\n", line, tree->lexeme);
					root->line++;
				}
				break;
			}

			rhs = tree->children[1]->type;
			if(!rhs)
				break;

			if(!compareTypes(lhs, rhs)){
				printf("Line: %d, Error: Incompatible types for operator '%s'\n", line, tree->lexeme);
				root->line++;
				break;
			}
			switch(tree->lexeme[0]){
				case 'A':
				case 'O':
					if(lhs->basic != TYPE_BOOLEAN){
						printf("Line: %d, Error: Invalid types for operator '%s'. Expecting BOOLEAN\n", line, tree->lexeme);
						root->line++;
					}
					break;
				case '<':
				case '>':
				case '!':
				case '=':
					tree->type = createType(TYPE_BOOLEAN);
				default:
					if(lhs->basic != TYPE_INTEGER && lhs->basic != TYPE_REAL){
						printf("Line: %d, Error: Invalid types for operator '%s'. Expecting INTEGER or REAL\n", line, tree->lexeme);
						root->line++;
					}
					break;
			}
			if(!tree->type)
				tree->type = lhs;
			break;

		case CONSTRUCT_WHICH_ID:
			lhs = tree->children[1]->type;
			if(!lhs)
				break;
			if(lhs->basic != TYPE_INTEGER){
				printf("Line: %d, Error: Invalid array access\n", line);
				root->line++;
				break;
			}

			rhs = tree->children[0]->type;
			if(!rhs)
				break;

			if(tree->children[1]->construct == CONSTRUCT_CONSTANT){
				int value = atoi(tree->children[1]->lexeme);
				if(!rhs->startLexeme[0] && !rhs->endLexeme[0] && (value < rhs->start || value > rhs->end)){
					printf("Line: %d, Error: Index out of bounds of array: %s\n", line, tree->children[0]->lexeme);
					root->line++;
					break;
				}
			}
			tree->type = rhs->output;
			break;

		case CONSTRUCT_ASSIGN:
			temp = tree->children[0];
			lhs = temp->type;
			if(!lhs)
				break;

			rhs = tree->children[1]->type;
			if(!rhs)
				break;

			if(!compareTypes(lhs, rhs)){
				printf("Line: %d, Error: Invalid assignment\n", line);
				root->line++;
				break;
			}

			if(temp->entry){
				if(temp->entry->useType == USE_FOR){
					printf("Line: %d, Error: Cannot assign to for loop variable: %s\n", line, temp->lexeme);
					root->line++;
				}else
					temp->entry->used = 1;
			}
			break;

		case CONSTRUCT_SWITCH:
			lhs = tree->children[0]->type;
			if(!lhs)
				break;
			if(lhs->basic == TYPE_REAL || lhs->basic == TYPE_ARRAY){
				printf("Line: %d, Error: Invalid switch variable type\n", line);
				root->line++;
				break;
			}

			temp = tree->children[1];
			line = 0;
			int * cases = (int *)malloc(sizeof(int) * line);
			while(temp){
				if(compareTypes(lhs, temp->children[0]->type)){
					int value = 0;
					switch(lhs->basic){
						case TYPE_BOOLEAN:
							value = temp->children[0]->lexeme[0] == 't';
							break;
						case TYPE_INTEGER:
							value = atoi(temp->children[0]->lexeme);
							break;
					}
					for(int i = 0; i < line; i++){
						if(cases[i] == value){
							printf("Line: %d, Error: Duplicate case\n", temp->children[0]->line);
							root->line++;
							break;
						}
					}
					semanticCheck(temp, root);
					cases = (int *)realloc(cases, sizeof(int) * (line + 1));
					cases[line++] = value;
				}else{
					printf("Line: %d, Error: Invalid case type\n", temp->children[0]->line);
					root->line++;
				}
				temp = temp->next;
			}
			free(cases);
			if(tree->totalChildren > 2){
				if(lhs->basic == TYPE_BOOLEAN){
					printf("Line: %d, Error: Default not possible\n", tree->children[2]->line);
					root->line++;
				}else
					semanticCheck(tree->children[2], root);
			}else if(lhs->basic == TYPE_INTEGER){
				printf("Line: %d, Error: Default expected\n", tree->line);
				root->line++;
			}
			break;

		case CONSTRUCT_LOOP:
			lhs = tree->children[0]->type;
			if(!lhs)
				break;
			switch(tree->totalChildren){
				case 2:
					// while
					if(!checkUsed(tree->children[0])){
						printf("Line: %d, Error: while loop condition not updating\n", tree->line);
						root->line++;
					}
					assignUseType(tree->children[0], USE_DEFAULT);
					if(lhs->basic != TYPE_BOOLEAN){
						printf("Line: %d, Error: Expecting BOOLEAN expression\n", line);
						root->line++;
					}
					break;
				case 4:
					// for
					assignUseType(tree->children[0], USE_DEFAULT);
					if(lhs->basic != TYPE_INTEGER){
						printf("Line: %d, Error: Expecting type INTEGER\n", line);
						root->line++;
					}
					break;
			}
			break;

		case CONSTRUCT_REUSE_MODULE:

			// Recursion
			if(!strcmp(tree->children[tree->totalChildren - 2]->lexeme, tree->table->id)){
				printf("Line: %d, Error: Recursion not allowed\n", line);
				root->line++;
				break;
			}

			temp = tree->children[tree->totalChildren - 2];
			lhs = temp->type;
			if(!lhs)
				break;

			if(lhs->basic == TYPE_MODULE_DECLARE){
				// Updating declaration to take definition
				lhs = getInSymbolTable(temp, temp->table)->type;
				if(lhs->basic == TYPE_MODULE_DECLARE){
					printf("Line: %d, Error: Module not defined: %s\n", line, temp->lexeme);
					root->line++;
					break;
				}
			}

			rhs = lhs;

			// Input
			temp = tree->children[tree->totalChildren - 1];
			while(temp && lhs->input){
				if(getInSymbolTable(temp, temp->table) && !compareTypes(lhs->input, temp->type)){
					printf("Line: %d, Error: Input parameter of wrong type: %s\n", line, temp->lexeme);
					root->line++;
				}
				temp = temp->next;
				lhs = lhs->input;
			}

			if(temp || lhs->input){
				// One linked list is not empty
				printf("Line: %d, Error: Invalid number of input parameters\n", line);
				root->line++;
			}

			// Output
			if(tree->totalChildren == 3){
				temp = tree->children[0];
				while(temp && rhs->output){
					if(getInSymbolTable(temp, temp->table)){
						if(!compareTypes(rhs->output, temp->type)){
							printf("Line: %d, Error: Output parameter of wrong type: %s\n", line, temp->lexeme);
							root->line++;
						}else if(temp->type && temp->entry){
							if(temp->entry->useType == USE_FOR){
								printf("Line: %d, Error: Cannot assign to for loop variable: %s\n", line, temp->lexeme);
								root->line++;
							}else
								temp->entry->used = 1;
						}
					}
					temp = temp->next;
					rhs = rhs->output;
				}

				if(temp || rhs->output){
					// One linked list is not empty
					printf("Line: %d, Error: Invalid number of output parameters\n", line);
					root->line++;
				}
			}else if(rhs->output){
				printf("Line: %d, Error: Invalid number of output parameters\n", line);
				root->line++;
			}
			break;

		case CONSTRUCT_IO:
			if(tree->lexeme[0] == 'g'){
				temp = tree->children[0];
				if(temp->type && temp->entry){
					if(temp->entry->useType == USE_FOR){
						printf("Line: %d, Error: Cannot assign to for loop variable: %s\n", line, temp->lexeme);
						root->line++;
					}else
						temp->entry->used = 1;
				}
			}
			break;

		case CONSTRUCT_MODULE:
			if(!getInSymbolTable(tree->children[0], tree->table->parent))
				break;
			tree->entry = tree->children[0]->entry;
			if(tree->totalChildren <= 3)
				break;

			// Output parameters
			temp = tree->children[2];
			while(temp){
				if(!temp->entry->used){
					printf("Line: %d, Error: Output variable not assigned: %s\n", tree->line, temp->lexeme);
					root->line++;
				}
				temp = temp->next;
			}
			break;
	}
	return;

}
