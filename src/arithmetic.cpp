#include "arithmetic.h"
#include "utils.h"

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

optional<Expression> Arithmetic::create_calculation(string left, string right, Operator op){
    if (is_str_integer(left) && is_str_integer(right)){
        return _create_calculation(stoi(left), stoi(right), op);
    }
    if (is_str_numeric(left) && is_str_numeric(right)){
        return _create_calculation(stof(left), stof(right), op);
    }
    return std::nullopt;
}
