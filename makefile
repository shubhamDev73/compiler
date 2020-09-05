all:
	gcc -c grammar.c
	gcc grammar.o main.c -o compiler
	rm grammar.o
