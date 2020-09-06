#include "languageDef.h"

#ifndef _codegenDef
#define _codegenDef

#define REGISTERS 4

// have to be negative to seperate from others
#define ESP -1
#define EBP -2
#define EIP -3

#define OPERAND_VARIABLE 0
#define OPERAND_REGISTER 1
#define OPERAND_CONSTANT 2

#define OPERATOR_DEFAULT 0
#define OPERATOR_ASSIGN 1
#define OPERATOR_PLUS 2
#define OPERATOR_MINUS 3
#define OPERATOR_UNARY_MINUS 4
#define OPERATOR_INC 5
#define OPERATOR_DEC 6
#define OPERATOR_MUL 7
#define OPERATOR_DIV 8
#define OPERATOR_GE 9
#define OPERATOR_GT 10
#define OPERATOR_EQ 11
#define OPERATOR_NE 12
#define OPERATOR_LT 13
#define OPERATOR_LE 14
#define OPERATOR_JMP 15
#define OPERATOR_PRINT_MSG 16
#define OPERATOR_PRINT 17
#define OPERATOR_GET_VALUE_MSG 18
#define OPERATOR_GET_VALUE 19
#define OPERATOR_PUSH 20
#define OPERATOR_POP 21
#define OPERATOR_CALL 22
#define OPERATOR_RETURN 23
#define OPERATOR_END 24

typedef struct _quadruple * quadruple;
struct _quadruple{
	quadruple next;
	int operator;
	int op1, op2;
	int type1, type2;
	int size1, size2;
	int memory1, memory2;
	int label;
};

typedef struct _code * code;
struct _code{
	quadruple start;
	int var;
	int type;
	int size;
	int memory;
};

#endif
