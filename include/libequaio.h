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

// TokenType type;
// string value;
struct Token {
    TokenType type;
    string value;
};

typedef vector<int> address;

typedef enum{
    EXPRESSION_OPERATOR_INFIX, // only for binary operators
    EXPRESSION_OPERATOR_PREFIX,
    EXPRESSION_VALUE,
} ExpType;

// vector<string> variables;
// vector<string> infix\_operators; 
// vector<string> prefix\_operators; 
// bool handle\_numerics;             
struct Context{
    vector<string> variables;
    vector<string> infix_operators;  //also include ','
    vector<string> prefix_operators; //also include functions
    bool handle_numerics;            // true if numbers are allowed
};

// ExpType type;
// string  symbol;
// bool    bracketed;
// vector<Expression> child;
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
    Expression at(address addr) const;

    vector<string> extract_variables() const;

    // return map that maps variables from pattern to expressions in this expression
    // NOTE : this->can_pattern_match(pattern) must be true
    optional<map<string, Expression>> try_match_pattern(Expression pattern) const;
    Expression apply_variable_map(map<string, Expression> variable_map) const;

    // rule of equality
    vector<Expression> apply_rule_equal(Expression rule, Context ctx) const;
    friend bool operator==(const Expression& lhs, const Expression& rhs);
    friend bool operator!=(const Expression& lhs, const Expression& rhs);
    
    static Expression create_equality(Expression lhs, Expression rhs);
    static Expression create_symbol(string symbol);
};

std::ostream& operator<<(std::ostream& os, const Token& token);
std::ostream& operator<<(std::ostream& os, const Expression& exp);
