#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "astDef.h"
#include "ast.h"

void intToStr(int n, char * str){

	int i = 0;
	if(n < 0)
		n = -n;
	else if(n == 0)
		str[i++] = '0';
	while(n > 0){
		for(int j = i++; j > 0; j--){
			str[j] = str[j - 1];
		}
		str[0] = (n % 10) + '0';
		n /= 10;
	}
	str[i] = '\0';
	return;

}

int hash(char * id){


	// djb2 algorithm

	unsigned long hash = 5381;
	int c;

	while((c = *id++))
		hash = ((hash << 5) + hash) + c; // hash * 33 + c

	return hash % SYMBOL_TABLE_ENTRIES;

}

tableEntry getInSymbolTable(ast tree, symTable table){

	// Gets tree->lexeme in symbol table (ancestor too), and assigns entry and table to it
	// Neccessary if scope is different from current scope

	int index = hash(tree->lexeme);

	while(table){
		tableEntry entry =  table->entries[index];
		while(entry){
			if(!strcmp(entry->id, tree->lexeme)){
				// Found
				tree->entry = entry;
				tree->type = entry->type;
				tree->table = table;
				return entry;
			}
			entry = entry->next;
		}
		table = table->parent;
	}

	return NULL;

}

tableEntry getIdInSymbolTable(char * id, symTable table){

	// More general version, used for array lexemes

	int index = hash(id);

	while(table){
		tableEntry entry =  table->entries[index];
		while(entry){
			if(!strcmp(entry->id, id))
				return entry;
			entry = entry->next;
		}
		table = table->parent;
	}

	return NULL;

}

tableEntry insertInSymbolTable(char * id, type _type, int line, symTable table, int useType, ast root){

	// Inserts id in given symbol table, and reports error if already present

	int index = hash(id);
	tableEntry entry =  table->entries[index];

	// Finding where to insert
	tableEntry insert = entry;
	while(entry){
		if(!strcmp(entry->id, id))
			break;
		if(entry->next)
			insert = entry->next;
		entry = entry->next;
	}

	// Input parameter can be shadowed
	if(entry && entry->useType != USE_INPUT){
		if(entry->type->basic == TYPE_MODULE_DECLARE){
			// Replacing module declaration by definition
			entry->type = _type;
			if(!entry->used){
				printf("Line: %d, Error: Module declaration invalid: %s\n", line, id);
				root->line++;
			}
		}else{
			printf("Line: %d, Error: Identifier already declared: %s\n", line, id);
			root->line++;
		}
		return NULL;
	}

	tableEntry new = (tableEntry)malloc(sizeof(struct _tableEntry));
	new->next = NULL;
	strcpy(new->id, id);
	new->type = _type;
	new->useType = useType;
	new->used = 0;

	// Calculating width
	new->width = WIDTHS[_type->basic];
	if(_type->basic == TYPE_ARRAY && useType != USE_INPUT && !_type->startLexeme[0] && !_type->endLexeme[0])
		new->width += (1 + _type->end - _type->start) * WIDTHS[_type->output->basic];

	// Inserting
	if(insert){
		if(entry && entry->useType == USE_INPUT){
			insert = table->entries[index];
			table->entries[index] = new;
			new->next = insert;
		}else
			insert->next = new;
	}else
		table->entries[index] = new;

	// Updating table width
	// Input and output parameters don't change width
	if(useType == USE_DEFAULT){
		if(!table->parent)
			table->width += new->width;

		while(table->parent){
			// For all ancestor scopes (except ROOT)
			table->width += new->width;
			if(!table->parent->parent)
				break;
			table = table->parent;
		}

		// Calculating offset
		if(_type->basic == TYPE_ARRAY)
			new->offset = table->width - new->width + WIDTHS[TYPE_ARRAY];
		else
			new->offset = table->width;
	}
	return new;

}

void createSymbolTable(ast tree, ast root){

	ast temp;

	switch(tree->construct){
		case CONSTRUCT_VARIABLE:
			if(!getInSymbolTable(tree, tree->table)){
				printf("Line: %d, Error: Undeclared identifier: %s\n", tree->line, tree->lexeme);
				root->line++;
			}
			break;

		case CONSTRUCT_DECLARE:
			temp = tree;
			while(temp){
				if(temp->type->basic == TYPE_ARRAY){
					if(temp->type->startLexeme[0] && !getIdInSymbolTable(temp->type->startLexeme, temp->table)){
						printf("Line: %d, Error: Undeclared identifier: %s\n", temp->line, temp->type->startLexeme);
						root->line++;
					}
					if(temp->type->endLexeme[0] &&!getIdInSymbolTable(temp->type->endLexeme, temp->table)){
						printf("Line: %d, Error: Undeclared identifier: %s\n", temp->line, temp->type->endLexeme);
						root->line++;
					}
					if(!temp->type->startLexeme[0] && !temp->type->endLexeme[0] && temp->type->start > temp->type->end){
						printf("Line: %d, Error: Array range invalid\n", temp->line);
						root->line++;
					}
				}
				temp->entry = insertInSymbolTable(temp->lexeme, temp->type, temp->line, temp->table, USE_DEFAULT, root);
				temp = temp->next;
			}
			break;

		case CONSTRUCT_MODULE:
			// Lexeme available with children[0] but type with current node
			tree->children[0]->type = tree->type;
			strcpy(tree->table->id, tree->children[0]->lexeme);
			tree->children[0]->entry = insertInSymbolTable(tree->children[0]->lexeme, tree->type, tree->line, tree->table->parent, USE_DEFAULT, root);

			// Input parameters
			temp = tree->children[1];
			int offset = - 2 * WIDTHS[TYPE_ARRAY]; // system constant (1 address for EIP, 1 address for EBP)
			while(temp){
				temp->entry = insertInSymbolTable(temp->lexeme, temp->type, temp->line, tree->table, USE_INPUT, root);
				temp->entry->offset = offset;
				offset -= temp->entry->width;

				// Assigning correct offset for array range
				if(temp->type->basic == TYPE_ARRAY){
					if(temp->type->startLexeme[0])
						insertInSymbolTable(temp->type->startLexeme, createType(TYPE_INTEGER), temp->line, tree->table, USE_INPUT, root)->offset = offset;
					offset -= WIDTHS[TYPE_INTEGER];
					if(temp->type->endLexeme[0])
						insertInSymbolTable(temp->type->endLexeme, createType(TYPE_INTEGER), temp->line, tree->table, USE_INPUT, root)->offset = offset;
					offset -= WIDTHS[TYPE_INTEGER];
				}

				temp = temp->next;
			}

			// Output parameters
			if(tree->totalChildren == 4){
				temp = tree->children[2];
				while(temp){
					temp->entry = insertInSymbolTable(temp->lexeme, temp->type, temp->line, tree->table, USE_OUTPUT, root);
					temp->entry->offset = offset;
					offset -= WIDTHS[temp->type->basic];
					temp = temp->next;
				}
			}
			break;

		case CONSTRUCT_LOOP:
		case CONSTRUCT_SWITCH:
			strcpy(tree->table->id, tree->table->parent->id);
			break;
	}

	for(int i = 0; i < tree->totalChildren; i++)
		createSymbolTable(tree->children[i], root);

	if(tree->next && tree->construct != CONSTRUCT_DECLARE)
		createSymbolTable(tree->next, root);

	// Assigning used for a module declaration
	if(tree->construct == CONSTRUCT_REUSE_MODULE){
		temp = tree->children[tree->totalChildren - 2];
		if(!temp->type)
			return;
		if(temp->type->basic == TYPE_MODULE_DECLARE){
			tableEntry entry = getInSymbolTable(temp, temp->table);
			if(!entry)
				return;
			entry->used = 1;
		}
	}

	return;

}
