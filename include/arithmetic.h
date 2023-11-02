#pragma once
#include "libequaio.h"
#include <string>

using std::string;

namespace Arithmetic {
    
typedef enum{
    OPERATOR_ADD = '+',
    OPERATOR_SUB = '-',
    OPERATOR_MUL = '*',
    OPERATOR_DIV = '/',
} Operator;

const map<Operator, string> operation_name = {
    {OPERATOR_ADD, "add"},
    {OPERATOR_SUB, "subtract"},
    {OPERATOR_MUL, "multiply"},
    {OPERATOR_DIV, "divide"},
};

optional<Expression> create_calculation(string left, string right, Operator op);

}
