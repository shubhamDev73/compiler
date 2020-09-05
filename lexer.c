#include <stdlib.h>
#include <string.h>

#include "lexerDef.h"
#include "grammar.h"

char buffer1[SIZE_BUFFER];
char buffer2[SIZE_BUFFER];
int bufferIndex;
int bufferNum;

token new;
int pos;
char string[SIZE_STRING];
int comment;
int line;

void openStream(const char * fileName){

	stream = fopen(fileName, "r");
	if(!stream){
		printf("Invalid source file. Exiting...\n");
		exit(1);
		return;
	}
	bufferNum = 1;
	bufferIndex = 0;
	fread(buffer1, sizeof(char), SIZE_BUFFER, stream);

	new = (token)malloc(sizeof(struct _token));
	strcpy(string, "");
	pos = 0;
	comment = 0;
	line = 1;

	return;

}

void closeStream(){

	free(new);
	fclose(stream);
	return;

}

char getChar(){

	char c;
	if(bufferNum)
		c = buffer1[bufferIndex++];
	else
		c = buffer2[bufferIndex++];

	if(bufferIndex >= SIZE_BUFFER){
		// Clearing current buffer and reading in other buffer
		bufferIndex = 0;
		if(bufferNum){
			memset(buffer1, 0, SIZE_BUFFER * sizeof(char));
			fread(buffer2, sizeof(char), SIZE_BUFFER, stream);
		}else{
			memset(buffer2, 0, SIZE_BUFFER * sizeof(char));
			fread(buffer1, sizeof(char), SIZE_BUFFER, stream);
		}
		bufferNum = 1 - bufferNum;
	}

	if(c == '\n')
		line++;
	return c;

}

token createToken(const char * terminal, grammar G){

	new->terminal = findSymbol(terminal, 0, 0, G);
	strcpy(new->lexeme, string);
	strcpy(string, "");
	pos = 0;
	new->line = line;
	return new;

}

token getNextToken(grammar G){

	char c;
	int dot, e, error;

	do{
		// If comment, keep on finding symbols till comment ends
		new->terminal = NULL;
		dot = 0;
		e = 0;
		error = 0;

		while(!new->terminal){
			// Keep on reading characters till terminal formed
			c = getChar();

			if(!strcmp("<<<", string))
				new = createToken("DRIVERDEF", G);
			else if(!strcmp(">>>", string))
				new = createToken("DRIVERENDDEF", G);
			else if(!strcmp("<=", string))
				new = createToken("LE", G);
			else if(!strcmp(">=", string))
				new = createToken("GE", G);
			else if(!strcmp("==", string))
				new = createToken("EQ", G);
			else if(!strcmp("!=", string))
				new = createToken("NE", G);
			else if(!strcmp("<<", string) && c != '<')
				new = createToken("DEF", G);
			else if(!strcmp(">>", string) && c != '>')
				new = createToken("ENDDEF", G);
			else if(!strcmp("..", string))
				new = createToken("RANGEOP", G);
			else if(!strcmp(":=", string))
				new = createToken("ASSIGNOP", G);
			else if(!strcmp("**", string)){
				new = createToken("COMMENTMARK", G);
				comment = 1 - comment;
			}

			// Starts with a digit
			if(pos > 0 && ((string[0] >= '0' && string[0] <= '9') || string[0] == '.')){
				// Starts with .
				if(string[0] == '.'){
					if(c != '.'){
						error = 1;
					}
				}else if(c < '0' || c > '9'){
					// Non digit characterr
					if(c == '.' && !e){
						if(!dot){
							dot = 1;
						}else if(string[pos - 1] == '.'){
							// NUM.. => .. is RANGEOP
							if(pos <= SIZE_ID || comment){
								string[pos - 1] = '\0';
		 						new = createToken("NUM", G);
								string[0] = '.';
								pos = 1;
							}else{
								error = 1;
							}
						}else if(pos <= SIZE_ID || comment){
	 						new = createToken("RNUM", G);
						}else{
							error = 1;
						}
					}else if((c == 'e' || c == 'E') && !e && string[pos - 1] >= '0' && string[pos - 1] <= '9')
						// e/E
						e = 1;
					else if(e == 1 && (c == '+' || c == '-') && (string[pos - 1] == 'e' || string[pos - 1] == 'E'))
						// E+/-
						e = 2;
 					else if(e || dot){
 						if(string[pos - 1] >= '0' && string[pos - 1] <= '9' && (pos <= SIZE_ID || comment))
 							new = createToken("RNUM", G);
 						else
 							// 12.0E+ is error
 							error = 1;
					}else if(pos <= SIZE_ID || comment)
 						new = createToken("NUM", G);
					else
						error = 1;
				}
			}else if(!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') || !c || c == EOF){
				// Non alphabet (stored previous alphabets in string)

				if(!strcmp("integer", string))
					new = createToken("INTEGER", G);
				else if(!strcmp("real", string))
					new = createToken("REAL", G);
				else if(!strcmp("boolean", string))
					new = createToken("BOOLEAN", G);
				else if(!strcmp("of", string))
					new = createToken("OF", G);
				else if(!strcmp("array", string))
					new = createToken("ARRAY", G);
				else if(!strcmp("start", string))
					new = createToken("START", G);
				else if(!strcmp("end", string))
					new = createToken("END", G);
				else if(!strcmp("declare", string))
					new = createToken("DECLARE", G);
				else if(!strcmp("module", string))
					new = createToken("MODULE", G);
				else if(!strcmp("driver", string))
					new = createToken("DRIVER", G);
				else if(!strcmp("program", string))
					new = createToken("PROGRAM", G);
				else if(!strcmp("get_value", string))
					new = createToken("GET_VALUE", G);
				else if(!strcmp("print", string))
					new = createToken("PRINT", G);
				else if(!strcmp("use", string))
					new = createToken("USE", G);
				else if(!strcmp("with", string))
					new = createToken("WITH", G);
				else if(!strcmp("parameters", string))
					new = createToken("PARAMETERS", G);
				else if(!strcmp("true", string))
					new = createToken("TRUE", G);
				else if(!strcmp("false", string))
					new = createToken("FALSE", G);
				else if(!strcmp("takes", string))
					new = createToken("TAKES", G);
				else if(!strcmp("input", string))
					new = createToken("INPUT", G);
				else if(!strcmp("returns", string))
					new = createToken("RETURNS", G);
				else if(!strcmp("AND", string))
					new = createToken("AND", G);
				else if(!strcmp("OR", string))
					new = createToken("OR", G);
				else if(!strcmp("for", string))
					new = createToken("FOR", G);
				else if(!strcmp("in", string))
					new = createToken("IN", G);
				else if(!strcmp("switch", string))
					new = createToken("SWITCH", G);
				else if(!strcmp("case", string))
					new = createToken("CASE", G);
				else if(!strcmp("break", string))
					new = createToken("BREAK", G);
				else if(!strcmp("default", string))
					new = createToken("DEFAULT", G);
				else if(!strcmp("while", string))
					new = createToken("WHILE", G);

				else if(pos > 0 && ((string[0] >= 'a' && string[0] <= 'z') || (string[0] >= 'A' && string[0] <= 'Z') || string[0] == '_')){
					if(pos <= SIZE_ID || comment)
						new = createToken("ID", G);
					else
						error = 1;
				}
			}

			// Small tokens
			if(string[0] == '+')
				new = createToken("PLUS", G);
			else if(string[0] == '-')
				new = createToken("MINUS", G);
			else if(string[0] == '*' && c != '*')
				new = createToken("MUL", G);
			else if(string[0] == '/')
				new = createToken("DIV", G);
			else if(string[0] == '<' && c != '<' && c != '=')
				new = createToken("LT", G);
			else if(string[0] == '>' && c != '>' && c != '=')
				new = createToken("GT", G);
			else if(string[0] == ':' && c != '=')
				new = createToken("COLON", G);
			else if(string[0] == ';')
				new = createToken("SEMICOL", G);
			else if(string[0] == ',')
				new = createToken("COMMA", G);
			else if(string[0] == '[')
				new = createToken("SQBO", G);
			else if(string[0] == ']')
				new = createToken("SQBC", G);
			else if(string[0] == ')')
				new = createToken("BC", G);
			else if(string[0] == '(')
				new = createToken("BO", G);
			// Lexical errors
			else if(string[0] == '=' && c != '=')
				error = 1;
			else if(string[0] == '!' && c != '=')
				error = 1;
			else if(string[0] == '`')
				error = 1;
			else if(string[0] == '~')
				error = 1;
			else if(string[0] == '@')
				error = 1;
			else if(string[0] == '#')
				error = 1;
			else if(string[0] == '$')
				error = 1;
			else if(string[0] == '%')
				error = 1;
			else if(string[0] == '^')
				error = 1;
			else if(string[0] == '&')
				error = 1;
			else if(string[0] == '?')
				error = 1;
			else if(string[0] == '\'')
				error = 1;
			else if(string[0] == '|')
				error = 1;
			else if(string[0] == '\\')
				error = 1;
			else if(string[0] == '"')
				error = 1;

			if(error){
				if(!comment)
					printf("Line %d, Lexical error: %s\n", line, string);
				strcpy(string, "");
				pos = 0;
				dot = 0;
				e = 0;
				error = 0;
			}

			if(!new->terminal && (!c || c == EOF))
				return NULL;

			// Skipping blanks
			if(c != ' ' && c != '\t' && c != '\n' && c != '\r'){
				if(pos >= SIZE_STRING - 1)
					pos = SIZE_STRING - 2;
				string[pos++] = c;
				string[pos] = '\0';
			}
		}
	}while(comment || new->terminal == findSymbol("COMMENTMARK", 0, 0, G));

	new->line = (c == '\n' ? line - 1 : line);
	return new;

}
