#include "expression.h"
#include <iostream>

typedef enum {
    TOKEN_SYMBOL,
    TOKEN_OPENPAREN,
    TOKEN_CLOSEPAREN,
} TokenType;

// TokenType type;
// string value;
struct Token {
    TokenType type;
    string value;
};
std::ostream& operator<<(std::ostream& os, const Token& token);

vector<Token> tokenize(string str);
vector<Token> normalize_tokens(vector<Token> tokens, Context ctx);
int find_correspoding_closeparen(vector<Token> tokens, int start, int end);
int find_correspoding_openparen(vector<Token> tokens, int start, int end);

optional<Expression> parse_expression_from_tokens(vector<Token> tokens, Context ctx, int start = 0, int end = -1);
optional<Expression> parse_expression(string str, Context ctx);
optional<Expression> parse_statement(string str, string statement_symbol, Context ctx);
