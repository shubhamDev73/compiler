all:
	gcc -c grammar.c lexer.c parser.c
	gcc grammar.o lexer.o parser.o main.c -o compiler
	rm grammar.o lexer.o parser.o
