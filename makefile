all:
	gcc -c grammar.c lexer.c parser.c ast.c symbolTable.c semantics.c codegen.c
	gcc grammar.o lexer.o parser.o ast.o symbolTable.o semantics.o codegen.o main.c -o compiler
	rm grammar.o lexer.o parser.o ast.o symbolTable.o semantics.o codegen.o
