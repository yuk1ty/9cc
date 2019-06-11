#include "9cc.h"

Token *new_token(int ty, char *input) {
    Token *token = malloc(sizeof(Token));
    token->ty = ty;
    token->input = input;
    return token;
}

Token *new_token_val(int ty, int val, char *input) {
    Token *token = malloc(sizeof(Token));
    token->ty = ty;
    token->val = val;
    token->input = input;
    return token;
}

int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
            ('A' <= c && c <= 'Z') ||
            ('0' <= c && c <= '9') ||
            (c == '_');
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
                || *p == '(' || *p == ')' || *p == '>' || *p == '<' 
                || *p == ';' || *p == '=') {
            Token *token = new_token(*p, p);
            vec_push(tokens, (void *) token);
            i++;
            p++;
            continue;
        }

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            Token *token = new_token(TK_RETURN, p);
            vec_push(tokens, (void *) token);
            i++;
            p += 6;
            continue;
        }

        if ('a' <= *p && *p <= 'z') {
            Token *token = new_token(TK_IDENT, p);
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

Node *new_node_ident(char name) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_IDENT;
    node->name = name;
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

Node *assign();
Node *stmt();
Node *equality();
Node *rational();
Node *add();
Node *mul();
Node *term();
Node *unary();

void program() {
    int i = 0;
    while(((Token *)tokens->data[pos])->ty != TK_EOF) {
        code[i++] = stmt();
    }
    code[i] = NULL;
}

Node *assign() {
    Node *node = equality();
    if (consume('=')) {
        node = new_node('=', node, assign());
    }
    return node;
}

Node *expr() {
    Node *node = assign();
    return node;
}

Node *stmt() {
    Node *node = expr();
    if (!consume(';')) {
        error_at(((Token *)tokens->data[pos])->input, "';'ではないトークンです");
    }
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

    if (t->ty == TK_IDENT) {
        Token *nt = tokens->data[pos++];
        return new_node_ident(nt->input[0]); // TODO 違うかも？
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