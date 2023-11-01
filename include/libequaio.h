#pragma once

#include <optional>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

using std::optional; 
using std::string; 
using std::vector; 
using std::map;

typedef enum {
    TOKEN_SYMBOL,
    TOKEN_OPENPAREN,
    TOKEN_CLOSEPAREN,
} TokenType;
struct Token {
    TokenType   type;
    string value;
};


typedef enum{
    EXPRESSION_OPERATOR_INFIX, // only for binary operators
    EXPRESSION_OPERATOR_PREFIX,
    EXPRESSION_VALUE,
} ExpType;

struct Context{
    vector<string> variables;
    vector<string> binary_operators; //also include ','
    vector<string> prefix_operators; //also include functions
};

struct Expression {
    ExpType type;
    string  symbol;
    bool    bracketed;
    vector<Expression> child;

    Expression copy() const;
    string to_string() const;

    // check structural equality
    // check if this expression can be matched by the pattern
    // only based on operators
    bool can_pattern_match(Expression pattern, Context ctx) const;

    static vector<string> extract_variables(Expression expr);

    // return map that maps variables from pattern to expressions in this expression
    // NOTE : this->can_pattern_match(pattern) must be true
    optional<map<string, Expression>> try_match_pattern(Expression pattern) const;
    Expression apply_variable_map(map<string, Expression> variable_map) const;

    // rule of equality
    vector<Expression> apply_rule_equal(Expression rule, Context ctx) const;
    friend bool operator==(const Expression& lhs, const Expression& rhs);
    friend bool operator!=(const Expression& lhs, const Expression& rhs);
    
};


std::ostream& operator<<(std::ostream& os, const Token& token);
std::ostream& operator<<(std::ostream& os, const Expression& exp);
vector<Token> tokenize(string str);
int find_correspoding_closeparen(vector<Token> tokens, int start, int end);
optional<Expression> parse_expression_from_tokens(vector<Token> tokens, Context ctx, int start, int end);
optional<Expression> parse_expression(string str, Context ctx);
optional<Expression> parse_statement(string str, string statement_symbol, Context ctx);
