#include <iostream>
#include <assert.h>
#include "expression.h"
#include "utils.h"

using std::endl; 

Expression Expression::copy() const{
    Expression expr = {
        type, 
        symbol, 
        bracketed, 
        vector<Expression>(children.size()),
    };
    for (size_t i = 0; i < children.size(); i++) 
        expr.children[i] = children[i].copy();
    return expr;
}

Expression Expression::at(address addr) const{
    const Expression *expr = this;
    if (addr.size() == 0) return *expr;
    for (size_t i = 0; i < addr.size()-1; i++){
        expr = &expr->children[addr[i]];
    }
    return expr->children[addr.back()];
}

address Expression::parent_address_of(const address addr) {
    if (addr.size() == 0) return {};
    return {addr.begin(), addr.end()-1};
}
address Expression::child_address_of(const address addr, int child_index) {
    address child_addr(addr);
    child_addr.push_back(child_index);
    return child_addr;
}

vector<address> _get_all_address(Expression expr, address addr){
    vector<address> result = {addr};
    if (expr.children.size() <= 0) return result;

    for (size_t i = 0; i < expr.children.size(); i++){
        auto child_addr = Expression::child_address_of(addr, i);
        auto child_all_address = _get_all_address(expr.children[i], child_addr);
        result.insert(result.end(), child_all_address.begin(), child_all_address.end());
    }
    return result;
}

vector<address> Expression::get_all_address() const{
    return _get_all_address(*this, {});
}

string Expression::to_string() const {
    string str = "";
    // bool no_child = children.size() == 0;
    if (bracketed) str += "(";
    if (type == EXPRESSION_OPERATOR_UNARY){
        str += symbol;
        str += children[0].to_string();
    } else if (type == EXPRESSION_OPERATOR_BINARY){
        str += children[0].to_string();
        str += " " + symbol + " ";
        str += children[1].to_string();
    } else if (type == EXPRESSION_VALUE){
        str += symbol;
    }
    if (bracketed) str += ")";
    return str;
}

// check structural equality
// check if this expression can be matched by the pattern
// only based on operators
bool Expression::can_pattern_match(Expression pattern, Context ctx) const{
    bool is_constant = vector_contain(pattern.symbol, ctx.variables);
    if (ctx.handle_numerics) is_constant |= is_str_numeric(pattern.symbol);

    if (pattern.type == EXPRESSION_VALUE && !is_constant) return true;

    // is operator
    if (type != pattern.type) return false;
    if (symbol != pattern.symbol) return false;

    if (children.size() != pattern.children.size()) return false;
    for (size_t i = 0; i < children.size(); i++){
        if (!children[i].can_pattern_match(pattern.children[i], ctx)) return false;
    }
    return true;
}

vector<string> Expression::extract_variables() const{
    if (this->children.size() == 0 && this->type == EXPRESSION_VALUE) return vector<string>{this->symbol};
    vector<string> variables;
    for (size_t i = 0; i < this->children.size(); i++){
        auto child_variables = this->children[i].extract_variables();
        variables.insert(variables.end(), child_variables.begin(), child_variables.end());
    }
    return variables;
}

vector<address> _get_operator_chains_from(Expression root_expr, address addr, bool from_top){
    Expression expr      = root_expr.at(addr);
    string this_symbol   = expr.symbol;
    if (addr.size() > 0 && !from_top){
        address parent_addr = Expression::parent_address_of(addr);
        string parent_symbol = root_expr.at(parent_addr).symbol;
        if (this_symbol == parent_symbol) return _get_operator_chains_from(root_expr, parent_addr, false);
    }

    vector<address> operator_chains = {addr};
    for (size_t i = 0; i < expr.children.size(); i++){
        address child_addr = Expression::child_address_of(addr, i);

        string child_symbol = expr.children[i].symbol;
        if (child_symbol != this_symbol) continue;
        auto child_operator_chains = _get_operator_chains_from(root_expr, child_addr, true);
        operator_chains.insert(operator_chains.end(), child_operator_chains.begin(), child_operator_chains.end());
    }
    return operator_chains;
}

vector<address> Expression::get_operator_chains_from(address addr) const{
    return _get_operator_chains_from(*this, addr, false);
}

bool Expression::is_in_same_operator_chain(address addr1, address addr2) const{
    // check if both parent are in the same operator chain
    address addr1_parent = Expression::parent_address_of(addr1);
    address addr2_parent = Expression::parent_address_of(addr2);
    auto operator_chains = this->get_operator_chains_from(addr1_parent);
    return vector_contain(addr2_parent, operator_chains);
}

void Expression::strip_parentheses_for_associative_op(string op){
    switch (this->type){
        case EXPRESSION_VALUE:{ return; }
        case EXPRESSION_OPERATOR_UNARY:{
            for (auto &child : this->children) child.strip_parentheses_for_associative_op(op);
            return;
        }
        case EXPRESSION_OPERATOR_BINARY:{
            if (this->symbol == op) {
                for (auto &child : this->children){
                    if (child.symbol == op) child.bracketed = false;
                    child.strip_parentheses_for_associative_op(op);
                }
            }else{
                for (auto &child : this->children) child.strip_parentheses_for_associative_op(op);
                return;
            }
        }
    }
}


// return map that maps variables from pattern to expressions in this expression
// NOTE : this->can_pattern_match(pattern) must be true
optional<map<string, Expression>> Expression::try_match_pattern(Expression pattern) const{
    if (pattern.type == EXPRESSION_VALUE){
        return map<string, Expression>{{pattern.symbol, *this}};
    }

    map<string, Expression> variable_map = {};
    for (size_t i = 0; i < pattern.children.size(); i++){
        auto child_variable_map = children[i].try_match_pattern(pattern.children[i]);
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
        vector<Expression>(children.size()),
    };
    for (size_t i = 0; i < children.size(); i++) 
        expr.children[i] = children[i].apply_variable_map(variable_map);
    return expr;
}

// rule of equality
vector<Expression> Expression::apply_rule_equal(Expression rule, Context ctx) const{
    if (rule.symbol != "=") return {};

    // turn lhs to rhs
    auto lhs = rule.children[0];
    auto rhs = rule.children[1];

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
    for (size_t i = 0; i < this->children.size(); i++){
        auto child_result = this->children[i].apply_rule_equal(rule, ctx);
        for (auto &child_expr : child_result){
            auto expr = this->copy();
            expr.children[i] = child_expr;
            result.push_back(expr);
        }
    }
    return result;
}

bool operator==(const Expression& lhs, const Expression& rhs){
    if (lhs.type != rhs.type) return false;
    if (lhs.symbol != rhs.symbol) return false;
    if (lhs.children.size() != rhs.children.size()) return false;
    for (size_t i = 0; i < lhs.children.size(); i++){
        if (!(lhs.children[i] == rhs.children[i])) return false;
    }
    return true;
}
bool operator!=(const Expression& lhs, const Expression& rhs){
    return !(lhs == rhs);
}
    
std::ostream& operator<<(std::ostream& os, const Expression& exp){
    switch (exp.type){
        case EXPRESSION_OPERATOR_BINARY:
            os << "EXPRESSION_OPERATOR_BINARY: " << exp.symbol; break;
        case EXPRESSION_OPERATOR_UNARY:
            os << "EXPRESSION_OPERATOR_UNARY: " << exp.symbol; break;
        case EXPRESSION_VALUE:
            os << "EXPRESSION_VALUE: " << exp.symbol; break;
    }
    if (exp.bracketed) os << " (bracketed)";
    os << endl;
    for (size_t i = 0; i < exp.children.size(); i++){
        os << exp.children[i];
    }
    return os;
}


/// ================= helper functions ================================

Expression Expression::create_equality(Expression lhs, Expression rhs){
    lhs.bracketed = false;
    rhs.bracketed = false;
    return {
        EXPRESSION_OPERATOR_BINARY,
        "=",
        false,
        { lhs, rhs }
    };
}

Expression Expression::create_symbol(string symbol){
    return {
        EXPRESSION_VALUE,
        symbol,
        false,
        {}
    };
}
