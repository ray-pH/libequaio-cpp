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

Expression create_calculation(int left, int right, Operator op);
Expression create_calculation(float left, float right, Operator op);

}
