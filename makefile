all:
	gcc -c grammar.c lexer.c
	gcc grammar.o lexer.o main.c -o compiler
	rm grammar.o lexer.o
