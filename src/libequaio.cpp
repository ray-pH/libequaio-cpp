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

/// ================= helper functions ================================

Expression Expression::create_equality(Expression lhs, Expression rhs){
    lhs.bracketed = false;
    rhs.bracketed = false;
    return {
        EXPRESSION_OPERATOR_INFIX,
        "=",
        false,
        { lhs, rhs }
    };
}
