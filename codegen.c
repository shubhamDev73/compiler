#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "codegenDef.h"
#include "symbolTable.h"

int var;
int label;

void newCode(){

	// Resetting temporaries and labels
	var = REGISTERS + 1;
	label = REGISTERS + 1;
	return;

}

int getVar(){
	return ++var;
}

int getLabel(){
	return ++label;
}

void outputCode(code _code, const char * outFile){

	FILE * fp = NULL;
	fp = fopen(outFile, "w");

	if(!fp)
		fp = fopen("code.asm", "w");

	quadruple q = _code->start;

	char registers_dword[REGISTERS][4] = {"EAX", "EBX", "ECX", "EDX"};
	char registers_word[REGISTERS][4] = {"AX", "BX", "CX", "DX"};
	char registers_byte[REGISTERS][4] = {"AL", "BL", "CL", "DL"};

	char pointers[4][4] = {"", "ESP", "EBP", "EIP"};

	// Constants
	fprintf(fp, "section .data\n");

	fprintf(fp, "input db 'Input: Enter '\n");
	fprintf(fp, "input_len equ $ - input\n");

	fprintf(fp, "boolean db 'boolean '\n");
	fprintf(fp, "boolean_len equ $ - boolean\n");

	fprintf(fp, "true db 'true'\n");
	fprintf(fp, "true_len equ $ - true\n");
	fprintf(fp, "false db 'false'\n");
	fprintf(fp, "false_len equ $ - false\n");

	fprintf(fp, "integer db 'integer '\n");
	fprintf(fp, "integer_len equ $ - integer\n");

	fprintf(fp, "real db 'real '\n");
	fprintf(fp, "real_len equ $ - real\n");

	fprintf(fp, "value db 'value'\n");
	fprintf(fp, "value_len equ $ - value\n");

	fprintf(fp, "array db ' array elements of '\n");
	fprintf(fp, "array_len equ $ - array\n");
	fprintf(fp, "array_range db 'type for range '\n");
	fprintf(fp, "array_range_len equ $ - array_range\n");
	fprintf(fp, "array_to db ' to '\n");
	fprintf(fp, "array_to_len equ $ - array_to\n");

	fprintf(fp, "output db 'Output: '\n");
	fprintf(fp, "output_len equ $ - output\n");

	fprintf(fp, "error db 'RUN TIME ERROR: '\n");
	fprintf(fp, "error_len equ $ - error\n");

	fprintf(fp, "bound_error db 'Array index out of bound', 0AH\n");
	fprintf(fp, "bound_error_len equ $ - bound_error\n");

	fprintf(fp, "range_error db 'Array range invalid', 0AH\n");
	fprintf(fp, "range_error_len equ $ - range_error\n");

	fprintf(fp, "bool_error db 'Invalid boolean value', 0AH\n");
	fprintf(fp, "bool_error_len equ $ - bool_error\n");

	fprintf(fp, "temp_len equ 10\n");
	fprintf(fp, "\n");
	fprintf(fp, "section .bss\n");
	fprintf(fp, "temp resb temp_len\n");
	fprintf(fp, "\n");
	fprintf(fp, "section .text\n");
	fprintf(fp, "global _start\n");

	// 0 is EAX, 1 is EBX,...
	int operands[REGISTERS] = {0};

	while(q){
		// Assigning register to temporary

		if(q->op1 > 0 && q->type1 == OPERAND_REGISTER){
			if(q->memory1){
				// Free register
				for(int i = 0; i < REGISTERS; i++){
					if(operands[i] == q->op1){
						q->op1 = i;
						operands[i] = 0;
						break;
					}
				}
			}else{
				// Assign register
				int free = REGISTERS, i;
				for(i = 0; i < REGISTERS; i++){
					if(free == REGISTERS && !operands[i])
						free = i;
					if(operands[i] == q->op1)
						break;
				}
				if(i == REGISTERS){
					if(free == REGISTERS)
						// Register overflow
						free = 0;
					operands[free] = q->op1;
					q->op1 = free;
				}else{
					q->op1 = i;
				}
			}
		}

		// Get register
		if(q->op2 > 0 && q->type2 == OPERAND_REGISTER){
			for(int i = 0; i < REGISTERS; i++){
				if(operands[i] == q->op2){
					q->op2 = i;
					operands[i] = 0;
					break;
				}
			}
		}

		// Output to file
		if(q->operator == OPERATOR_DEFAULT){
			// Labels
			// Labels for comparison operators imply jump instruction
			if(q->label == -1)
				fprintf(fp, "_start:\n");
			else if(q->label > 0)
				fprintf(fp, "%s%d:\n", q->type1 ? "_m" : "s", q->label);
		}else{
			char op1[12];
			char size1[12];

			char op2[12];
			char size2[12];

			char temp[12];

			// Operand 1

			switch(q->size1){
				case 1:
					strcpy(size1, "BYTE ");
					break;
				case 2:
					strcpy(size1, "WORD ");
					break;
				case 4:
					strcpy(size1, "DWORD ");
					break;
				default:
					strcpy(size1, "");
					break;
			}

			if(q->type1 == OPERAND_REGISTER && q->op1 < 0)
				strcpy(op1, pointers[-q->op1]); // Special registers
			else{
				intToStr(q->op1, temp);
				strcpy(op1, temp);

				if(q->type1 == OPERAND_REGISTER){
					if(q->memory1)
						strcpy(op1, registers_dword[q->op1]); // Memory address is always 32 bit
					else{
						switch(q->size1){
							case 1:
								strcpy(op1, registers_byte[q->op1]);
								break;
							case 2:
								strcpy(op1, registers_word[q->op1]);
								break;
							case 4:
							default:
								strcpy(op1, registers_dword[q->op1]);
								break;
						}
					}
				}

				if(q->type1 == OPERAND_VARIABLE){
					// Variable offset is always w.r.t. base pointer
					strcpy(temp, "EBP");
					if(q->op1 > 0)
						strcat(temp, " - ");
					else
						strcat(temp, " + ");
					strcat(temp, op1);
					strcpy(op1, temp);
				}

				if(q->memory1){
					// Memory notation
					strcpy(temp, "[");
					strcat(temp, op1);
					strcpy(op1, temp);
					strcat(op1, "]");
				}
			}

			// Operand 2

			switch(q->size2){
				case 1:
					strcpy(size2, "BYTE ");
					break;
				case 2:
					strcpy(size2, "WORD ");
					break;
				case 4:
					strcpy(size2, "DWORD ");
					break;
				default:
					strcpy(size2, "");
					break;
			}
			if(q->type2 == OPERAND_REGISTER && q->op2 < 0)
				strcpy(op2, pointers[-q->op2]); // Special registers
			else{
				intToStr(q->op2, temp);
				strcpy(op2, temp);

				if(q->type2 == OPERAND_REGISTER){
					if(q->memory2)
						strcpy(op2, registers_dword[q->op2]); // Memory address is always 32 bit
					else{
						switch(q->size2){
							case 1:
								strcpy(op2, registers_byte[q->op2]);
								break;
							case 2:
								strcpy(op2, registers_word[q->op2]);
								break;
							case 4:
							default:
								strcpy(op2, registers_dword[q->op2]);
								break;
						}
					}
				}

				if(q->type2 == OPERAND_VARIABLE){
					// Variable offset is always w.r.t. base pointer
					strcpy(temp, "EBP");
					if(q->op2 > 0)
						strcat(temp, " - ");
					else
						strcat(temp, " + ");
					strcat(temp, op2);
					strcpy(op2, temp);
				}

				if(q->memory2){
					// Memory notation
					strcpy(temp, "[");
					strcat(temp, op2);
					strcpy(op2, temp);
					strcat(op2, "]");
				}
			}

			switch(q->operator){
				case OPERATOR_ASSIGN:
					fprintf(fp, "MOV %s%s, %s%s\n", size1, op1, size2, op2);
					break;

				case OPERATOR_PLUS:
					fprintf(fp, "ADD %s%s, %s%s\n", size1, op1, size2, op2);
					break;

				case OPERATOR_MINUS:
					fprintf(fp, "SUB %s%s, %s%s\n", size1, op1, size2, op2);
					break;

				case OPERATOR_UNARY_MINUS:
					fprintf(fp, "NOT %s%s\n", size1, op1);
					fprintf(fp, "ADD %s%s, 1\n", size1, op1);
					break;

				case OPERATOR_INC:
					fprintf(fp, "INC %s%s\n", size1, op1);
					break;

				case OPERATOR_DEC:
					fprintf(fp, "DEC %s%s\n", size1, op1);
					break;

				case OPERATOR_MUL:
					fprintf(fp, "IMUL %s%s, %s%s\n", size1, op1, size2, op2);
					break;

				case OPERATOR_DIV:
					// EAX can only be divided, hence if used, store it
					if(q->op1 != 0){
						fprintf(fp, "MOV EDI, EAX\n");
						switch(q->size1){
							case 2:
								strcpy(temp, registers_word[0]);
								if(q->op2 == 0)
									strcpy(op2, "DI");
								break;
							case 4:
							default:
								strcpy(temp, registers_dword[0]);
								if(q->op2 == 0)
									strcpy(op2, "EDI");
								break;
						}
						fprintf(fp, "MOV %s, %s%s\n", temp, size1, op1);
					}

					// EDX has to be zero, hence if used, store it
					if(operands[3])
						fprintf(fp, "MOV ESI, EDX\n");

					// Actual operation
					fprintf(fp, "XOR EDX, EDX\n");
					fprintf(fp, "IDIV %s%s\n", size2, op2);

					// Restoring stored registers
					if(operands[3])
						fprintf(fp, "MOV EDX, ESI\n");
					if(q->op1 != 0){
						fprintf(fp, "MOV %s%s, %s\n", size1, op1, temp);
						fprintf(fp, "MOV EAX, EDI\n");
					}
					break;

				case OPERATOR_GE:
					fprintf(fp, "CMP %s%s, %s%s\n", size1, op1, size2, op2);
					fprintf(fp, "JGE s%d\n", q->label);
					break;

				case OPERATOR_GT:
					fprintf(fp, "CMP %s%s, %s%s\n", size1, op1, size2, op2);
					fprintf(fp, "JG s%d\n", q->label);
					break;

				case OPERATOR_EQ:
					fprintf(fp, "CMP %s%s, %s%s\n", size1, op1, size2, op2);
					fprintf(fp, "JE s%d\n", q->label);
					break;

				case OPERATOR_NE:
					fprintf(fp, "CMP %s%s, %s%s\n", size1, op1, size2, op2);
					fprintf(fp, "JNE s%d\n", q->label);
					break;

				case OPERATOR_LT:
					fprintf(fp, "CMP %s%s, %s%s\n", size1, op1, size2, op2);
					fprintf(fp, "JL s%d\n", q->label);
					break;

				case OPERATOR_LE:
					fprintf(fp, "CMP %s%s, %s%s\n", size1, op1, size2, op2);
					fprintf(fp, "JLE s%d\n", q->label);
					break;

				case OPERATOR_JMP:
					fprintf(fp, "JMP s%d\n", q->label);
					break;

				case OPERATOR_PRINT_MSG:
					fprintf(fp, "MOV EAX, 4\n");
					fprintf(fp, "MOV EBX, 1\n");
					fprintf(fp, "MOV ECX, output\n");
					fprintf(fp, "MOV EDX, output_len\n");
					fprintf(fp, "INT 80H\n");
					break;

				case OPERATOR_PRINT:
					if(q->type1 == OPERAND_CONSTANT){
						// Constant
						if(q->size2){
							// Boolean
							if(q->op1){
								fprintf(fp, "MOV ECX, true\n");
								fprintf(fp, "MOV EDX, true_len\n");
							}else{
								fprintf(fp, "MOV ECX, false\n");
								fprintf(fp, "MOV EDX, false_len\n");
							}
						}else{
							// Number
							if(q->memory2 > 0){
								// ASCII
								fprintf(fp, "MOV BYTE [temp], %s\n", op1);
								fprintf(fp, "MOV ECX, temp\n");
								fprintf(fp, "MOV EDX, 1\n");
							}else if(q->memory2 < 0){
								// Run-time error
								fprintf(fp, "MOV ECX, error\n");
								fprintf(fp, "MOV EDX, error_len\n");
								fprintf(fp, "MOV EAX, 4\n");
								fprintf(fp, "MOV EBX, 1\n");
								fprintf(fp, "INT 80H\n");
								switch(-q->memory2){
									case 1:
										fprintf(fp, "MOV ECX, bound_error\n");
										fprintf(fp, "MOV EDX, bound_error_len\n");
										break;
									case 2:
										fprintf(fp, "MOV ECX, range_error\n");
										fprintf(fp, "MOV EDX, range_error_len\n");
										break;
								}
							}else{
								// Printing number by digits
								int neg = 0;
								if(q->op1 < 0){
									fprintf(fp, "MOV BYTE [temp], '-'\n");
									neg = 1;
								}
								for(int i = 0; i < strlen(op1); i++)
									fprintf(fp, "MOV BYTE [temp + %d], '%c'\n", i + neg, op1[i]);
								fprintf(fp, "MOV ECX, temp\n");
								fprintf(fp, "MOV EDX, %ld\n", strlen(op1) + neg);
							}
						}
					}else{
						// Variable
						if(q->size2){
							// Boolean
							if(q->label){
								// Array
								fprintf(fp, "MOV EDX, %d\n", q->size1);
								fprintf(fp, "MUL EDX\n");
								fprintf(fp, "ADD EAX, DWORD %s\n", op1);
								fprintf(fp, "MOV AL, BYTE [EAX]\n");
							}else
								fprintf(fp, "MOV AL, %s%s\n", size1, op1);

							// Selecting true or false
							fprintf(fp, "CMP AL, 0\n");
							fprintf(fp, "JE bool%d\n", q->label ? q->label : q->type2);
							fprintf(fp, "MOV ECX, true\n");
							fprintf(fp, "MOV EDX, true_len\n");
							fprintf(fp, "JMP endbool%d\n", q->label ? q->label : q->type2);
							fprintf(fp, "bool%d:\n", q->label ? q->label : q->type2);
							fprintf(fp, "MOV ECX, false\n");
							fprintf(fp, "MOV EDX, false_len\n");
							fprintf(fp, "endbool%d:\n", q->label ? q->label : q->type2);
						}else{
							// Integer (as no real)
							if(q->label){
								// Array
								fprintf(fp, "MOV EDX, %d\n", q->size1);
								fprintf(fp, "MUL EDX\n");
								fprintf(fp, "ADD EAX, DWORD %s\n", op1);
								fprintf(fp, "MOV AX, %s[EAX]\n", size1);
							}else
								fprintf(fp, "MOV AX, %s%s\n", size1, op1);

							// Printing by digits (also negative numbers)
							fprintf(fp, "XOR EDI, EDI\n");
							fprintf(fp, "XOR ECX, ECX\n");
							fprintf(fp, "CMP AX, 0\n");
							fprintf(fp, "JGE start%d\n", q->label ? q->label : q->type2);
							fprintf(fp, "NOT AX\n");
							fprintf(fp, "ADD AX, 1\n");
							fprintf(fp, "MOV EDI, 1\n");
							fprintf(fp, "start%d:\n", q->label ? q->label : q->type2);
							fprintf(fp, "XOR DX, DX\n");
							fprintf(fp, "MOV BX, 10\n");
							fprintf(fp, "DIV BX\n");
							fprintf(fp, "ADD DX, '0'\n");
							fprintf(fp, "MOV EBX, ECX\n");
							fprintf(fp, "CMP EBX, 0\n");
							fprintf(fp, "JLE end%d\n", q->label ? q->label : q->type2);
							fprintf(fp, "label%d:\n", q->label ? q->label : q->type2);
							fprintf(fp, "MOV DH, BYTE [temp + EBX - 1]\n");
							fprintf(fp, "MOV BYTE [temp + EBX], DH\n");
							fprintf(fp, "DEC EBX\n");
							fprintf(fp, "CMP EBX, 0\n");
							fprintf(fp, "JNE label%d\n", q->label ? q->label : q->type2);
							fprintf(fp, "end%d:\n", q->label ? q->label : q->type2);
							fprintf(fp, "INC ECX\n");
							fprintf(fp, "MOV BYTE [temp], DL\n");
							fprintf(fp, "CMP AX, 0\n");
							fprintf(fp, "JNE start%d\n", q->label ? q->label : q->type2);
							fprintf(fp, "CMP EDI, 0\n");
							fprintf(fp, "JE print%d\n", q->label ? q->label : q->type2);
							fprintf(fp, "MOV EBX, ECX\n");
							fprintf(fp, "minus%d:\n", q->label ? q->label : q->type2);
							fprintf(fp, "MOV DH, BYTE [temp + EBX - 1]\n");
							fprintf(fp, "MOV BYTE [temp + EBX], DH\n");
							fprintf(fp, "DEC EBX\n");
							fprintf(fp, "CMP EBX, 0\n");
							fprintf(fp, "JNE minus%d\n", q->label ? q->label : q->type2);
							fprintf(fp, "MOV BYTE [temp], '-'\n");
							fprintf(fp, "INC ECX\n");
							fprintf(fp, "print%d:\n", q->label ? q->label : q->type2);
							fprintf(fp, "MOV EDX, ECX\n");
							fprintf(fp, "MOV ECX, temp\n");
						}
					}

					// Printing
					fprintf(fp, "MOV EAX, 4\n");
					fprintf(fp, "MOV EBX, 1\n");
					fprintf(fp, "INT 80H\n");

					// If run-time error, quit
					if(q->type1 == OPERAND_CONSTANT && !q->size2 && q->memory2 < 0){
						fprintf(fp, "MOV EAX, 1\n");
						fprintf(fp, "MOV EBX, 0\n");
						fprintf(fp, "INT 80H\n");
					}
					break;

				case OPERATOR_GET_VALUE_MSG:
					fprintf(fp, "MOV EAX, 4\n");
					fprintf(fp, "MOV EBX, 1\n");
					fprintf(fp, "MOV ECX, input\n");
					fprintf(fp, "MOV EDX, input_len\n");
					fprintf(fp, "INT 80H\n");

					switch(q->type1){
						case TYPE_BOOLEAN:
							fprintf(fp, "MOV EAX, 4\n");
							fprintf(fp, "MOV EBX, 1\n");
							fprintf(fp, "MOV ECX, boolean\n");
							fprintf(fp, "MOV EDX, boolean_len\n");
							break;
						case TYPE_INTEGER:
							fprintf(fp, "MOV EAX, 4\n");
							fprintf(fp, "MOV EBX, 1\n");
							fprintf(fp, "MOV ECX, integer\n");
							fprintf(fp, "MOV EDX, integer_len\n");
							break;
						case TYPE_REAL:
							fprintf(fp, "MOV EAX, 4\n");
							fprintf(fp, "MOV EBX, 1\n");
							fprintf(fp, "MOV ECX, real\n");
							fprintf(fp, "MOV EDX, real_len\n");
							break;
						case TYPE_ARRAY:
							// Printing number of elements

							if(q->size2 || q->memory2){
								// Dynamic
								if(q->memory2)
									fprintf(fp, "MOV AX, WORD [EBP %s %d]\n", q->type2 > 0 ? "-" : "+", q->type2 > 0 ? q->type2 : -q->type2);
								else
									fprintf(fp, "MOV AX, %d\n", q->type2);
								if(q->size2)
									fprintf(fp, "SUB AX, WORD [EBP %s %d]\n", q->memory1 > 0 ? "-" : "+", q->memory1 > 0 ? q->memory1 : -q->memory1);
								else
									fprintf(fp, "SUB AX, %d\n", q->memory1);

								// Printing by digits
								fprintf(fp, "ADD AX, 1\n");
								fprintf(fp, "XOR ECX, ECX\n");
								fprintf(fp, "msgdiff%d:\n", q->label);
								fprintf(fp, "XOR DX, DX\n");
								fprintf(fp, "MOV BX, 10\n");
								fprintf(fp, "DIV BX\n");
								fprintf(fp, "ADD DX, '0'\n");
								fprintf(fp, "MOV EBX, ECX\n");
								fprintf(fp, "CMP EBX, 0\n");
								fprintf(fp, "JE endmsgdiff%d\n", q->label);
								fprintf(fp, "labelmsgdiff%d:\n", q->label);
								fprintf(fp, "MOV DH, BYTE [temp + EBX - 1]\n");
								fprintf(fp, "MOV BYTE [temp + EBX], DH\n");
								fprintf(fp, "DEC EBX\n");
								fprintf(fp, "CMP EBX, 0\n");
								fprintf(fp, "JNE labelmsgdiff%d\n", q->label);
								fprintf(fp, "endmsgdiff%d:\n", q->label);
								fprintf(fp, "INC ECX\n");
								fprintf(fp, "MOV BYTE [temp], DL\n");
								fprintf(fp, "CMP AX, 0\n");
								fprintf(fp, "JNE msgdiff%d\n", q->label);
								fprintf(fp, "MOV EDX, ECX\n");
							}else{
								// Static
								intToStr(q->type2 - q->memory1 + 1, temp);
								for(int i = 0; i < strlen(temp); i++)
									fprintf(fp, "MOV BYTE [temp + %d], '%c'\n", i, temp[i]);
								fprintf(fp, "MOV EDX, %ld\n", strlen(temp));
							}

							// Printing
							fprintf(fp, "MOV EAX, 4\n");
							fprintf(fp, "MOV EBX, 1\n");
							fprintf(fp, "MOV ECX, temp\n");
							break;
					}
					fprintf(fp, "INT 80H\n");

					fprintf(fp, "MOV EAX, 4\n");
					fprintf(fp, "MOV EBX, 1\n");

					if(q->type1 == TYPE_ARRAY){
						fprintf(fp, "MOV ECX, array\n");
						fprintf(fp, "MOV EDX, array_len\n");
						fprintf(fp, "INT 80H\n");

						fprintf(fp, "MOV EAX, 4\n");
						fprintf(fp, "MOV EBX, 1\n");
						switch(q->size1){
							// Output type
							case TYPE_BOOLEAN:
								fprintf(fp, "MOV ECX, boolean\n");
								fprintf(fp, "MOV EDX, boolean_len\n");
								break;
							case TYPE_INTEGER:
								fprintf(fp, "MOV ECX, integer\n");
								fprintf(fp, "MOV EDX, integer_len\n");
								break;
							case TYPE_REAL:
								fprintf(fp, "MOV ECX, real\n");
								fprintf(fp, "MOV EDX, real_len\n");
								break;
						}
						fprintf(fp, "INT 80H\n");

						fprintf(fp, "MOV EAX, 4\n");
						fprintf(fp, "MOV EBX, 1\n");
						fprintf(fp, "MOV ECX, array_range\n");
						fprintf(fp, "MOV EDX, array_range_len\n");
						fprintf(fp, "INT 80H\n");

						if(q->size2){
							// start is variable
							fprintf(fp, "MOV AX, WORD [EBP %s %d]\n", q->memory1 > 0 ? "-" : "+", q->memory1 > 0 ? q->memory1 : -q->memory1);
							fprintf(fp, "XOR ECX, ECX\n");
							fprintf(fp, "msgi%d:\n", q->label);
							fprintf(fp, "XOR DX, DX\n");
							fprintf(fp, "MOV BX, 10\n");
							fprintf(fp, "DIV BX\n");
							fprintf(fp, "ADD DX, '0'\n");
							fprintf(fp, "MOV EBX, ECX\n");
							fprintf(fp, "CMP EBX, 0\n");
							fprintf(fp, "JE endmsgi%d\n", q->label);
							fprintf(fp, "labelmsgi%d:\n", q->label);
							fprintf(fp, "MOV DH, BYTE [temp + EBX - 1]\n");
							fprintf(fp, "MOV BYTE [temp + EBX], DH\n");
							fprintf(fp, "DEC EBX\n");
							fprintf(fp, "CMP EBX, 0\n");
							fprintf(fp, "JNE labelmsgi%d\n", q->label);
							fprintf(fp, "endmsgi%d:\n", q->label);
							fprintf(fp, "INC ECX\n");
							fprintf(fp, "MOV BYTE [temp], DL\n");
							fprintf(fp, "CMP AX, 0\n");
							fprintf(fp, "JNE msgi%d\n", q->label);
							fprintf(fp, "MOV EDX, ECX\n");
						}else{
							// start is constant
							intToStr(q->memory1, temp);
							for(int i = 0; i < strlen(temp); i++)
								fprintf(fp, "MOV BYTE [temp + %d], '%c'\n", i, temp[i]);
							fprintf(fp, "MOV EDX, %ld\n", strlen(temp));
						}

						fprintf(fp, "MOV EAX, 4\n");
						fprintf(fp, "MOV EBX, 1\n");
						fprintf(fp, "MOV ECX, temp\n");
						fprintf(fp, "INT 80H\n");

						fprintf(fp, "MOV EAX, 4\n");
						fprintf(fp, "MOV EBX, 1\n");
						fprintf(fp, "MOV ECX, array_to\n");
						fprintf(fp, "MOV EDX, array_to_len\n");
						fprintf(fp, "INT 80H\n");

						if(q->memory2){
							// end is variable
							fprintf(fp, "MOV AX, WORD [EBP %s %d]\n", q->type2 > 0 ? "-" : "+", q->type2 > 0 ? q->type2 : -q->type2);
							fprintf(fp, "XOR ECX, ECX\n");
							fprintf(fp, "msgo%d:\n", q->label);
							fprintf(fp, "XOR DX, DX\n");
							fprintf(fp, "MOV BX, 10\n");
							fprintf(fp, "DIV BX\n");
							fprintf(fp, "ADD DX, '0'\n");
							fprintf(fp, "MOV EBX, ECX\n");
							fprintf(fp, "CMP EBX, 0\n");
							fprintf(fp, "JE endmsgo%d\n", q->label);
							fprintf(fp, "labelmsgo%d:\n", q->label);
							fprintf(fp, "MOV DH, BYTE [temp + EBX - 1]\n");
							fprintf(fp, "MOV BYTE [temp + EBX], DH\n");
							fprintf(fp, "DEC EBX\n");
							fprintf(fp, "CMP EBX, 0\n");
							fprintf(fp, "JNE labelmsgo%d\n", q->label);
							fprintf(fp, "endmsgo%d:\n", q->label);
							fprintf(fp, "INC ECX\n");
							fprintf(fp, "MOV BYTE [temp], DL\n");
							fprintf(fp, "CMP AX, 0\n");
							fprintf(fp, "JNE msgo%d\n", q->label);
							fprintf(fp, "MOV EDX, ECX\n");
						}else{
							// end is constant
							intToStr(q->type2, temp);
							for(int i = 0; i < strlen(temp); i++)
								fprintf(fp, "MOV BYTE [temp + %d], '%c'\n", i, temp[i]);
							fprintf(fp, "MOV EDX, %ld\n", strlen(temp));
						}

						fprintf(fp, "MOV EAX, 4\n");
						fprintf(fp, "MOV EBX, 1\n");
						fprintf(fp, "MOV ECX, temp\n");
					}else{
						// Non-array
						fprintf(fp, "MOV ECX, value\n");
						fprintf(fp, "MOV EDX, value_len\n");
					}
					fprintf(fp, "INT 80H\n");
					break;

				case OPERATOR_GET_VALUE:
					// Getting value from stdin
					fprintf(fp, "MOV EAX, 3\n");
					fprintf(fp, "MOV EBX, 0\n");
					fprintf(fp, "MOV ECX, temp\n");
					fprintf(fp, "MOV EDX, temp_len\n");
					fprintf(fp, "INT 80H\n");

					// Assigning in AX (also checking if negative)
					fprintf(fp, "XOR AX, AX\n");
					fprintf(fp, "XOR BX, BX\n");
					fprintf(fp, "XOR ECX, ECX\n");
					fprintf(fp, "CMP BYTE [temp], '-'\n");
					fprintf(fp, "JNE start%d\n", q->label ? q->label : q->type2);
					fprintf(fp, "MOV ECX, 1\n");
					fprintf(fp, "start%d:\n", q->label ? q->label : q->type2);
					fprintf(fp, "MOV BL, BYTE [temp + ECX]\n");
					fprintf(fp, "CMP BL, 0AH\n");
					fprintf(fp, "JE end%d\n", q->label ? q->label : q->type2);
					fprintf(fp, "SUB BX, '0'\n");
					fprintf(fp, "MOV DX, 10\n");
					fprintf(fp, "MUL DX\n");
					fprintf(fp, "ADD AX, BX\n");
					fprintf(fp, "INC ECX\n");
					fprintf(fp, "JMP start%d\n", q->label ? q->label : q->type2);
					fprintf(fp, "end%d:\n", q->label ? q->label : q->type2);
					fprintf(fp, "CMP BYTE [temp], '-'\n");
					fprintf(fp, "JNE assign%d\n", q->label ? q->label : q->type2);
					fprintf(fp, "NOT AX\n");
					fprintf(fp, "ADD AX, 1\n");
					fprintf(fp, "assign%d:\n", q->label ? q->label : q->type2);

					if(q->size1 == 1){
						// Boolean
						fprintf(fp, "CMP AX, 0\n");
						fprintf(fp, "JL boolfalse%d\n", q->label ? q->label : q->type2);
						fprintf(fp, "CMP AX, 1\n");
						fprintf(fp, "JLE booltrue%d\n", q->label ? q->label : q->type2);
						fprintf(fp, "boolfalse%d:\n", q->label ? q->label : q->type2);
						fprintf(fp, "MOV ECX, error\n");
						fprintf(fp, "MOV EDX, error_len\n");
						fprintf(fp, "MOV EAX, 4\n");
						fprintf(fp, "MOV EBX, 1\n");
						fprintf(fp, "INT 80H\n");
						fprintf(fp, "MOV ECX, bool_error\n");
						fprintf(fp, "MOV EDX, bool_error_len\n");
						fprintf(fp, "MOV EAX, 4\n");
						fprintf(fp, "MOV EBX, 1\n");
						fprintf(fp, "INT 80H\n");
						fprintf(fp, "MOV EAX, 1\n");
						fprintf(fp, "MOV EBX, 0\n");
						fprintf(fp, "INT 80H\n");
						fprintf(fp, "booltrue%d:\n", q->label ? q->label : q->type2);
					}

					if(q->label){
						// Array
						fprintf(fp, "MOV BX, AX\n");
						fprintf(fp, "POP EAX\n");
						fprintf(fp, "MOV ECX, EAX\n");
						fprintf(fp, "MOV EDX, %d\n", q->size1);
						fprintf(fp, "MUL EDX\n");
						fprintf(fp, "ADD EAX, DWORD %s\n", op1);
						switch(q->size1){
							case 1:
								strcpy(temp, registers_byte[1]);
								break;
							case 2:
								strcpy(temp, registers_word[1]);
								break;
							case 4:
							default:
								strcpy(temp, registers_dword[1]);
								break;
						}
						fprintf(fp, "MOV %s[EAX], %s\n", size1, temp);
						fprintf(fp, "MOV EAX, ECX\n");
					}else
						// Single varaible
						fprintf(fp, "MOV %s%s, AX\n", size1, op1);
					break;

				case OPERATOR_PUSH:
					if(q->size1 == 1){
						// Can't push 1 byte
						fprintf(fp, "SUB ESP, 1\n");
						fprintf(fp, "MOV AL, %s%s\n", size1, op1);
						fprintf(fp, "MOV BYTE [ESP], AL\n");
					}else
						fprintf(fp, "PUSH %s%s\n", size1, op1);
					break;

				case OPERATOR_POP:
					if(q->size1 == 1){
						// Can't pop 1 byte
						fprintf(fp, "MOV AL, BYTE [ESP]\n");
						fprintf(fp, "MOV %s%s, AL\n", size1, op1);
						fprintf(fp, "ADD ESP, 1\n");
					}else
						fprintf(fp, "POP %s%s\n", size1, op1);
					break;

				case OPERATOR_CALL:
					fprintf(fp, "CALL _m%d\n", q->label);
					break;

				case OPERATOR_RETURN:
					fprintf(fp, "RET\n");
					break;

				case OPERATOR_END:
					// Quit
					fprintf(fp, "MOV EAX, 1\n");
					fprintf(fp, "MOV EBX, 0\n");
					fprintf(fp, "INT 80H\n");
					break;
			}
		}
		quadruple temp = q->next;
		free(q);
		q = temp;
	}

	fclose(fp);
	free(_code);
	return;

}

quadruple append(quadruple start, quadruple q){

	// Appends at end, returns start

	if(start){
		quadruple end = start;
		while(end->next)
			end = end->next;
		end->next = q;
	}else
		start = q;
	return start;

}

quadruple createQ(quadruple start, int operator, int op1, int type1, int size1, int memory1, int op2, int type2, int size2, int memory2, int label){

	// Creates new quadruple, appends at end of start
	quadruple q = (quadruple)malloc(sizeof(struct _quadruple));
	q->next = NULL;
	q->operator = operator;

	q->op1 = op1;
	q->type1 = type1;
	q->size1 = size1;
	q->memory1 = memory1;

	q->op2 = op2;
	q->type2 = type2;
	q->size2 = size2;
	q->memory2 = memory2;

	q->label = label;
	return append(start, q);

}

quadruple createLabel(quadruple start, int operator, int label){
	// Shorter version, to create labels
	return createQ(start, operator, 0, 0, 0, 0, 0, 0, 0, 0, label);
}

code generateCode(ast tree, int currentLabel){

	code codes[tree->totalChildren];
	int labels[3] = {0}; // To store labels / pass ith down to ith child

	// Assign labels to children (also save, as children may use different labels)
	switch(tree->construct){
		case CONSTRUCT_STATEMENTS:
			labels[1] = getLabel();
			break;

		case CONSTRUCT_ASSIGN:
			labels[1] = getLabel();
			break;

		case CONSTRUCT_OPERATOR:
			labels[0] = tree->lexeme[0] == 'O' ? currentLabel : getLabel();
			labels[1] = currentLabel;
			break;

		case CONSTRUCT_LOOP:
			if(tree->totalChildren == 2){
				labels[0] = getLabel();
				labels[2] = getLabel();
			}
			break;

		case CONSTRUCT_SWITCH:
			labels[0] = getLabel();
			labels[1] = getLabel();
			break;
	}

	// Getting children's code
	if(tree->construct != CONSTRUCT_SWITCH)
		for(int i = 0; i < tree->totalChildren; i++)
			codes[i] = generateCode(tree->children[i], labels[i]);

	quadruple q = NULL, tempQ = NULL;
	int operator = OPERATOR_DEFAULT;
	ast tempTree;
	ast new;
	code tempCode;
	type _type;
	tableEntry entryStart = NULL, entryEnd = NULL;

	code _code = (code)malloc(sizeof(struct _code));
	_code->var = 0;
	_code->type = 0;
	_code->memory = 0;
	_code->size = 0;

	int startInstruction = 0; // To skip some code appending

	// Before appending children
	switch(tree->construct){
		case CONSTRUCT_OPERATOR:
			switch(tree->lexeme[0]){
				case 'A':
					q = append(q, codes[0]->start);
					q = createLabel(q, OPERATOR_JMP, getLabel());

					q = createLabel(q, OPERATOR_DEFAULT, labels[0]);
					q = append(q, codes[1]->start);

					q = createLabel(q, OPERATOR_DEFAULT, label);

					startInstruction = 2;
					break;
			}
			break;

		case CONSTRUCT_LOOP:
			switch(tree->totalChildren){
				case 2:
					// while
					q = createLabel(q, OPERATOR_DEFAULT, currentLabel);
					if(tree->children[0]->construct == CONSTRUCT_VARIABLE)
						q = createQ(q, OPERATOR_EQ, codes[0]->var, codes[0]->type, codes[0]->size, codes[0]->memory, 0, OPERAND_CONSTANT, 0, 0, labels[2]);
					else{
						q = append(q, codes[0]->start);
						q = createLabel(q, OPERATOR_JMP, labels[2]);
					}
					q = createLabel(q, OPERATOR_DEFAULT, labels[0]);
					startInstruction = 1;
					break;
				case 4:
					// for
					q = append(q, codes[1]->start);
					q = createQ(q, OPERATOR_ASSIGN, codes[0]->var, codes[0]->type, codes[0]->size, codes[0]->memory, codes[1]->var, codes[1]->type, codes[1]->size, codes[1]->memory, 0);
					q = createLabel(q, OPERATOR_DEFAULT, getLabel());
					startInstruction = 2;
					break;
			}
			break;

		case CONSTRUCT_SWITCH:
			codes[0] = generateCode(tree->children[0], 0);
			tempTree = tree->children[1];
			while(tempTree){
				labels[2] = getLabel();

				tempCode = generateCode(tempTree->children[0], 0);
				q = createQ(q, OPERATOR_EQ, codes[0]->var, codes[0]->type, codes[0]->size, codes[0]->memory, tempCode->var, OPERAND_CONSTANT, 0, 0, labels[2]);
				free(tempCode);

				tempQ = createLabel(tempQ, OPERATOR_DEFAULT, labels[2]);
				tempCode = generateCode(tempTree->children[1], 0);
				tempQ = append(tempQ, tempCode->start);
				tempQ = createLabel(tempQ, OPERATOR_JMP, labels[0]);
				free(tempCode);

				tempTree = tempTree->next;
			}
			q = createLabel(q, OPERATOR_JMP, labels[1]);
			tempQ = createLabel(tempQ, OPERATOR_DEFAULT, labels[1]);
			if(tree->totalChildren == 3){
				codes[2] = generateCode(tree->children[2], 0);
				tempQ = append(tempQ, codes[2]->start);
			}
			q = append(q, tempQ);
			q = createLabel(q, OPERATOR_DEFAULT, labels[0]);
			free(codes[0]);
			startInstruction = 4;
			break;

		case CONSTRUCT_IO:
			operator = tree->lexeme[0] == 'p' ? OPERATOR_PRINT : OPERATOR_GET_VALUE;

			// start and end symbol entries
			_type = tree->children[0]->type;
			if(_type->startLexeme[0])
				entryStart = getIdInSymbolTable(_type->startLexeme, tree->table);
			if(_type->endLexeme[0])
				entryEnd = getIdInSymbolTable(_type->endLexeme, tree->table);

			// Message
			q = createQ(q, operator - 1, 0, _type->basic, _type->output ? _type->output->basic : 0, entryStart ? entryStart->offset : _type->start, 0, entryEnd ? entryEnd->offset : _type->end, entryStart != NULL, entryEnd != NULL, getLabel());

			// Enter
			if(operator == OPERATOR_GET_VALUE)
				q = createQ(q, OPERATOR_PRINT, '\n', OPERAND_CONSTANT, 0, 0, 0, 0, 0, 1, 0);
			break;

		case CONSTRUCT_MODULE:
			// Creating unique label
			q = createLabel(q, OPERATOR_DEFAULT, tree->entry->offset);
			q->type1 = 1;

			// Saving EBP, allocating space on stack
			q = createQ(q, OPERATOR_PUSH, EBP, OPERAND_REGISTER, 0, 0, 0, 0, 0, 0, 0);
			q = createQ(q, OPERATOR_ASSIGN, EBP, OPERAND_REGISTER, 0, 0, ESP, OPERAND_REGISTER, 0, 0, 0);
			q = createQ(q, OPERATOR_MINUS, ESP, OPERAND_REGISTER, 0, 0, tree->table->width, OPERAND_CONSTANT, 0, 0, 0);
			break;

		case CONSTRUCT_ENTRY:
			// Start label
			q = createLabel(q, OPERATOR_DEFAULT, -1);

			// Allocating space on stack
			q = createQ(q, OPERATOR_ASSIGN, EBP, OPERAND_REGISTER, 0, 0, ESP, OPERAND_REGISTER, 0, 0, 0);
			q = createQ(q, OPERATOR_MINUS, ESP, OPERAND_REGISTER, 0, 0, tree->table->width, OPERAND_CONSTANT, 0, 0, 0);
			break;
	}

	// Appending children
	for(int i = startInstruction; i < tree->totalChildren; i++)
		q = append(q, codes[i]->start);

	// After appending children
	switch(tree->construct){
		case CONSTRUCT_VARIABLE:
			// Variable is just its offset from EBP
			_code->var = tree->entry->offset;
			_code->type = OPERAND_VARIABLE;
			if(tree->type->basic == TYPE_ARRAY)
				_code->size = WIDTHS[TYPE_ARRAY]; // entry stores total width, but we need address
			else
				_code->size = tree->entry->width;
			_code->memory = 1;
			break;

		case CONSTRUCT_WHICH_ID:
			// (index - start) * sizeof(element_type) + base_address

			tempTree = tree->children[0];

			// Clearing total register (as will be assigned AX, but will be used EAX)
			q = createQ(q, OPERATOR_ASSIGN, getVar(), OPERAND_REGISTER, 0, 0, 0, OPERAND_CONSTANT, 0, 0, 0);
			q = createQ(q, OPERATOR_PLUS, var, OPERAND_REGISTER, codes[1]->size, 0, codes[1]->var, codes[1]->type, codes[1]->size, codes[1]->memory, 0);

			labels[0] = getLabel();
			labels[1] = getLabel();

			if(tempTree->type->startLexeme[0])
				entryStart = getIdInSymbolTable(tempTree->type->startLexeme, tempTree->table);
			if(tempTree->type->endLexeme[0])
				entryEnd = getIdInSymbolTable(tempTree->type->endLexeme, tempTree->table);

			// Bound check start
			if(entryStart)
				q = createQ(q, OPERATOR_LT, var, OPERAND_REGISTER, entryStart->width, 0, entryStart->offset, OPERAND_VARIABLE, entryStart->width, 1, labels[0]);
			else{
				if(tempTree->entry->useType == USE_INPUT)
					q = createQ(q, OPERATOR_LT, var, OPERAND_REGISTER, WIDTHS[TYPE_INTEGER], 0, tempTree->entry->offset - WIDTHS[TYPE_ARRAY], OPERAND_VARIABLE, WIDTHS[TYPE_INTEGER], 1, labels[0]);
				else
					q = createQ(q, OPERATOR_LT, var, OPERAND_REGISTER, 0, 0, tempTree->type->start, OPERAND_CONSTANT, 0, 0, labels[0]);
			}

			// Bound check end
			if(entryEnd)
				q = createQ(q, OPERATOR_GT, var, OPERAND_REGISTER, entryEnd->width, 0, entryEnd->offset, OPERAND_VARIABLE, entryEnd->width, 1, labels[0]);
			else{
				if(tempTree->entry->useType == USE_INPUT)
					q = createQ(q, OPERATOR_GT, var, OPERAND_REGISTER, WIDTHS[TYPE_INTEGER], 0, tempTree->entry->offset - WIDTHS[TYPE_ARRAY] - WIDTHS[TYPE_INTEGER], OPERAND_VARIABLE, WIDTHS[TYPE_INTEGER], 1, labels[0]);
				else
					q = createQ(q, OPERATOR_GT, var, OPERAND_REGISTER, 0, 0, tempTree->type->end, OPERAND_CONSTANT, 0, 0, labels[0]);
			}

			// index - start
			if(entryStart)
				q = createQ(q, OPERATOR_MINUS, var, OPERAND_REGISTER, entryStart->width, 0, entryStart->offset, OPERAND_VARIABLE, entryStart->width, 1, 0);
			else{
				if(tempTree->entry->useType == USE_INPUT)
					q = createQ(q, OPERATOR_MINUS, var, OPERAND_REGISTER, WIDTHS[TYPE_INTEGER], 0, tempTree->entry->offset - WIDTHS[TYPE_ARRAY], OPERAND_VARIABLE, WIDTHS[TYPE_INTEGER], 1, 0);
				else
					q = createQ(q, OPERATOR_MINUS, var, OPERAND_REGISTER, WIDTHS[TYPE_INTEGER], 0, tempTree->type->start, OPERAND_CONSTANT, 0, 0, 0);
			}

			_code->var = var;
			_code->type = OPERAND_REGISTER;
			_code->size = WIDTHS[tempTree->type->output->basic];
			_code->memory = 1;

			q = createQ(q, OPERATOR_ASSIGN, getVar(), OPERAND_REGISTER, 0, 0, WIDTHS[tempTree->type->output->basic], OPERAND_CONSTANT, 0, 0, 0);
			q = createQ(q, OPERATOR_MUL, _code->var, OPERAND_REGISTER, 0, 0, var, OPERAND_REGISTER, 0, 0, 0);
			q = createQ(q, OPERATOR_PLUS, _code->var, OPERAND_REGISTER, 0, 0, codes[0]->var, codes[0]->type, codes[0]->size, codes[0]->memory, 0);

			q = createLabel(q, OPERATOR_JMP, labels[1]);

			// Print bound error
			q = createLabel(q, OPERATOR_DEFAULT, labels[0]);
			q = createQ(q, OPERATOR_PRINT, 0, OPERAND_CONSTANT, 0, 0, 0, 0, 0, -1, 0);

			q = createLabel(q, OPERATOR_DEFAULT, labels[1]);

			break;

		case CONSTRUCT_CONSTANT:
			_code->type = OPERAND_CONSTANT;
			switch(tree->type->basic){
				case TYPE_BOOLEAN:
					_code->var = tree->lexeme[0] == 't';
					if(currentLabel && _code->var)
						q = createLabel(q, OPERATOR_JMP, currentLabel); // Skipping true and false
					break;
				case TYPE_INTEGER:
					_code->var = atoi(tree->lexeme);
					break;
				// No real
			}
			break;

		case CONSTRUCT_DECLARE:
			// Assign address to arrays
			if(tree->type->basic == TYPE_ARRAY){
				tempTree = tree;

				if(tempTree->type->startLexeme[0])
					entryStart = getIdInSymbolTable(tempTree->type->startLexeme, tempTree->table);
				if(tempTree->type->endLexeme[0])
					entryEnd = getIdInSymbolTable(tempTree->type->endLexeme, tempTree->table);

				while(tempTree){

					if(!entryStart && !entryEnd){
						// Static (assigning address of first element)
						q = createQ(q, OPERATOR_ASSIGN, tempTree->entry->offset, OPERAND_VARIABLE, WIDTHS[TYPE_ARRAY], 1, EBP, OPERAND_REGISTER, 0, 0, 0);
						q = createQ(q, OPERATOR_MINUS, tempTree->entry->offset, OPERAND_VARIABLE, WIDTHS[TYPE_ARRAY], 1, tempTree->entry->offset + tempTree->entry->width - WIDTHS[TYPE_ARRAY], OPERAND_CONSTANT, 0, 0, 0);
					}else{
						// Dynamic (space needed = (end - start + 1) * sizeof(type))
						q = createQ(q, OPERATOR_ASSIGN, getVar(), OPERAND_REGISTER, WIDTHS[TYPE_ARRAY], 0, 0, OPERAND_CONSTANT, 0, 0, 0);
						_code->var = var;

						if(entryEnd)
							q = createQ(q, OPERATOR_ASSIGN, var, OPERAND_REGISTER, entryEnd->width, 0, entryEnd->offset, OPERAND_VARIABLE, entryEnd->width, 1, 0);
						else
							q = createQ(q, OPERATOR_ASSIGN, var, OPERAND_REGISTER, WIDTHS[TYPE_INTEGER], 0, tempTree->type->end, OPERAND_CONSTANT, 0, 0, 0);

						if(entryStart)
							q = createQ(q, OPERATOR_MINUS, var, OPERAND_REGISTER, entryStart->width, 0, entryStart->offset, OPERAND_VARIABLE, entryStart->width, 1, 0);
						else
							q = createQ(q, OPERATOR_MINUS, var, OPERAND_REGISTER, WIDTHS[TYPE_INTEGER], 0, tempTree->type->start, OPERAND_CONSTANT, 0, 0, 0);

						q = createQ(q, OPERATOR_PLUS, var, OPERAND_REGISTER, WIDTHS[TYPE_INTEGER], 0, 1, OPERAND_CONSTANT, 0, 0, 0);

						// Range check
						q = createQ(q, OPERATOR_GT, var, OPERAND_REGISTER, WIDTHS[TYPE_INTEGER], 0, 0, OPERAND_CONSTANT, 0, 0, getLabel());
						q = createQ(q, OPERATOR_PRINT, 0, OPERAND_CONSTANT, 0, 0, 0, 0, 0, -2, 0);

						q = createLabel(q, OPERATOR_DEFAULT, label);
						q = createQ(q, OPERATOR_ASSIGN, getVar(), OPERAND_REGISTER, 0, 0, WIDTHS[tempTree->type->output->basic], OPERAND_CONSTANT, 0, 0, 0);
						q = createQ(q, OPERATOR_MUL, _code->var, OPERAND_REGISTER, 0, 0, var, OPERAND_REGISTER, 0, 0, 0);

						// Creating stack space, and ESP to array's memory location
						q = createQ(q, OPERATOR_MINUS, ESP, OPERAND_REGISTER, 0, 0, _code->var, OPERAND_REGISTER, 0, 0, 0);
						q = createQ(q, OPERATOR_ASSIGN, tempTree->entry->offset, OPERAND_VARIABLE, WIDTHS[TYPE_ARRAY], 1, ESP, OPERAND_REGISTER, 0, 0, 0);
					}
					tempTree = tempTree->next;
				}
			}
			break;

		case CONSTRUCT_ASSIGN:
			if(tree->children[1]->type->basic == TYPE_BOOLEAN){
				// Boolean => short circuiting
				q = createQ(q, OPERATOR_ASSIGN, getVar(), OPERAND_REGISTER, 0, 0, 0, OPERAND_CONSTANT, 0, 0, 0);
				q = createLabel(q, OPERATOR_JMP, getLabel());

				q = createLabel(q, OPERATOR_DEFAULT, labels[1]);
				q = createQ(q, OPERATOR_ASSIGN, var, OPERAND_REGISTER, 0, 0, 1, OPERAND_CONSTANT, 0, 0, 0);

				q = createLabel(q, OPERATOR_DEFAULT, label);
				q = createQ(q, OPERATOR_ASSIGN, 
					codes[0]->var, codes[0]->type, codes[0]->size, codes[0]->memory, 
					var, OPERAND_REGISTER, codes[0]->size, 0, 0);
			}else{
				if(codes[1]->memory){
					// Both memory not valid instruction
					q = createQ(q, OPERATOR_ASSIGN, getVar(), OPERAND_REGISTER, codes[0]->size, 0, codes[1]->var, codes[1]->type, codes[1]->size, codes[1]->memory, 0);
					q = createQ(q, OPERATOR_ASSIGN, 
						codes[0]->var, codes[0]->type, codes[0]->size, codes[0]->memory, 
						var, OPERAND_REGISTER, codes[0]->size, 0, 0);
				}else{
					q = createQ(q, OPERATOR_ASSIGN, 
						codes[0]->var, codes[0]->type, codes[0]->size, codes[0]->memory, 
						codes[1]->var, codes[1]->type, codes[0]->size, codes[1]->memory, 0);
				}
			}
			break;

		case CONSTRUCT_OPERATOR:
			switch(tree->lexeme[0]){
				case '+':
					operator = OPERATOR_PLUS;
					break;
				case '-':
					operator = OPERATOR_MINUS;
					break;
				case '*':
					operator = OPERATOR_MUL;
					break;
				case '/':
					operator = OPERATOR_DIV;
					break;
				case '>':
					operator = tree->lexeme[1] ? OPERATOR_GE : OPERATOR_GT;
					break;
				case '<':
					operator = tree->lexeme[1] ? OPERATOR_LE : OPERATOR_LT;
					break;
				case '!':
					operator = OPERATOR_NE;
					break;
				case '=':
					operator = OPERATOR_EQ;
					break;
			}
			switch(tree->lexeme[0]){
				case '-':
					if(tree->totalChildren == 1){
						// Unary minus, can only be applied on register
						if(codes[0]->type != OPERAND_REGISTER){
							q = createQ(q, OPERATOR_ASSIGN, getVar(), OPERAND_REGISTER, codes[0]->size ? codes[0]->size : WIDTHS[TYPE_INTEGER], 0, codes[0]->var, codes[0]->type, codes[0]->size, codes[0]->memory, 0);
							q = createQ(q, OPERATOR_UNARY_MINUS, var, OPERAND_REGISTER, codes[0]->size ? codes[0]->size : WIDTHS[TYPE_INTEGER], 0, 0, 0, 0, 0, 0);
							_code->var = var;
							_code->size = codes[0]->size ? codes[0]->size : WIDTHS[TYPE_INTEGER];
							_code->type = OPERAND_REGISTER;
						}else{
							q = createQ(q, OPERATOR_UNARY_MINUS, codes[0]->var, codes[0]->type, codes[0]->size ? codes[0]->size : WIDTHS[TYPE_INTEGER], codes[0]->memory, 0, 0, 0, 0, 0);
							_code->var = codes[0]->var;
							_code->size = codes[0]->size ? codes[0]->size : WIDTHS[TYPE_INTEGER];
							_code->type = codes[0]->type;
						}
						break;
					}
				case '+':
				case '*':
					// Arithmetic operator
					 if(codes[0]->type == OPERAND_CONSTANT || codes[0]->memory){
						// Modifies first parameter
						q = createQ(q, OPERATOR_ASSIGN, getVar(), OPERAND_REGISTER, codes[0]->size ? codes[0]->size : WIDTHS[TYPE_INTEGER], 0, codes[0]->var, codes[0]->type, codes[0]->size, codes[0]->memory, 0);
						q = createQ(q, operator, 
							var, OPERAND_REGISTER, codes[0]->size ? codes[0]->size : WIDTHS[TYPE_INTEGER], 0, 
							codes[1]->var, codes[1]->type, codes[1]->size, codes[1]->memory, 0);
						_code->var = var;
						_code->size = codes[0]->size ? codes[0]->size : WIDTHS[TYPE_INTEGER];
						_code->type = OPERAND_REGISTER;
					}else{
						q = createQ(q, operator, 
							codes[0]->var, codes[0]->type, codes[0]->size ? codes[0]->size : WIDTHS[TYPE_INTEGER], codes[0]->memory, 
							codes[1]->var, codes[1]->type, codes[1]->size, codes[1]->memory, 0);
						_code->var = codes[0]->var;
						_code->type = codes[0]->type;
						_code->size = codes[0]->size ? codes[0]->size : WIDTHS[TYPE_INTEGER];
						_code->memory = codes[0]->memory;
					}
					break;
				case '/':
					// Operand 1 has to be register
					if(codes[0]->type == OPERAND_REGISTER && !codes[0]->memory)
						_code->var = codes[0]->var;
					else{
						q = createQ(q, OPERATOR_ASSIGN, getVar(), OPERAND_REGISTER, codes[0]->size ? codes[0]->size : WIDTHS[TYPE_INTEGER], 0, codes[0]->var, codes[0]->type, codes[0]->size, codes[0]->memory, 0);
						_code->var = var;
					}
					_code->type = OPERAND_REGISTER;
					_code->size = codes[0]->size ? codes[0]->size : WIDTHS[TYPE_INTEGER];

					// Operand 2 can't be constant
					if(codes[1]->type == OPERAND_CONSTANT)
						q = createQ(q, OPERATOR_ASSIGN, getVar(), OPERAND_REGISTER, codes[0]->size ? codes[0]->size : WIDTHS[TYPE_INTEGER], 0, codes[1]->var, codes[1]->type, 0, 0, 0);

					q = createQ(q, operator, 
						_code->var, OPERAND_REGISTER, codes[0]->size ? codes[0]->size : WIDTHS[TYPE_INTEGER], 0, 
						codes[1]->type == OPERAND_CONSTANT ? var : codes[1]->var, codes[1]->type == OPERAND_CONSTANT ? OPERAND_REGISTER : codes[1]->type, codes[1]->type == OPERAND_CONSTANT ? (codes[0]->size ? codes[0]->size : WIDTHS[TYPE_INTEGER]) : codes[1]->size, codes[1]->memory, 0);
					break;
				case '>':
				case '<':
				case '!':
				case '=':
					// Relational operators
					if(codes[0]->type == OPERAND_CONSTANT || (codes[0]->memory && codes[1]->memory)){
						// First constant or both memory not possible
						q = createQ(q, OPERATOR_ASSIGN, getVar(), OPERAND_REGISTER, codes[0]->size ? codes[0]->size : WIDTHS[TYPE_INTEGER], 0, codes[0]->var, codes[0]->type, codes[0]->size, codes[0]->memory, 0);

						q = createQ(q, operator, 
							var, OPERAND_REGISTER, codes[0]->size ? codes[0]->size : WIDTHS[TYPE_INTEGER], 0, 
							codes[1]->var, codes[1]->type, codes[1]->size, codes[1]->memory, currentLabel);

						// Freeing register as no longer needed
						q = createQ(q, OPERATOR_DEFAULT, 0, 0, 0, 0, var, OPERAND_REGISTER, 0, 0, 0);
					}else{
						q = createQ(q, operator, 
							codes[0]->var, codes[0]->type, codes[0]->size, codes[0]->memory, 
							codes[1]->var, codes[1]->type, codes[1]->size, codes[1]->memory, currentLabel);
					}
					break;
			}
			break;

		case CONSTRUCT_IO:
			if(_type->basic == TYPE_ARRAY){
				// Looping through all elements
				q = createQ(q, OPERATOR_ASSIGN, getVar(), OPERAND_REGISTER, 0, 0, 0, OPERAND_CONSTANT, 0, 0, 0);
				q = createLabel(q, OPERATOR_DEFAULT, getLabel());
				labels[0] = label;

				// IO instructions uses all registers, hence PUSH to store them
				q = createQ(q, OPERATOR_PUSH, var, OPERAND_REGISTER, 0, 0, 0, 0, 0, 0, 0);

				// Actual operation
				q = createQ(q, operator, codes[0]->var, codes[0]->type, WIDTHS[_type->output->basic], codes[0]->memory, var, OPERAND_REGISTER, _type->output->basic == TYPE_BOOLEAN, 0, getLabel());

				if(operator == OPERATOR_PRINT){
					q = createQ(q, OPERATOR_PRINT, ' ', OPERAND_CONSTANT, 0, 0, 0, 0, 0, 1, 0);
					q = createQ(q, OPERATOR_POP, var, OPERAND_REGISTER, 0, 0, 0, 0, 0, 0, 0);
				}

				q = createQ(q, OPERATOR_INC, var, OPERAND_REGISTER, 0, 0, 0, 0, 0, 0, 0);

				// End check
				if(!entryStart && !entryEnd){
					// Static
					q = createQ(q, OPERATOR_LE, var, OPERAND_REGISTER, 0, 0, _type->end - _type->start, OPERAND_CONSTANT, 0, 0, labels[0]);
					q = createQ(q, OPERATOR_DEFAULT, 0, 0, 0, 0, var, OPERAND_REGISTER, 0, 0, 0);
				}else{
					// Dynamic
					_code->var = var;
					if(entryEnd)
						q = createQ(q, OPERATOR_ASSIGN, getVar(), OPERAND_REGISTER, entryEnd->width, 0, entryEnd->offset, OPERAND_VARIABLE, entryEnd->width, 1, 0);
					else
						q = createQ(q, OPERATOR_ASSIGN, getVar(), OPERAND_REGISTER, WIDTHS[TYPE_INTEGER], 0, _type->end, OPERAND_CONSTANT, 0, 0, 0);

					if(entryStart)
						q = createQ(q, OPERATOR_MINUS, var, OPERAND_REGISTER, entryStart->width, 0, entryStart->offset, OPERAND_VARIABLE, entryStart->width, 1, 0);
					else
						q = createQ(q, OPERATOR_MINUS, var, OPERAND_REGISTER, WIDTHS[TYPE_INTEGER], 0, _type->start, OPERAND_CONSTANT, 0, 0, 0);

					q = createQ(q, OPERATOR_LE, _code->var, OPERAND_REGISTER, WIDTHS[TYPE_INTEGER], 0, var, OPERAND_REGISTER, WIDTHS[TYPE_INTEGER], 0, labels[0]);
					q = createQ(q, OPERATOR_DEFAULT, 0, 0, 0, 0, _code->var, OPERAND_REGISTER, 0, 0, 0);
				}
			}else
				q = createQ(q, operator, codes[0]->var, codes[0]->type, codes[0]->size, codes[0]->memory, 0, getLabel(), _type->basic == TYPE_BOOLEAN, 0, 0);

			// New line at end of print
			if(operator == OPERATOR_PRINT)
				q = createQ(q, OPERATOR_PRINT, '\n', OPERAND_CONSTANT, 0, 0, 0, 0, 0, 1, 0);
			break;

		case CONSTRUCT_LOOP:
			switch(tree->totalChildren){
				case 2:
					// while
					q = createLabel(q, OPERATOR_JMP, currentLabel);
					q = createLabel(q, OPERATOR_DEFAULT, labels[2]);
					break;
				case 4:
					// for
					// INC or DEC depending for loop variable on range
					if(atoi(tree->children[1]->lexeme) < atoi(tree->children[2]->lexeme)){
						q = createQ(q, OPERATOR_INC, codes[0]->var, codes[0]->type, codes[0]->size, codes[0]->memory, 0, 0, 0, 0, 0);
						q = createQ(q, OPERATOR_LE, codes[0]->var, codes[0]->type, codes[0]->size, codes[0]->memory, codes[2]->var, codes[2]->type, codes[2]->size, codes[2]->memory, label);
					}else{
						q = createQ(q, OPERATOR_DEC, codes[0]->var, codes[0]->type, codes[0]->size, codes[0]->memory, 0, 0, 0, 0, 0);
						q = createQ(q, OPERATOR_GE, codes[0]->var, codes[0]->type, codes[0]->size, codes[0]->memory, codes[2]->var, codes[2]->type, codes[2]->size, codes[2]->memory, label);
					}
					break;
			}
			break;

		case CONSTRUCT_REUSE_MODULE:

			// Pushing parameters

			if(tree->totalChildren == 3){
				// Output
				tempTree = tree->children[0];

				// Converting linked list to array
				new = (ast)malloc(sizeof(struct _ast));
				new->totalChildren = 0;
				new->children = (ast *)malloc(sizeof(struct _ast) * new->totalChildren);

				while(tempTree){
					new->children = (ast *)realloc(new->children, sizeof(ast) * (new->totalChildren + 1));
					new->children[new->totalChildren++] = tempTree;
					tempTree = tempTree->next;
				}

				// Reverse order
				for(int i = new->totalChildren - 1; i >= 0; i--)
					q = createQ(q, OPERATOR_PUSH, 0, OPERAND_CONSTANT, new->children[i]->entry->width, 0, 0, 0, 0, 0, 0);

				free(new->children);
				free(new);
			}

			// Input
			tempTree = tree->children[tree->totalChildren - 1];

			// Converting linked list to array
			new = (ast)malloc(sizeof(struct _ast));
			new->totalChildren = 0;
			new->children = (ast *)malloc(sizeof(struct _ast) * new->totalChildren);
			while(tempTree){
				new->children = (ast *)realloc(new->children, sizeof(ast) * (new->totalChildren + 1));
				new->children[new->totalChildren++] = tempTree;
				tempTree = tempTree->next;
			}

			// Reverse order
			for(int i = new->totalChildren - 1; i >= 0; i--){
				tempCode = generateCode(new->children[i], 0);

				if(new->children[i]->type->basic == TYPE_ARRAY){
					// Push start and end values
					_type = new->children[i]->type;
					if(_type->endLexeme[0]){
						entryEnd = getIdInSymbolTable(_type->endLexeme, tree->table);
						q = createQ(q, OPERATOR_PUSH, entryEnd->offset, OPERAND_VARIABLE, entryEnd->width, 1, 0, 0, 0, 0, 0);
					}else
						q = createQ(q, OPERATOR_PUSH, _type->end, OPERAND_CONSTANT, WIDTHS[TYPE_INTEGER], 0, 0, 0, 0, 0, 0);

					if(_type->startLexeme[0]){
						entryStart = getIdInSymbolTable(_type->startLexeme, tree->table);
						q = createQ(q, OPERATOR_PUSH, entryStart->offset, OPERAND_VARIABLE, entryStart->width, 1, 0, 0, 0, 0, 0);
					}else
						q = createQ(q, OPERATOR_PUSH, _type->start, OPERAND_CONSTANT, WIDTHS[TYPE_INTEGER], 0, 0, 0, 0, 0, 0);
				}

				q = createQ(q, OPERATOR_PUSH, tempCode->var, tempCode->type, tempCode->size, tempCode->memory, 0, 0, 0, 0, 0);
				free(tempCode);
			}

			// Function call
			q = createLabel(q, OPERATOR_CALL, tree->children[tree->totalChildren - 2]->entry->offset);

			// Popping parameters

			// Input
			for(int i = 0; i < new->totalChildren; i++){
				q = createQ(q, OPERATOR_POP, getVar(), OPERAND_REGISTER, WIDTHS[new->children[i]->type->basic], 0, 0, 0, 0, 0, 0);
				q = createQ(q, OPERATOR_DEFAULT, 0, 0, 0, 0, var, OPERAND_REGISTER, 0, 0, 0);

				if(new->children[i]->type->basic == TYPE_ARRAY){
					// start and end
					q = createQ(q, OPERATOR_POP, getVar(), OPERAND_REGISTER, WIDTHS[TYPE_INTEGER], 0, 0, 0, 0, 0, 0);
					q = createQ(q, OPERATOR_DEFAULT, 0, 0, 0, 0, var, OPERAND_REGISTER, 0, 0, 0);
					q = createQ(q, OPERATOR_POP, getVar(), OPERAND_REGISTER, WIDTHS[TYPE_INTEGER], 0, 0, 0, 0, 0, 0);
					q = createQ(q, OPERATOR_DEFAULT, 0, 0, 0, 0, var, OPERAND_REGISTER, 0, 0, 0);
				}
			}
			free(new->children);
			free(new);

			if(tree->totalChildren == 3){
				// Output
				tempTree = tree->children[0];
				while(tempTree){
					q = createQ(q, OPERATOR_POP, tempTree->entry->offset, OPERAND_VARIABLE, tempTree->entry->width, 1, 0, 0, 0, 0, 0);
					tempTree = tempTree->next;
				}
			}
			break;

		case CONSTRUCT_ENTRY:
			// Freeing stack and quitting
			q = createQ(q, OPERATOR_ASSIGN, ESP, OPERAND_REGISTER, 0, 0, EBP, OPERAND_REGISTER, 0, 0, 0);
			q = createLabel(q, OPERATOR_END, 0);
			break;

		case CONSTRUCT_MODULE:
			// Freeing stack, popping EBP and returning
			q = createQ(q, OPERATOR_ASSIGN, ESP, OPERAND_REGISTER, 0, 0, EBP, OPERAND_REGISTER, 0, 0, 0);
			q = createQ(q, OPERATOR_POP, EBP, OPERAND_REGISTER, 0, 0, 0, 0, 0, 0, 0);
			q = createLabel(q, OPERATOR_RETURN, 0);
			break;
	}

	// Freeing codes (as instructions are linearized, we need only first instruction)
	if(tree->construct != CONSTRUCT_SWITCH)
		for(int i = 0; i < tree->totalChildren; i++)
			free(codes[i]);

	_code->start = q;
	return _code;

}
