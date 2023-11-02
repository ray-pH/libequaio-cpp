#include "arithmetic.h"

using namespace Arithmetic;

template<typename T>
T _calculate(T left, T right, Operator op){
    switch (op){
        case OPERATOR_ADD:
            return left + right;
        case OPERATOR_SUB:
            return left - right;
        case OPERATOR_MUL:
            return left * right;
        case OPERATOR_DIV:
            return left / right;
        default:
            return 0;
    }
}

int calculate(int left, int right, Operator op){
    return _calculate<int>(left, right, op);
}
float calculate(float left, float right, Operator op){
    return _calculate<float>(left, right, op);
}


template<typename T>
Expression _create_calculation(T left, T right, Operator op){
    T result = calculate(left, right, op);
    Expression exprleft = Expression::create_symbol(std::to_string(left));
    Expression exprright = Expression::create_symbol(std::to_string(right));
    Expression rhs = Expression::create_symbol(std::to_string(result));
    Expression lhs = {
        EXPRESSION_OPERATOR_INFIX,
        string(1,op),
        false,
        {exprleft, exprright}
    };
    return Expression::create_equality(lhs, rhs);
}

Expression Arithmetic::create_calculation(int left, int right, Operator op){
    return _create_calculation(left, right, op);
}
Expression Arithmetic::create_calculation(float left, float right, Operator op){
    return _create_calculation(left, right, op);
}