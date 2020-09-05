#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "grammarDef.h"

grammar createEmptyGrammar(){

	grammar G = (grammar)malloc(sizeof(struct _grammar));
	G->start = NULL;
	G->null = NULL;
	G->rules = NULL;
	G->terminals = NULL;
	G->non_terminals = NULL;
	G->terminalsSize = 0;
	G->non_terminalsSize = 0;
	return G;

}

symbol createSymbol(const char * data, int is_non_terminal){

	symbol s = (symbol)malloc(sizeof(struct _symbol));
	strcpy(s->data, data);
	s->is_non_terminal = is_non_terminal;
	return s;

}

rule createEmptyRule(){

	rule new = (rule)malloc(sizeof(struct _rule));
	new->next = NULL;
	new->lhs = NULL;
	new->rhsSize = 0;
	new->rhs = NULL;
	return new;

}

symbol findSymbol(const char * c, int start, int end, grammar G){

	if(start == end)
		end = start + strlen(c);

	// Finding in terminals
	for(int i = 0; i < G->terminalsSize; i++){
		int j, flag = 0;
		for(j = start; j < end; j++){
			if(G->terminals[i]->data[j - start] != c[j]){
				flag = 1;
				break;
			}
		}
		if(!flag && G->terminals[i]->data[j - start] == '\0')
			return G->terminals[i];
	}

	// Finding in non-terminals
	for(int i = 0; i < G->non_terminalsSize; i++){
		int j, flag = 0;
		for(j = start; j < end; j++){
			if(G->non_terminals[i]->data[j - start] != c[j]){
				flag = 1;
				break;
			}
		}
		if(!flag && G->non_terminals[i]->data[j - start] == '\0')
			return G->non_terminals[i];
	}

	return NULL;

}

grammar readGrammar(const char * grammarFile){

	const char * terminals[] = {"e", "INTEGER", "REAL", "BOOLEAN", "OF", "ARRAY", "START",
		"END", "DECLARE", "MODULE", "DRIVER", "PROGRAM", "GET_VALUE", "PRINT", "USE", "WITH",
		"PARAMETERS", "TRUE", "FALSE", "TAKES", "INPUT", "RETURNS", "AND", "OR", "FOR", "IN",
		"SWITCH", "CASE", "BREAK", "DEFAULT", "WHILE", "PLUS", "MINUS", "MUL", "DIV", "LT",
		"LE", "GE", "GT", "EQ", "NE", "DEF", "ENDDEF", "COLON", "RANGEOP", "SEMICOL", "COMMA",
		"ASSIGNOP", "SQBO", "SQBC", "BO", "BC", "COMMENTMARK", "ID", "NUM", "RNUM", "DRIVERDEF",
		"DRIVERENDDEF"};
	const char * non_terminals[] = {"<program>", "<moduleDeclarations>", "<moduleDeclaration>",
		"<otherModules>", "<driverModule>", "<module>", "<ret>", "<input_plist>", "<N1>", "<output_plist>",
		"<N2>", "<dataType>", "<range_arrays>", "<type>", "<moduleDef>", "<statements>", "<statement>", "<id>",
		"<ioStmt>", "<boolConstt>", "<var_id_num>", "<var>", "<whichId>", "<simpleStmt>", "<assignmentStmt>",
		"<index>", "<moduleReuseStmt>", "<optional>", "<idList>", "<N3>", "<arithmeticOrBooleanExpr>", "<N7>",
		"<AnyTerm>", "<N8>", "<arithmeticExpr>", "<N4>", "<term>", "<N5>", "<factor>", "<N6>", "<op1>", "<op2>",
		"<logicalOp>", "<relationalOp>", "<declareStmt>", "<conditionalStmt>", "<caseStmts>", "<N9>", "<value>",
		"<default>", "<iterativeStmt>", "<range>", "<array_id>", "<num>"};
	int terminalsSize = sizeof(terminals) / sizeof(char *);
	int non_terminalsSize = sizeof(non_terminals) / sizeof(char *);

	grammar G = createEmptyGrammar();
	G->terminalsSize = terminalsSize;
	G->non_terminalsSize = non_terminalsSize;

	char lines[128][128];
	int lineNum = 0, linePos = 0;

	FILE * fp = fopen(grammarFile, "r");
	if(!fp){
		printf("Invalid grammar file. Exiting...\n");
		exit(1);
		return NULL;
	}

	// Storing in memory
	while(!feof(fp)){
		char c;
		while((c = fgetc(fp)) != EOF && c != '\n' && c != '\r')
			lines[lineNum][linePos++] = c;
		if(c == '\n'){
			lines[lineNum++][linePos] = '\0';
			linePos = 0;
		}
	}
	fclose(fp);

	// Creating terminals
	G->terminals = (symbol *)malloc(sizeof(symbol) * (terminalsSize));
	for(int i = 0; i < terminalsSize; i++)
		G->terminals[i] = createSymbol(terminals[i], 0);

	// Creating non-terminals
	G->non_terminals = (symbol *)malloc(sizeof(symbol) * non_terminalsSize);
	for(int i = 0; i < non_terminalsSize; i++)
		G->non_terminals[i] = createSymbol(non_terminals[i], 1);

	G->start = G->non_terminals[0];
	G->null = G->terminals[0];

	G->rules = createEmptyRule();
	rule prev = G->rules;

	// Creating rules
	for(int i = 0; i < lineNum; i++){
		int start = 0, end = 0;

		// Skipping blanks
		while(lines[i][end] != '\0' && (lines[i][end] == ' ' || lines[i][end] == '\t'))
			end++;

		start = end;

		// Finding blank
		while(lines[i][end] != '\0' && lines[i][end] != ' ' && lines[i][end] != '\t')
			end++;

		if(start != end){
			// LHS
			rule new;
			if(G->rules->next)
				new = createEmptyRule();
			else
				new = G->rules;

			new->lhs = findSymbol(lines[i], start, end, G);
			if(!new->lhs){
				printf("Invalid symbol used in grammar. Could not find: ");
				for(int j = start; j < end; j++)
					putchar(lines[i][j]);
				printf("\nLine number: %d\nExiting...\n", i +1);
				exit(1);
				return NULL;
			}

			while(lines[i][end] != ':')
				end++;
			end++;

			// Counting RHS symbols
			int num = 0;
			start = end;
			while(lines[i][end] != '\0'){
				// Skipping blanks
				while(lines[i][end] != '\0' && (lines[i][end] == ' ' || lines[i][end] == '\t'))
					end++;
				// Finding blank
				while(lines[i][end] != '\0' && lines[i][end] != ' ' && lines[i][end] != '\t')
					end++;
				num++;
			}
			new->rhs = (symbol *)malloc(sizeof(symbol) * num);
			new->rhsSize = num;

			end = start;
			for(int j = 0; j < num; j++){
				// Skipping blanks
				while(lines[i][end] != '\0' && (lines[i][end] == ' ' || lines[i][end] == '\t'))
					end++;

				start = end;

				// Finding blank
				while(lines[i][end] != '\0' && lines[i][end] != ' ' && lines[i][end] != '\t')
					end++;

				if(start != end){
					// RHS
					new->rhs[j] = findSymbol(lines[i], start, end, G);
					if(!new->rhs[j]){
						printf("Invalid symbol used in grammar. Could not find: ");
						for(int k = start; k < end; k++)
							putchar(lines[i][k]);
						printf("\nLine number: %d\nExiting...\n", i + 1);
						exit(1);
						return NULL;
					}
				}
			}
			prev->next = new;
			prev = new;
		}
	}

	return G;

}

void freeGrammar(grammar G){

	while(G->rules){
		rule temp = G->rules->next;
		free(G->rules->rhs);
		free(G->rules);
		G->rules = temp;
	}
	for(int i = 0; i < G->terminalsSize; i++)
		free(G->terminals[i]);
	for(int i = 0; i < G->non_terminalsSize; i++)
		free(G->non_terminals[i]);
	free(G->terminals);
	free(G->non_terminals);
	free(G);
	return;

}
