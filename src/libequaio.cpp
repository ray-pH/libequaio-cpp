#include <optional>
#include <string>
#include <vector>
#include <iostream>
#include <assert.h>
#include <algorithm>
#include <map>
#include "libequaio.h"
#include "utils.h"

using std::optional; 
using std::string; 
using std::vector; 
using std::endl; 
using std::map;

std::ostream& operator<<(std::ostream& os, const Token& token){
    switch (token.type){
        case TOKEN_SYMBOL:
            os << "TOKEN_SYMBOL: " << token.value; break;
        case TOKEN_OPENPAREN:
            os << "TOKEN_OPENPAREN"; break;
        case TOKEN_CLOSEPAREN:
            os << "TOKEN_CLOSEPAREN"; break;
    }
    return os;
}



Expression Expression::copy() const{
    Expression expr = {
        type, 
        symbol, 
        bracketed, 
        vector<Expression>(child.size()),
    };
    for (size_t i = 0; i < child.size(); i++) 
        expr.child[i] = child[i].copy();
    return expr;
}

string Expression::to_string() const {
    string str = "";
    bool no_child = child.size() == 0;
    if (bracketed && !no_child) str += "(";
    if (type == EXPRESSION_OPERATOR_PREFIX){
        str += symbol;
        str += child[0].to_string();
    } else if (type == EXPRESSION_OPERATOR_INFIX){
        str += child[0].to_string();
        str += " " + symbol + " ";
        str += child[1].to_string();
    } else if (type == EXPRESSION_VALUE){
        str += symbol;
    }
    if (bracketed && !no_child) str += ")";
    return str;
}

// check structural equality
// check if this expression can be matched by the pattern
// only based on operators
bool Expression::can_pattern_match(Expression pattern, Context ctx) const{
    bool is_constant = vector_contain(pattern.symbol, ctx.variables);
    if (pattern.type == EXPRESSION_VALUE && !is_constant) return true;

    // is operator
    if (type != pattern.type) return false;
    if (symbol != pattern.symbol) return false;

    if (child.size() != pattern.child.size()) return false;
    for (size_t i = 0; i < child.size(); i++){
        if (!child[i].can_pattern_match(pattern.child[i], ctx)) return false;
    }
    return true;
}

vector<string> Expression::extract_variables(Expression expr){
    if (expr.child.size() == 0 && expr.type == EXPRESSION_VALUE) return vector<string>{expr.symbol};
    vector<string> variables;
    for (size_t i = 0; i < expr.child.size(); i++){
        auto child_variables = extract_variables(expr.child[i]);
        variables.insert(variables.end(), child_variables.begin(), child_variables.end());
    }
    return variables;
}

// return map that maps variables from pattern to expressions in this expression
// NOTE : this->can_pattern_match(pattern) must be true
optional<map<string, Expression>> Expression::try_match_pattern(Expression pattern) const{
    if (pattern.type == EXPRESSION_VALUE){
        return map<string, Expression>{{pattern.symbol, *this}};
    }

    map<string, Expression> variable_map = {};
    for (size_t i = 0; i < pattern.child.size(); i++){
        auto child_variable_map = child[i].try_match_pattern(pattern.child[i]);
        if (!child_variable_map.has_value()) return std::nullopt;
        // check if there is a conflict with the variable_map
        for (const auto & [key, value] : child_variable_map.value()){
            if (map_contain(key, variable_map)){
                if (variable_map[key] != value) return std::nullopt;
            }
            // add to variable_map
            variable_map[key] = value;
        }
    }
    return variable_map;
}

Expression Expression::apply_variable_map(map<string, Expression> variable_map) const{
    if (type == EXPRESSION_VALUE){
        if (map_contain(symbol, variable_map)){
            return variable_map[symbol];
        } else {
            return *this;
        }
    }
    Expression expr = {
        type, 
        symbol, 
        bracketed, 
        vector<Expression>(child.size()),
    };
    for (size_t i = 0; i < child.size(); i++) 
        expr.child[i] = child[i].apply_variable_map(variable_map);
    return expr;
}

// rule of equality
vector<Expression> Expression::apply_rule_equal(Expression rule, Context ctx) const{
    if (rule.symbol != "=") return {};

    // turn lhs to rhs
    auto lhs = rule.child[0];
    auto rhs = rule.child[1];

    // auto variables = extract_variables(lhs);
    vector<Expression> result = {};

    if (this->can_pattern_match(lhs, ctx)){
        auto variable_map = this->try_match_pattern(lhs);
        if (variable_map.has_value()){
            auto applied_rhs = rhs.apply_variable_map(variable_map.value());
            result.push_back(applied_rhs);
        }
    }

    // do it for each child
    for (size_t i = 0; i < this->child.size(); i++){
        auto child_result = this->child[i].apply_rule_equal(rule, ctx);
        for (auto &child_expr : child_result){
            auto expr = this->copy();
            expr.child[i] = child_expr;
            result.push_back(expr);
        }
    }
    return result;
}

bool operator==(const Expression& lhs, const Expression& rhs){
    if (lhs.type != rhs.type) return false;
    if (lhs.symbol != rhs.symbol) return false;
    if (lhs.child.size() != rhs.child.size()) return false;
    for (size_t i = 0; i < lhs.child.size(); i++){
        if (!(lhs.child[i] == rhs.child[i])) return false;
    }
    return true;
}
bool operator!=(const Expression& lhs, const Expression& rhs){
    return !(lhs == rhs);
}
    
std::ostream& operator<<(std::ostream& os, const Expression& exp){
    switch (exp.type){
        case EXPRESSION_OPERATOR_INFIX:
            os << "EXPRESSION_OPERATOR_INFIX: " << exp.symbol; break;
        case EXPRESSION_OPERATOR_PREFIX:
            os << "EXPRESSION_OPERATOR_PREFIX: " << exp.symbol; break;
        case EXPRESSION_VALUE:
            os << "EXPRESSION_VALUE: " << exp.symbol; break;
    }
    if (exp.bracketed) os << " (bracketed)";
    os << endl;
    for (size_t i = 0; i < exp.child.size(); i++){
        os << exp.child[i];
    }
    return os;
}

// struct Statement {
// }

vector<Token> tokenize(string str){
    vector<Token> tokens;
    string current_token = "";
    for (size_t i = 0; i < str.length(); i++){
        if (str[i] == '('){
            if (current_token != ""){
                tokens.push_back({TOKEN_SYMBOL, current_token});
                current_token = "";
            }
            tokens.push_back({TOKEN_OPENPAREN, ""});
        } else if (str[i] == ')'){
            if (current_token != ""){
                tokens.push_back({TOKEN_SYMBOL, current_token});
                current_token = "";
            }
            tokens.push_back({TOKEN_CLOSEPAREN, ""});
        } else if (str[i] == ' '){
            if (current_token != ""){
                tokens.push_back({TOKEN_SYMBOL, current_token});
                current_token = "";
            }
        } else {
            current_token += str[i];
        }
    }
    if (current_token != ""){
        tokens.push_back({TOKEN_SYMBOL, current_token});
    }
    return tokens;
}

int find_correspoding_closeparen(vector<Token> tokens, int start, int end){
    int depth = 0;
    for (int i = start; i < end; i++){
        if (tokens[i].type == TOKEN_OPENPAREN) depth++;
        else if (tokens[i].type == TOKEN_CLOSEPAREN) depth--;
        if (depth == 0) return i;
    }
    return -1;
}

optional<Expression> parse_expression_from_tokens(vector<Token> tokens, Context ctx, int start = 0, int end = -1){
    if (end == -1) end = tokens.size();
    int rightmost_parsed = start;
    optional<Expression> leftexpr;

    switch (tokens[start].type){
        case TOKEN_OPENPAREN:{
            int closeparen_id = find_correspoding_closeparen(tokens, start, end);
            if (closeparen_id == -1) return std::nullopt;

            rightmost_parsed = closeparen_id;
            auto inner_expr = parse_expression_from_tokens(tokens, ctx, start+1, closeparen_id);
            leftexpr = inner_expr;
            break;
        }
        case TOKEN_SYMBOL:{
            Expression expr = {
                EXPRESSION_VALUE, 
                tokens[start].value, 
                true, 
                {}
            };

            rightmost_parsed = start;
            leftexpr = std::make_optional(expr);
            break;
        }
        case TOKEN_CLOSEPAREN:
            assert(false && "Unreachable");
        default:
            // assert unreachable
            assert(false && "Invalid token type");
    }

    if (!leftexpr.has_value()) return std::nullopt;
    if (rightmost_parsed+1 >= end) return leftexpr;

    // check if next token is a binary operator
    if (vector_contain(tokens[rightmost_parsed+1].value, ctx.binary_operators)){
        if (rightmost_parsed+2 >= end) return std::nullopt;

        auto rightexpr = parse_expression_from_tokens(tokens, ctx, rightmost_parsed+2, end);
        if (!rightexpr.has_value()) return std::nullopt;

        Expression expr = {
            EXPRESSION_OPERATOR_INFIX, 
            tokens[rightmost_parsed+1].value, 
            true, 
            {leftexpr.value(), rightexpr.value()}
        };
        return std::make_optional(expr);
    } else {
        // assume this is a prefix operator
        auto rightexpr = parse_expression_from_tokens(tokens, ctx, rightmost_parsed+1, end);
        if (!rightexpr.has_value()) return std::nullopt;

        Expression expr = {
            EXPRESSION_OPERATOR_PREFIX, 
            tokens[rightmost_parsed+1].value, 
            true, 
            {rightexpr.value()}
        };
        return std::make_optional(expr);
    }

}

optional<Expression> parse_expression(string str, Context ctx){
    auto tokens = tokenize(str);
    return parse_expression_from_tokens(tokens, ctx);
}

// ex: statement symbol is "=" for equality
optional<Expression> parse_statement(string str, string statement_symbol, Context ctx){
    // find location of statement symbol
    int statement_symbol_id = -1;
    for (size_t i = 0; i < str.length(); i++){
        if (str[i] == statement_symbol[0]){
            bool found = true;
            for (size_t j = 0; j < statement_symbol.length(); j++){
                if (str[i+j] != statement_symbol[j]){
                    found = false;
                    break;
                }
            }
            if (found){
                statement_symbol_id = i;
                break;
            }
        }
    }
    if (statement_symbol_id == -1) return std::nullopt;

    auto leftexpr = parse_expression(str.substr(0, statement_symbol_id), ctx);
    if (!leftexpr.has_value()) return std::nullopt;

    auto rightexpr = parse_expression(str.substr(statement_symbol_id+statement_symbol.length(), str.length()), ctx);
    if (!rightexpr.has_value()) return std::nullopt;

    return Expression{
        EXPRESSION_OPERATOR_INFIX, 
        statement_symbol, 
        false, 
        {leftexpr.value(), rightexpr.value()}
    };
}


/// ================= helper functions ================================
Expression Expression::create_equality(Expression lhs, Expression rhs){
    return {
        EXPRESSION_OPERATOR_INFIX,
        "=",
        false,
        { lhs, rhs }
    };
}
