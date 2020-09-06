all:
	gcc -c grammar.c lexer.c parser.c ast.c symbolTable.c
	gcc grammar.o lexer.o parser.o ast.o symbolTable.o main.c -o compiler
	rm grammar.o lexer.o parser.o ast.o symbolTable.o
