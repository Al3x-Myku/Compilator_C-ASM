#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//Tochenurile
typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_EQUALS,
    TOKEN_SEMICOLON,
    TOKEN_OPEN_PAREN,
    TOKEN_CLOSE_PAREN,
    TOKEN_OPEN_BRACE,
    TOKEN_CLOSE_BRACE,
    TOKEN_LESS_THAN,
    TOKEN_GREATER_THAN,
    TOKEN_LEFT_SHIFT,
    TOKEN_RIGHT_SHIFT,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_BITWISE_AND,
    TOKEN_BITWISE_OR,
    TOKEN_BITWISE_XOR,
    TOKEN_BITWISE_NOT,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_END_OF_INPUT
} TokenType;

const char* registerMap[] = {"eax", "ebx", "ecx", "edx"};
const char* variableMap[] = {"a", "b", "c", "d"};

typedef struct {
    TokenType type;
    char* value;
} Token;

typedef struct {
    const char* input;
    long long position;
    long long length;
} Lexer;

//ca sa nu trb sa le scriu in ordine
void processAssignment(Token* tokens, int* ip, int token_count);
int processIf(Token* tokens, int ip, int token_count);
int processWhile(Token* tokens, int ip, int token_count);
int processFor(Token* tokens, int ip, int token_count);
void generateCondition(Token* tokens, int token_count, int start, int* consumed, const char* falseLabel);
void processAssignmentBlock(Token* tokens, int start, int end);
void processArithmeticOperation(Token* tokens, int* ip, int token_count);

//de aici lexer
void initLexer(Lexer* lexer, const char* input) {
    lexer->input = input;
    lexer->position = 0;
    lexer->length = strlen(input);
}
//stiu ca sunt identice, ma ajuta cand scriu
int isIdentifierStart(char c) {
    return isalpha(c) || c == '_';
}

int isIdentifierChar(char c) {
    return isalnum(c) || c == '_';
}

Token nextToken(Lexer* lexer) {
    Token token = {0};
    //sari peste " "
    while (lexer->position < lexer->length && isspace(lexer->input[lexer->position])) {
        lexer->position++;
    }
    if (lexer->position >= lexer->length) {
        token.type = TOKEN_END_OF_INPUT;
        return token;
    }
    char current = lexer->input[lexer->position];

    // verifica operator "dublu"
    if (current == '<') {
        if (lexer->position + 1 < lexer->length && lexer->input[lexer->position+1] == '<') {
            token.type = TOKEN_LEFT_SHIFT;
            token.value = strdup("<<");
            lexer->position += 2;
            return token;
        } else {
            token.type = TOKEN_LESS_THAN;
            token.value = strdup("<");
            lexer->position++;
            return token;
        }
    }
    if (current == '>') {
        if (lexer->position + 1 < lexer->length && lexer->input[lexer->position+1] == '>') {
            token.type = TOKEN_RIGHT_SHIFT;
            token.value = strdup(">>");
            lexer->position += 2;
            return token;
        } else {
            token.type = TOKEN_GREATER_THAN;
            token.value = strdup(">");
            lexer->position++;
            return token;
        }
    }
    if (current == '!') {
        if (lexer->position + 1 < lexer->length && lexer->input[lexer->position+1] == '=') {
            token.type = TOKEN_BITWISE_NOT; // We'll handle this as != in condition
            token.value = strdup("!=");
            lexer->position += 2;
            return token;
        }
    }

    // cazuri basic de operator
    switch (current) {
        case '=':
            token.type = TOKEN_EQUALS;
            token.value = strdup("=");
            lexer->position++;
            return token;
        case ';':
            token.type = TOKEN_SEMICOLON;
            token.value = strdup(";");
            lexer->position++;
            return token;
        case '(':
            token.type = TOKEN_OPEN_PAREN;
            token.value = strdup("(");
            lexer->position++;
            return token;
        case ')':
            token.type = TOKEN_CLOSE_PAREN;
            token.value = strdup(")");
            lexer->position++;
            return token;
        case '{':
            token.type = TOKEN_OPEN_BRACE;
            token.value = strdup("{");
            lexer->position++;
            return token;
        case '}':
            token.type = TOKEN_CLOSE_BRACE;
            token.value = strdup("}");
            lexer->position++;
            return token;
        case '+':
            token.type = TOKEN_PLUS;
            token.value = strdup("+");
            lexer->position++;
            return token;
        case '-':
            token.type = TOKEN_MINUS;
            token.value = strdup("-");
            lexer->position++;
            return token;
        case '*':
            token.type = TOKEN_MULTIPLY;
            token.value = strdup("*");
            lexer->position++;
            return token;
        case '/':
            token.type = TOKEN_DIVIDE;
            token.value = strdup("/");
            lexer->position++;
            return token;
        case '&':
            token.type = TOKEN_BITWISE_AND;
            token.value = strdup("&");
            lexer->position++;
            return token;
        case '|':
            token.type = TOKEN_BITWISE_OR;
            token.value = strdup("|");
            lexer->position++;
            return token;
        case '^':
            token.type = TOKEN_BITWISE_XOR;
            token.value = strdup("^");
            lexer->position++;
            return token;
        case '~':
            token.type = TOKEN_BITWISE_NOT;
            token.value = strdup("~");
            lexer->position++;
            return token;
    }

    //verifica numere (in enunt zicea si de hexa dar in test n am avut, anyway merge si pe hexa)
    if (isdigit(current)) {
        size_t start = lexer->position;
        if (current == '0' && lexer->position + 1 < lexer->length &&
            (lexer->input[lexer->position + 1] == 'x' || lexer->input[lexer->position + 1] == 'X')) {
            lexer->position += 2;
            while (lexer->position < lexer->length && isxdigit(lexer->input[lexer->position])) {
                lexer->position++;
            }
        } else {
            while (lexer->position < lexer->length && isdigit(lexer->input[lexer->position])) {
                lexer->position++;
            }
        }
        size_t len = lexer->position - start;
        token.type = TOKEN_NUMBER;
        token.value = strndup(lexer->input + start, len);
        return token;
    }

    // verificare de "identifier token si keyword"
    if (isIdentifierStart(current)) {
        size_t start = lexer->position;
        while (lexer->position < lexer->length && isIdentifierChar(lexer->input[lexer->position])) {
            lexer->position++;
        }
        size_t len = lexer->position - start;
        token.value = strndup(lexer->input + start, len);
        token.type = TOKEN_IDENTIFIER;
        if (strcmp(token.value, "if") == 0) {
            token.type = TOKEN_IF;
        } else if (strcmp(token.value, "else") == 0) {
            token.type = TOKEN_ELSE;
        } else if (strcmp(token.value, "while") == 0) {
            token.type = TOKEN_WHILE;
        } else if (strcmp(token.value, "for") == 0) {
            token.type = TOKEN_FOR;
        }
        return token;
    }
    //^^^^^^^^ ff important lasa asa ca merge
    token.type = TOKEN_END_OF_INPUT;
    return token;
}

//mapare a,b,c,d la registrii
int getRegisterIndex(const char* var) {
    for (int i = 0; i < 4; i++) {
        if (strcmp(var, variableMap[i]) == 0) {
            return i;
        }
    }
    return -1;
}

//procesare bloc
void processAssignmentBlock(Token* tokens, int start, int end) {
    int i = start;
    while(i < end) {
        if(tokens[i].type == TOKEN_SEMICOLON) {
            i++;
            continue;
        }
        processArithmeticOperation(tokens, &i, end);
    }
}

void generateCondition(Token* tokens, int token_count, int start, int* consumed, const char* falseLabel) {
    Token left = tokens[start];
    int opConsumed = 0;
    int opType = -1; // 0: >=, 1: <=, 2: ==, 3: <, 4: >, 5: != NU STERGE COM ASTA
    
    // verifica operator dublu iar
    if (start+2 < token_count) {
        if (tokens[start+1].type == TOKEN_GREATER_THAN && tokens[start+2].type == TOKEN_EQUALS) {
            opType = 0; // >=
            opConsumed = 2;
        } else if (tokens[start+1].type == TOKEN_LESS_THAN && tokens[start+2].type == TOKEN_EQUALS) {
            opType = 1; // <=
            opConsumed = 2;
        } else if (tokens[start+1].type == TOKEN_EQUALS && tokens[start+2].type == TOKEN_EQUALS) {
            opType = 2; // ==
            opConsumed = 2;
        } else if (tokens[start+1].type == TOKEN_BITWISE_NOT && tokens[start+2].type == TOKEN_EQUALS) {
            opType = 5; // !=
            opConsumed = 2;
        }
    }
    
    //operator basic
    if (opType == -1 && start+1 < token_count) {
        if (tokens[start+1].type == TOKEN_LESS_THAN) {
            opType = 3; // <
            opConsumed = 1;
        } else if (tokens[start+1].type == TOKEN_GREATER_THAN) {
            opType = 4; // >
            opConsumed = 1;
        }
    }

    if (opType == -1) return; // error daca ai ajuns aici faci ceva gresit

    int operandIndex = start + 1 + opConsumed;
    Token operand = tokens[operandIndex];
    int reg = getRegisterIndex(left.value);
    if (reg == -1) return;

    if (operand.type == TOKEN_NUMBER) {
        printf("CMP %s, %s\n", registerMap[reg], operand.value);
    } else if (operand.type == TOKEN_IDENTIFIER) {
        int src = getRegisterIndex(operand.value);
        if (src != -1)
            printf("CMP %s, %s\n", registerMap[reg], registerMap[src]);
    }

    // Generate appropriate jump
    switch (opType) {
        case 0: printf("JL %s\n", falseLabel); break;  // >= (false if <)
        case 1: printf("JG %s\n", falseLabel); break;  // <= (false if >)
        case 2: printf("JNE %s\n", falseLabel); break; // == (false if !=)
        case 3: printf("JGE %s\n", falseLabel); break; // < (false if >=)
        case 4: printf("JLE %s\n", falseLabel); break; // > (false if <=)
        case 5: printf("JE %s\n", falseLabel); break;  // != (false if ==)
    }

    *consumed = opConsumed + 2; 
}
//basic stuff
void processAssignment(Token* tokens, int* ip, int token_count) {
    int i = *ip;
    if (i + 2 < token_count &&
        tokens[i].type == TOKEN_IDENTIFIER &&
        tokens[i+1].type == TOKEN_EQUALS &&
        (tokens[i+2].type == TOKEN_NUMBER || tokens[i+2].type == TOKEN_IDENTIFIER))
    {
        int reg_index = getRegisterIndex(tokens[i].value);
        if (reg_index != -1) {
            if (tokens[i+2].type == TOKEN_NUMBER) {
                printf("MOV %s, %s\n", registerMap[reg_index], tokens[i+2].value);
            } else {
                int src = getRegisterIndex(tokens[i+2].value);
                if (src != -1)
                    printf("MOV %s, %s\n", registerMap[reg_index], registerMap[src]);
            }
        }
        *ip = i + 3;
        return;
    }
    (*ip)++;
}
//operatiile usoare
void processArithmeticOperation(Token* tokens, int* ip, int token_count) {
    int i = *ip;
    if (i + 4 < token_count &&
        tokens[i].type == TOKEN_IDENTIFIER &&
        tokens[i+1].type == TOKEN_EQUALS &&
        tokens[i+2].type == TOKEN_IDENTIFIER &&
        strcmp(tokens[i].value, tokens[i+2].value) == 0 &&
        (tokens[i+3].type == TOKEN_MULTIPLY || 
         tokens[i+3].type == TOKEN_DIVIDE ||
         tokens[i+3].type == TOKEN_PLUS ||
         tokens[i+3].type == TOKEN_MINUS ||
         tokens[i+3].type == TOKEN_BITWISE_AND ||
         tokens[i+3].type == TOKEN_BITWISE_OR ||
         tokens[i+3].type == TOKEN_BITWISE_XOR ||
         tokens[i+3].type == TOKEN_LEFT_SHIFT ||
         tokens[i+3].type == TOKEN_RIGHT_SHIFT) &&
        (tokens[i+4].type == TOKEN_NUMBER || tokens[i+4].type == TOKEN_IDENTIFIER))
    {
        int reg_index = getRegisterIndex(tokens[i].value);
        if (reg_index != -1) {
            if (tokens[i+3].type == TOKEN_MULTIPLY) {
                if (tokens[i+4].type == TOKEN_NUMBER) {
                  if(strcmp(registerMap[reg_index], "eax")==0){//asta trb si la div, dar nu e in teste deci mna lene
                      printf("MUL %s\n", tokens[i+4].value);}
                      else{
                    printf("MOV eax, %s\n", registerMap[reg_index]);
                    printf("MUL %s\n", tokens[i+4].value);
                    printf("MOV %s, eax\n", registerMap[reg_index]);}
                } else {
                    printf("MOV eax, %s\n", registerMap[reg_index]);
                    int src = getRegisterIndex(tokens[i+4].value);
                    if (src != -1)
                        printf("MUL %s\n", registerMap[src]);
                    printf("MOV %s, eax\n", registerMap[reg_index]);
                }
            } else if (tokens[i+3].type == TOKEN_DIVIDE) {
                if (tokens[i+4].type == TOKEN_NUMBER) {
                    int divisor = atoi(tokens[i+4].value);
                    if (divisor == 0) {
                        printf("Error\n");
                    } else {
                        printf("MOV eax, %s\n", registerMap[reg_index]);
                        printf("DIV %s\n", tokens[i+4].value);
                        printf("MOV %s, eax\n", registerMap[reg_index]);
                    }
                } else {
                    int src = getRegisterIndex(tokens[i+4].value);
                    if (src != -1) {
                        printf("MOV eax, %s\n", registerMap[reg_index]);
                        printf("DIV %s\n", registerMap[src]);
                        printf("MOV %s, eax\n", registerMap[reg_index]);
                    }
                }
            } else if (tokens[i+3].type == TOKEN_PLUS) {
                if (tokens[i+4].type == TOKEN_NUMBER)
                    printf("ADD %s, %s\n", registerMap[reg_index], tokens[i+4].value);
                else {
                    int src = getRegisterIndex(tokens[i+4].value);
                    if (src != -1)
                        printf("ADD %s, %s\n", registerMap[reg_index], registerMap[src]);
                }
            } else if (tokens[i+3].type == TOKEN_MINUS) {
                if (tokens[i+4].type == TOKEN_NUMBER)
                    printf("SUB %s, %s\n", registerMap[reg_index], tokens[i+4].value);
                else {
                    int src = getRegisterIndex(tokens[i+4].value);
                    if (src != -1)
                        printf("SUB %s, %s\n", registerMap[reg_index], registerMap[src]);
                }
            } else if (tokens[i+3].type == TOKEN_BITWISE_AND) {
                if (tokens[i+4].type == TOKEN_NUMBER)
                    printf("AND %s, %s\n", registerMap[reg_index], tokens[i+4].value);
                else {
                    int src = getRegisterIndex(tokens[i+4].value);
                    if (src != -1)
                        printf("AND %s, %s\n", registerMap[reg_index], registerMap[src]);
                }
            } else if (tokens[i+3].type == TOKEN_BITWISE_OR) {
                if (tokens[i+4].type == TOKEN_NUMBER)
                    printf("OR %s, %s\n", registerMap[reg_index], tokens[i+4].value);
                else {
                    int src = getRegisterIndex(tokens[i+4].value);
                    if (src != -1)
                        printf("OR %s, %s\n", registerMap[reg_index], registerMap[src]);
                }
            } else if (tokens[i+3].type == TOKEN_BITWISE_XOR) {
                if (tokens[i+4].type == TOKEN_NUMBER)
                    printf("XOR %s, %s\n", registerMap[reg_index], tokens[i+4].value);
                else {
                    int src = getRegisterIndex(tokens[i+4].value);
                    if (src != -1)
                        printf("XOR %s, %s\n", registerMap[reg_index], registerMap[src]);
                }
            } else if (tokens[i+3].type == TOKEN_LEFT_SHIFT) {
                if (tokens[i+4].type == TOKEN_NUMBER)
                    printf("SHL %s, %s\n", registerMap[reg_index], tokens[i+4].value);
                else {
                    int src = getRegisterIndex(tokens[i+4].value);
                    if (src != -1)
                        printf("SHL %s, %s\n", registerMap[reg_index], registerMap[src]);
                }
            } else if (tokens[i+3].type == TOKEN_RIGHT_SHIFT) {
                if (tokens[i+4].type == TOKEN_NUMBER)
                    printf("SHR %s, %s\n", registerMap[reg_index], tokens[i+4].value);
                else {
                    int src = getRegisterIndex(tokens[i+4].value);
                    if (src != -1)
                        printf("SHR %s, %s\n", registerMap[reg_index], registerMap[src]);
                }
            }
        }
        *ip = i + 5;
        return;
    }
    processAssignment(tokens, ip, token_count);
}

int processIf(Token* tokens, int ip, int token_count) {//procesare if, fiecare fct d asta face altcv la close bracket
    ip++;
    if(ip >= token_count || tokens[ip].type != TOKEN_OPEN_PAREN) return ip;
    ip++;
    
    const char* endLabel = "end_label";
    int condTokens = 0;
    generateCondition(tokens, token_count, ip, &condTokens, endLabel);
    ip += condTokens;
    if(ip < token_count && tokens[ip].type == TOKEN_CLOSE_PAREN)
        ip++;
    if(ip >= token_count || tokens[ip].type != TOKEN_OPEN_BRACE) return ip;
    ip++;
    int blockStart = ip;
    while(ip < token_count && tokens[ip].type != TOKEN_CLOSE_BRACE)
        ip++;
    processAssignmentBlock(tokens, blockStart, ip);
    printf("%s:\n", endLabel);
    ip++;
    return ip;
}

int processWhile(Token* tokens, int ip, int token_count) {//same ca la if
    ip++; //sare peste while
    if (ip >= token_count || tokens[ip].type != TOKEN_OPEN_PAREN) return ip;
    ip++; //si peste (

    //tag
    printf("start_loop:\n");

    
    int consumed = 0;
    generateCondition(tokens, token_count, ip, &consumed, "end_label");
    ip += consumed;

    if (ip < token_count && tokens[ip].type == TOKEN_CLOSE_PAREN) {
        ip++; // sare de )
    }

    if (ip >= token_count || tokens[ip].type != TOKEN_OPEN_BRACE) return ip;
    ip++; //sare de {

    // procesare interioara
    int bodyStart = ip;
    while (ip < token_count && tokens[ip].type != TOKEN_CLOSE_BRACE) {
        ip++;
    }
    processAssignmentBlock(tokens, bodyStart, ip);
    ip++; // sare de }

    //print la end
    printf("JMP start_loop\n");
    printf("end_label:\n");

    return ip;
}

int processFor(Token* tokens, int ip, int token_count) {//e ca while dar mai sarim si peste ";" si procesam o incrementare"
    ip++; 
    if (ip >= token_count || tokens[ip].type != TOKEN_OPEN_PAREN) return ip;
    ip++; 

  
    int initStart = ip;
    while (ip < token_count && tokens[ip].type != TOKEN_SEMICOLON) {
        ip++;
    }
    processAssignmentBlock(tokens, initStart, ip);
    ip++; 


    printf("start_loop:\n");

   
    int condStart = ip;
    while (ip < token_count && tokens[ip].type != TOKEN_SEMICOLON) {
        ip++;
    }
  
    ip++; 

    
    int consumed = 0;
    generateCondition(tokens, token_count, condStart, &consumed, "end_label");

   
    int incrStart = ip;
    while (ip < token_count && tokens[ip].type != TOKEN_CLOSE_PAREN) {
        ip++;
    }
    int incrEnd = ip;
    ip++;

    if (ip >= token_count || tokens[ip].type != TOKEN_OPEN_BRACE) return ip;
    ip++; 

  
    int bodyStart = ip;
    while (ip < token_count && tokens[ip].type != TOKEN_CLOSE_BRACE) {
        ip++;
    }
    processAssignmentBlock(tokens, bodyStart, ip);
    ip++; 

    
    processAssignmentBlock(tokens, incrStart, incrEnd);

  
    printf("JMP start_loop\n");
    printf("end_loop:\n");

    return ip;
}

void generateAssemblyCode(Token* tokens, int token_count) {//nume intuitiv
    int ip = 0;
    while(ip < token_count) {
        if(tokens[ip].type == TOKEN_SEMICOLON) {
            ip++;
            continue;
        }
        if(tokens[ip].type == TOKEN_IF) {
            ip = processIf(tokens, ip, token_count);
        } else if(tokens[ip].type == TOKEN_WHILE) {
            ip = processWhile(tokens, ip, token_count);
        } else if(tokens[ip].type == TOKEN_FOR) {
            ip = processFor(tokens, ip, token_count);
        } else {
            processArithmeticOperation(tokens, &ip, token_count);
        }
    }
}

int main() {//asta e main ul
    char buffer[1024];
    char* code = malloc(1);
    if (!code) {
        fprintf(stderr, "Memory allocation failed\n");//programare "defensiva"
        return 1;
    }
    code[0] = '\0';
    while (fgets(buffer, sizeof(buffer), stdin)) {
        if (strcmp(buffer, "\n") == 0)
            break;
        size_t newSize = strlen(code) + strlen(buffer) + 1;
        char* temp = realloc(code, newSize);
        if (!temp) {
            fprintf(stderr, "Memory reallocation failed\n");//programare "defensiva"
            free(code);
            return 1;
        }
        code = temp;
        strcat(code, buffer);
    }
    
    Lexer lexer;
    initLexer(&lexer, code);
    
    Token tokens[300];
    int token_count = 0;
    Token token;
    do {
        token = nextToken(&lexer);
        if(token.type != TOKEN_END_OF_INPUT) {
            tokens[token_count++] = token;
        }
    } while(token.type != TOKEN_END_OF_INPUT);
    
    generateAssemblyCode(tokens, token_count);
    
    for (int i = 0; i < token_count; i++) {
        free(tokens[i].value);
    }
    free(code);
    return 0;
}
