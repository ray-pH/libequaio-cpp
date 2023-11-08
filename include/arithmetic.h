#pragma once
#include "expression.h"
#include <string>

using std::string;

namespace Arithmetic {
    
typedef enum{
    OPERATOR_ADD,
    OPERATOR_SUB,
    OPERATOR_MUL,
    OPERATOR_DIV,
    OPERATOR_MINUS,
    OPERATOR_RECIP,
} Operator;

const map<Operator, string> operator_symbol = {
    {OPERATOR_ADD, "+"},
    {OPERATOR_SUB, "-"},
    {OPERATOR_MUL, "*"},
    {OPERATOR_DIV, "/"},

    {OPERATOR_MINUS, "_"}, // uniary minus e.g. -2, -12 written as _2, _12
    {OPERATOR_RECIP, "1/"}, // uniary reciprocal 
};
const map<Operator, string> operator_name = {
    {OPERATOR_ADD,   "add"},
    {OPERATOR_SUB,   "subtract"},
    {OPERATOR_MUL,   "multiply"},
    {OPERATOR_DIV,   "divide"},
    {OPERATOR_MINUS, "minus"},
    {OPERATOR_RECIP, "reciprocal"},
};

const Context arithmetic_context = {
    {},
    {
        operator_symbol.at(OPERATOR_ADD),
        operator_symbol.at(OPERATOR_SUB),
        operator_symbol.at(OPERATOR_MUL),
        operator_symbol.at(OPERATOR_DIV),
    },
    { 
        operator_symbol.at(OPERATOR_MINUS), 
        operator_symbol.at(OPERATOR_RECIP),
    },
    true,
};

optional<Expression> create_calculation(string left, string right, Operator op);
Expression create_rule_commute(int pos_i, int pos_j, int count, Operator op);

Expression turn_subtraction_to_addition(Expression expr);
Expression turn_addition_to_subtraction(Expression expr);
Expression turn_division_to_multiplication(Expression expr);
Expression turn_multiplication_to_division(Expression expr);
Expression remove_assoc_parentheses(Expression expr);
inline Expression create_rule_commute_addition(int pos_i, int pos_j, int count){
    return create_rule_commute(pos_i, pos_j, count, OPERATOR_ADD);
};

}
