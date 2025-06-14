Mini C-to-Assembly Compiler
This project is a simple compiler written in C that translates a small, C-like programming language into basic x86 assembly-like instructions. It reads source code from standard input, tokenizes it, and then parses it to generate the corresponding assembly output to standard output.

Features
Lexical Analysis (Lexer): Breaks down the source code into a stream of tokens.

Syntactic Analysis (Parser): Interprets the token stream to understand the code's structure.

Variable Support: Supports four hardcoded variables: a, b, c, d, which are mapped to the eax, ebx, ecx, and edx registers, respectively.

Control Flow:

if statements

while loops

for loops

Operations:

Assignment: a = 10; or a = b;

Arithmetic: +, -, *, /

Bitwise: & (AND), | (OR), ^ (XOR), << (Left Shift), >> (Right Shift)

Relational Operators: <, >, ==, !=, <=, >= for use in control flow conditions.

Supported Syntax
Variable Assignment
a = 100;
b = a;

Operations
Operations must be in the variable = variable op value format.
a = a + 5;
b = b * c;
c = c << 2;
d = d & 0xFF;

if Statement
if (a > 10) {
a = a - 1;
}

while Loop
while (b < 100) {
b = b + 1;
}

for Loop
for (c = 0; c < 10; c = c + 1) {
a = a + c;
}

How to Use
Compilation
You will need a C compiler like GCC to build the program.
gcc your_source_file.c -o compiler
(Replace your_source_file.c with the name of your file.)

Execution
You can run the compiler in two ways:

Interactive Mode
Run the executable and type your code directly into the terminal. Press Enter on a blank line to signal the end of the input.
./compiler
a = 10;
b = 20;
if (a < b) {
  a = a + b;
}
# Press Enter on a blank line here

From a File
Save your code in a file (e.g., test.mc) and use a pipe to send the content to the compiler.
# Create a test file
echo "a = 5; while (a > 0) { a = a - 1; }" > test.mc
# Run the compiler with the file as input
cat test.mc | ./compiler

Example
Input Code (example.mc):
a = 10;
b = 0;
while (a > b) {
b = b + 1;
}

Execution:
cat example.mc | ./compiler

Generated Assembly Output:
MOV eax, 10
MOV ebx, 0
start_loop:
CMP eax, ebx
JLE end_label
ADD ebx, 1
JMP start_loop
end_label:

How It Works
Reading Input: The main function reads multiple lines of code from stdin until it encounters an empty line.

Lexing (Tokenization): The Lexer and nextToken function scan the input string and convert it into a series of Tokens (e.g., TOKEN_IDENTIFIER, TOKEN_NUMBER, TOKEN_OPERATOR).

Parsing and Code Generation:

The generateAssemblyCode function iterates through the tokens.

It identifies statement types (if, while, for, or assignment/operation).

It calls dedicated functions (processIf, processWhile, processAssignmentBlock, etc.) to generate the appropriate assembly instructions, which are printed to stdout.

Conditions within control structures are handled by generateCondition, which produces a CMP instruction followed by a conditional jump (JNE, JLE, etc.).

Limitations
Only four variables (a, b, c, d) are supported.

No support for user-defined functions.

Limited expression parsing. Operations are restricted to the var = var op value pattern (e.g., a = b + c; is not supported).

Error handling is minimal. Malformed code may result in unexpected behavior or no output.
