#include <iostream>
#include <assert.h>
#include <iomanip>
#include <type_traits>
#include "libequaio.h"
#include "interface.h"
#include "utils.h"
#include "parser.h"

using std::cout;
using std::endl;

// vector<string> variables;
// vector<string> binary_operators; //also include ','
// vector<string> prefix_operators; //also include functions
//
// Context context;
// map<string,Expression> rules;
// Expression target;
// optional<Expression> current;
const string INDENT = "   ";
void Task::print_state() const{
    cout << "history:" << endl;
    for (const auto& [expr, name] : this->history){
        cout << INDENT << expr.to_string();
        if (name != "") cout << "    ... (" << name << ")";
        cout << endl;
    }

    cout << "rules :"<< endl;
    for (const auto& [key, value] : rules){
        // cout << INDENT << std::setw(15) << key << " : " << value.to_string() << endl;
        cout << INDENT << key << " : " << value.to_string() << endl;
    }
    cout << "target :" << endl;
    if (this->target.has_value()){
        cout << INDENT << this->target.value().to_string() << endl;
    } else {
        cout << INDENT << "None" << endl;
    }

    cout << "current:" << endl;
    if (this->current.has_value()){
        cout << INDENT;
        if (this->print_rhs_only){
            cout << "= " << this->current.value().child[0].to_string();
        }else{
            cout << this->current.value().to_string();
        }
        cout << endl;
    } else {
        cout << INDENT << "None" << endl;
    }

    if (this->error_messages.size() > 0){
        cout << "error messages:" << endl;
        for (const auto& msg : this->error_messages){
            cout << INDENT << msg << endl;
        }
    }
} 

bool Task::set_current_eq(string exprstr){
    auto expr = parse_statement(exprstr, "=", this->context);
    if (!expr.has_value()) return false;
    this->current = expr.value();
    this->history.push_back({expr.value(), ""});
    return true;
}
bool Task::set_target_eq(string exprstr){
    auto target = parse_statement(exprstr, "=", this->context);
    if (!target.has_value()) return false;
    this->target = target.value();
    return true;
}
void Task::add_rule_expr(string name, Expression expr){
    this->rules.insert({name, expr});
}
void Task::add_rule_eq(string name, string exprstr) {
    auto expr = parse_statement(exprstr, "=", this->context);
    if (!expr.has_value()) {
        this->error_messages.push_back("failed to parse rule: " + exprstr);
        return;
    }
    this->add_rule_expr(name, expr.value());
}

void Task::init_current_with_target_lhs(){
    if (!this->target.has_value()) {
        this->error_messages.push_back("target statement is not set");
        return;
    }
    Expression lhs  = this->target.value().child[0];
    this->current = Expression::create_equality(lhs,lhs);
}

void replace_expression_symbol(Expression &expr, string from_symbol, Expression to){
    // iterate over the child of expr
    for (auto &child : expr.child){
        if (child.symbol == from_symbol){
            child = to.copy();
        } else {
            replace_expression_symbol(child, from_symbol, to);
        }
    }
}

bool Task::apply_function_to_both_side_expr(Expression fexpr, string varname, string custom_name){
    if (!this->current.has_value()){
        this->error_messages.push_back("current statement is not set");
        return false;
    }
    if (this->current->symbol != "="){
        this->error_messages.push_back("current statement is not an equality");
        return false;
    }
    if (custom_name == "") custom_name = "apply " + fexpr.to_string() + " to both side";

    Expression lhs = this->current.value().child[0];
    Expression rhs = this->current.value().child[1];

    Expression new_lhs = fexpr.copy();
    Expression new_rhs = fexpr.copy();
    replace_expression_symbol(new_lhs, varname, lhs);
    replace_expression_symbol(new_rhs, varname, rhs);

    Expression newexpr = Expression::create_equality(new_lhs, new_rhs);
    this->current = newexpr;
    this->history.push_back({newexpr, custom_name});
    return true;
}

bool Task::apply_function_to_both_side(string fstr, string varname, string custom_name){
    auto fexpr = parse_expression(fstr, this->context);
    if (!fexpr.has_value()) {
        this->error_messages.push_back("failed to parse function: " + fstr);
        return false;
    }

    if (custom_name == "") custom_name = "apply " + fstr + " to both side";
    return this->apply_function_to_both_side_expr(fexpr.value(), varname, custom_name);
}


bool Task::apply_rule_expr(Expression expr, string name){
    if (!this->current.has_value()){
        this->error_messages.push_back("current statement is not set");
        return false;
    }
    if (this->current->symbol != "="){
        this->error_messages.push_back("current statement is not an equality");
        return false;
    }

    auto results = this->current.value().apply_rule_equal(expr, this->context);
    if (results.size() == 0){
        this->error_messages.push_back("failed to apply rule: " + name);
        return false;
    }

    // FIXME: for now only take the first result
    Expression newcurrent = results[0];
    this->current = newcurrent;
    this->history.push_back({newcurrent, name});
    return true;
}

bool Task::apply_rule(string rulename, string custom_name){
    if (!map_contain(rulename, this->rules)){
        this->error_messages.push_back("rule " + rulename + " is not defined");
        return false;
    }
    if (custom_name == "") custom_name = "apply rule: " + rulename;

    Expression ruleexpr = this->rules[rulename];
    return this->apply_rule_expr(ruleexpr, custom_name);
}

// =============== Arithmetics
bool Task::apply_arithmetic_to_both_side(Arithmetic::Operator op, string value, string custom_name){
    string varname = "X";
    Expression expr = {
        EXPRESSION_OPERATOR_INFIX,
        Arithmetic::operator_symbol.at(op),
        false,
        {
            Expression::create_symbol(varname),
            Expression::create_symbol(value),
        }
    };
    if (custom_name == "") custom_name = Arithmetic::operator_name.at(op) + " both side by " + value;
    return this->apply_function_to_both_side_expr(expr, varname, custom_name);
}

bool Task::apply_arithmetic_calculation(string left, string right, Arithmetic::Operator op, string custom_name){
    auto expr = Arithmetic::create_calculation(left, right, op);
    if (!expr.has_value()){
        this->error_messages.push_back("failed to create calculation: " + left + " " + 
                                       Arithmetic::operator_name.at(op) + " " + right);
        return false;
    }
    if (custom_name == "") custom_name = "calculate " + expr->to_string();
    return this->apply_rule_expr(expr.value(), custom_name);
}
