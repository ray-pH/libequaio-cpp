#pragma once
#include "libequaio.h"
#include <string>

using std::string;

namespace Arithmetic {
    
typedef enum{
    OPERATOR_ADD,
    OPERATOR_SUB,
    OPERATOR_MUL,
    OPERATOR_DIV,
    OPERATOR_MINUS,
} Operator;

const map<Operator, string> operator_symbol = {
    {OPERATOR_ADD, "+"},
    {OPERATOR_SUB, "-"},
    {OPERATOR_MUL, "*"},
    {OPERATOR_DIV, "/"},
    {OPERATOR_MINUS, "_"}, // uniary minus e.g. -2, -12 written as _2, _12
};
const map<Operator, string> operator_name = {
    {OPERATOR_ADD,   "add"},
    {OPERATOR_SUB,   "subtract"},
    {OPERATOR_MUL,   "multiply"},
    {OPERATOR_DIV,   "divide"},
    {OPERATOR_MINUS, "minux"},
};

const Context arithmetic_context = {
    {},
    {
        operator_symbol.at(OPERATOR_ADD),
        operator_symbol.at(OPERATOR_SUB),
        operator_symbol.at(OPERATOR_MUL),
        operator_symbol.at(OPERATOR_DIV),
    },
    { operator_symbol.at(OPERATOR_MINUS), },
    true,
};

optional<Expression> create_calculation(string left, string right, Operator op);
Expression create_rule_commute(int pos_i, int pos_j, int count, Operator op);

Expression turn_subtraction_to_addition(Expression expr);
Expression turn_addition_to_subtraction(Expression expr);
inline Expression create_rule_commute_addition(int pos_i, int pos_j, int count){
    return create_rule_commute(pos_i, pos_j, count, OPERATOR_ADD);
};

}
