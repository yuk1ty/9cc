#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Token
typedef struct {
    int ty;
    int val;
    char *input; // for error handling
} Token;

Token *new_token(int ty, char *input);
Token *new_token_val(int ty, int val, char *input);

enum {
    TK_NUM = 256,
    TK_IDENT, // identifiers
    TK_EQ, // ==
    TK_NE, // !=
    TK_LE, // <=
    TK_GE, // >=
    TK_RETURN, // return
    TK_EOF,
};

// Vector
typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

Vector *tokens;
Vector *new_vector();
void vec_push(Vector *vec, void *elem);

// Node
typedef struct Node {
    int ty;
    struct Node *lhs;
    struct Node *rhs;
    int val;
    char name;
} Node;

Node *code[100];
Node *expr();
void program();

enum {
    ND_NUM = 256,
    ND_IDENT,
};

// tests
void error(char *fmt, ...);
void error_at(char *loc, char *msg);
int expect(int line, int expected, int actual);
void runtest();

// others
int pos;
char *user_input;
void gen(Node *node);
void tokenize();