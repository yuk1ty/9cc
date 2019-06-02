#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    TK_NUM = 256,
    TK_EQ, // ==
    TK_NE, // !=
    TK_LE, // <=
    TK_GE, // >=
    TK_EOF,
};

typedef struct {
    int ty;
    int val;
    char *input; // for error handling
} Token;

Token *new_token(int ty, char *input);

Token *new_token(int ty, char *input) {
    Token *token = malloc(sizeof(Token));
    token->ty = ty;
    token->input = input;
    return token;
}

Token *new_token_val(int ty, int val, char *input);

Token *new_token_val(int ty, int val, char *input) {
    Token *token = malloc(sizeof(Token));
    token->ty = ty;
    token->val = val;
    token->input = input;
    return token;
}

enum {
    ND_NUM = 256,
};

typedef struct Node {
    int ty;
    struct Node *lhs;
    struct Node *rhs;
    int val;
} Node;

typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

char *user_input;

Vector *tokens;
Vector *new_vector();
void vec_push(Vector *vec, void *elem);

int pos = 0;

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *msg) {
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, "");
    fprintf(stderr, "^ %s\n", msg);
    exit(1);
}

void tokenize() {
    tokens = new_vector();
    char *p = user_input;

    int i = 0;
    while (*p) {
        // skip whitespaces
        if (isspace(*p)) {
            p++; 
            continue;
        }

        if (strncmp(p, ">=", 2) == 0) {
            Token *token = new_token(TK_GE, p);
            vec_push(tokens, (void *) token);
            i++;
            p += 2; 
            continue;
        }
        
        if (strncmp(p, "<=", 2) == 0) {
            Token *token = new_token(TK_LE, p);
            vec_push(tokens, (void *) token);
            i++;
            p += 2;
            continue;
        }

        if (strncmp(p, "==", 2) == 0) {
            Token *token = new_token(TK_EQ, p);
            vec_push(tokens, (void *) token);
            i++;
            p += 2;
            continue;
        }

        if (strncmp(p, "!=", 2) == 0) {
            Token *token = new_token(TK_NE, p);
            vec_push(tokens, (void *) token);
            i++;
            p += 2;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' 
                || *p == '(' || *p == ')' || *p == '>' || *p == '<') {
            Token *token = new_token(*p, p);
            vec_push(tokens, (void *) token);
            i++;
            p++;
            continue;
        }

        if (isdigit(*p)) {
            Token *token = new_token_val(TK_NUM, strtol(p, &p, 10), p);
            vec_push(tokens, (void *) token);
            i++;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    Token *eof = new_token(TK_EOF, p);
    vec_push(tokens, (void *) eof);
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

int consume(int ty) {
    Token *t = tokens->data[pos];

    if (t->ty != ty) {
        return 0;
    }
    pos++;
    return 1;
}

Node *expr();
Node *equality();
Node *rational();
Node *add();
Node *mul();
Node *term();
Node *unary();

Node *expr() {
    Node *node = equality();
    return node;
}

Node *equality() {
    Node *node = rational();

    for (;;) {
        if (consume(TK_EQ)) {
            node = new_node(TK_EQ, node, rational());
        } else if (consume(TK_NE)) {
            node = new_node(TK_NE, node, rational());
        } else {
            return node;
        }
    }
}

Node *rational() {
    Node *node = add();

    for (;;) {
        if (consume('<')) {
            node = new_node('<', node, add());
        } else if (consume(TK_LE)) {
            node = new_node(TK_LE, node, add());
        } else if (consume('>')) {
            node = new_node('>', add(), node);
        } else if (consume(TK_GE)) {
            node = new_node(TK_GE, add(), node);
        } else {
            return node;
        }
    }
}

Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume('+')) {
            node = new_node('+', node, mul());
        } else if (consume('-')) {
            node = new_node('-', node, mul());
        } else {
            return node;
        }
    }
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume('*')) {
            node = new_node('*', node, unary());
        } else if (consume('/')) {
            node = new_node('/', node, unary());
        } else {
            return node;
        }
    }
}

Node *term() {
    Token *t = tokens->data[pos];

    if (consume('(')) {
        Node *node = expr();
        if (!consume(')')) {
            error_at(t->input, "開きカッコに対応する閉じカッコがありません");
        }
        return node;
    }

    if (t->ty == TK_NUM) {
        Token *nt = tokens->data[pos++];
        return new_node_num(nt->val);
    }

    error_at(t->input, "数値でも開きカッコでもないトークンです");
}

Node *unary() {
    if (consume('+')) {
        return term();
    }

    if (consume('-')) {
        return new_node('-', new_node_num(0), term());
    }

    return term();
}

void gen(Node *node) {
    if (node->ty == ND_NUM) {
        printf("    push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->ty) {
    case '+':
        printf("    add rax, rdi\n");
        break;
    case '-':
        printf("    sub rax, rdi\n");
        break;
    case '*':
        printf("    imul rdi\n");
        break;
    case '/':
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    case '<':
    case '>':
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case TK_LE:
    case TK_GE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    case TK_EQ:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case TK_NE:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    }

    printf("    push rax\n");
}

Vector *new_vector() {
    Vector *vec = malloc(sizeof(Vector));
    vec->data = malloc(sizeof(void *) * 16);
    vec->capacity = 16;
    vec->len = 0;
    return vec;
}

void vec_push(Vector *vec, void *elem) {
    if (vec->capacity == vec->len) {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
    }
    vec->data[vec->len++] = elem;
}

int expect(int line, int expected, int actual) {
    if (expected == actual) {
        return 1;
    }
    fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);
    exit(1);
}

void runtest() {
    Vector *vec = new_vector();
    expect(__LINE__, 0, vec->len);

    for (int i = 0; i < 100; i++) {
        vec_push(vec, (void *) i); // TODO erase warning
    }

    expect(__LINE__, 100, vec->len);
    expect(__LINE__, 0, (long)vec->data[0]);
    expect(__LINE__, 50, (long)vec->data[50]);
    expect(__LINE__, 99, (long)vec->data[99]);

    printf("OK\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    if (strcmp(argv[1], "-test") == 0) {
        runtest();
        return 0;
    }

    user_input = argv[1];
    tokenize();

    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    gen(node);

    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}