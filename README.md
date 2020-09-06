# Compiler for ERPLAG language

##### Term project for the course Compiler Construction (CS F363)

## Desciption

This compiler consists of an LL(1) grammar, lexer module to create tokens from input string, a recursive descent parser module, abstract syntax tree (AST) module to create AST which is a heavily compressed (around 85%) version of parse tree, symbol table module which creates tree of symbol tables for identifiers and performs declaration checks, semantic analysizer module which performs most of the semantic checks, and a codegen module which converts AST to an intermediate linear code and then outputs to file.

Code produced is 32-bit NASM assembly language code.

The language specifications are provided in pdf format (the grammar provided in that pdf is incomplete. The complete version is available as `grammar.txt` and is read by the grammar module for creating parse tree).

Passes of various data structures are
- Grammar file: ~3 (can be reduced to 1)
- Semantics file: 1 - to create semantics
- Input source code: 1 - to read buffer from storage to memory
- Source buffer: 1 - to create tokens and parse tree
- Parse tree: 1 - to create AST
- AST: 3 - to create symbol table, to perform semantic checks and to create code
- Code: 1 - to save to file

## Installation and running

The compiler is written in `C` and hence requires `gcc` (version 5.4.0), which comes pre-installed in most Linux installations. It produces assembly language code which can be run using `NASM` which can be installed using `sudo apt-get install nasm` (version 2.14.02)

The compiler can be compiled and run using
- `make`
- `./compiler code.txt code.asm`

The compiled code can then be executed using
- `nasm -f elf code.asm`
- `ld -m elf_i386 -o code code.o`
- `./code`

## Additional information (apart from language specifications)

- The grammar has operator precedence built in it as follows (from highest to lowest)
  - Brackets `( )`
  - Unary minus `-`
  - Multiply and divide `* \`
  - Plus and binary minus `+ -`
  - Relational operators `< <= > >= == !=`
  - Logical operators `AND OR`
- While the grammar and corresponding parse tree is right associative (to remove left recursion), the AST constructed is transformed and is left associative.
- A semantically invalid assignment is not considered as assigning to a variable for further semantic checks.
- The for loop variable increments automatically and it decrements too if num2 is smaller than num1. It will execute once if num1 and num2 are equal. So, for loop will always have a valid execution if it is semantically correct at compile time. At the end of execution, its value is num2 +/- 1.
- Run time bound checking and index calculation of array input parameter (static or dynamic) is done using range passed by caller and not the static range available, if any, at compile time (although, static bound checking and type checking is done at compile time). So, passing a dynamic array in place of a static array is semantically, and run-time correct, and will give bound error if its index is out of its dynamic range (even if it was in the parameter’s static range). The only function call which is semantically invalid is if both actual and formal parameters are static arrays and have different ranges.
- If any dynamic array is used as an input parameter, its ranges specified in input list can be used in function body, and will be mapped to correct values on function call.
- Value of function output parameters are assigned zero at starting of function, and if that function does not assign any value to it during its execution, its value is returned as zero to callee.
- An array’s first element is farthest from the array’s memory location. Hence, dynamic array address is assigned after increasing size of stack, to new ESP.
- Logical AND and OR are implemented as short circuit instructions.
- Boolean values can only be input as 0 or 1. Any other input gives run-time error.
- Range of dynamic array is also checked at run-time, and gives an error if high < low.

## Files
1. **languageDef.h -** definitions of various constants specific to the language

1. **grammarDef.h -** definition of grammar, rule and semantic data types
1. **grammar.c -** creates grammar rules and semantics from their respective files

1. **lexerDef.h -** definitions of lexer, token data types and associated constants
1. **lexer.c -** implementation of lexer

1. **parserDef.h -** definitions of parse tree, parse table, first and follow sets
1. **parser.c -** creates first and follow sets, parse table, and parses the input source code (by calling the lexer functions)

1. **astDef.h -** definitions of AST data type
1. **ast.c -** creates AST from the parse tree by performing semantic rules

1. **symbolTableDef.h -** definitions of symbol table and its entries data types
1. **symbolTable.c -** creates symbol table and functions to search in it

1. **semantics.c -** semantic analyzer for the AST

1. **codegenDef.h -** data types for code and quadruple representation
1. **codegen.c -** creates code from AST and symbol table and saves to file

1. **main.c -** entry point for running the compiler

1. **grammar.txt -** grammar for the language. Non-terminals are in angle brackets, terminals are capitalised, and `e` is the special null terminal.
1. **semantics.txt -** semantic rules for the language. Each line has semantic rules corresponding to the rule present in the same line in grammar.txt.

1. **testcase -** folder contains test cases for errors (both syntax and semantic), as well as correct codes consisting of most of the language features.

## TO DO

1. Preprocessing
   - Read grammar file in lesser pass.
   - Automatically create terminals and non-terminals.
   - Storing grammar and semantics and parse table in memory (as they won't change during compiler execution).
   - Convert grammar terminals and non-terminals into hashmap `string -> symbol`
   - Create first and follow sets while reading grammar.
   - Change type semantic from `0, 1, 2,..` to `BOOLEAN, INTEGER, REAL,..`

1. Lexer
   - Create a union for `lexeme` and hence create operator type, true and false values, convert `NUM` and `RNUM` to integers and floats.

1. Parser
   - Remove symbol in parse tree (hence, free grammar earlier).
   - Create first and follow sets in 1 pass of grammar rules.
   - Clearing first and follow sets after creating parse table (insert follow for error recovery in parse table).
   - Correct error recovery.
   - Convert firstAndFollow into hashmap: `symbol -> set[2]`
   - Convert parseTable into hashmap: `symbol -> (symbol -> rule)`

1. AST creation
   - Change type data structure (possibly by creating a union).

1. Symbol table
   - Combine AST and symbol table creation pass.

1. Semantic analysis
   - `while(true)` should not be an error.
   - Evaluate constant expressions such as `2>3 AND true` at compile time.
   - Implement type casting for expressions such as `integer < real`

1. Code generation
   - Use `[EBP + EAX + 8]` kind of operands.
   - Implement real numbers.
   - Runtime input errors for integers.
   - Optimization.
   - Handle register overflow.
   - Make output 64 bit compatible.
   - Combine semantic analysis and codegen pass.
