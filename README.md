# C Lexer and Simple Code Generator

This project is a C-based lexer and a simple code generator that processes a minimalistic C-like syntax and outputs equivalent pseudo-assembly instructions. It supports variable assignments, arithmetic operations, control structures like `if`, `while`, and `for`, as well as bitwise operations.

## Features

- Lexical analysis of:
  - Identifiers
  - Numbers (decimal and hexadecimal)
  - Operators: `+`, `-`, `*`, `/`, `&`, `|`, `^`, `~`, `<<`, `>>`
  - Comparison: `==`, `!=`, `<`, `>`, `<=`, `>=`
  - Delimiters: `=`, `;`, `{}`, `()`
  - Keywords: `if`, `else`, `while`, `for`

- Token generation via a custom lexer
- Basic parsing of:
  - Assignments (e.g., `a = 5;`, `b = a;`)
  - Arithmetic (e.g., `a = a + 2;`, `b = b * c;`)
  - Conditional logic (e.g., `if (a > 5) { ... }`)
  - Loop logic (e.g., `while (...)`, `for (...)`)
- ASM-style instruction output (e.g., `MOV`, `ADD`, `MUL`, `CMP`, `JNE`, etc.)

## How It Works

- The `Lexer` scans the input string and creates a list of `Token` objects.
- Each token is categorized using `TokenType`.
- The code generator then interprets the token stream and emits ASM-like instructions for:
  - Assignments
  - Arithmetic expressions
  - Conditions (with `CMP` and conditional jumps like `JL`, `JNE`, etc.)
- Register mapping:
  - Variables `a`, `b`, `c`, `d` are mapped to registers `eax`, `ebx`, `ecx`, `edx`.

## Example Input

```c
a = 5;
b = a + 2;
if (a >= 3) {
    c = c << 1;
}
```
# 🧾 Example Output - Code Generator
```c
MOV eax, 5
MOV ebx, eax
ADD ebx, 2
CMP eax, 3
JL label_else
MOV ecx, ecx
SHL ecx, 1
label_else:
```
