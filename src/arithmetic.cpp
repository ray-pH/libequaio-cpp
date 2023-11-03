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
        operator_symbol.at(op),
        false,
        {exprleft, exprright}
    };
    return Expression::create_equality(lhs, rhs);
}

optional<Expression> Arithmetic::create_calculation(string left, string right, Operator op){
    // minus is a unary operator, which is different from subtraction
    if (op == OPERATOR_MINUS) return std::nullopt;

    if (is_str_integer(left) && is_str_integer(right)){
        return _create_calculation(stoi(left), stoi(right), op);
    }
    if (is_str_numeric(left) && is_str_numeric(right)){
        return _create_calculation(stof(left), stof(right), op);
    }
    return std::nullopt;
}

// turn `a - b` into `a + (-b)`
Expression Arithmetic::turn_subtraction_to_addition(Expression expr){
    bool is_subtraction = expr.type == EXPRESSION_OPERATOR_INFIX 
                        && expr.symbol == operator_symbol.at(OPERATOR_SUB);
    if (is_subtraction){
        Expression newright = {
            EXPRESSION_OPERATOR_PREFIX,
            operator_symbol.at(OPERATOR_MINUS),
            true,
            {turn_subtraction_to_addition(expr.child[1])}
        };
        return {
            EXPRESSION_OPERATOR_INFIX,
            operator_symbol.at(OPERATOR_ADD),
            expr.bracketed,
            {turn_subtraction_to_addition(expr.child[0]), newright}
        };
    }

    // apply recursively to all children
    for (size_t i = 0; i < expr.child.size(); i++){
        expr.child[i] = turn_subtraction_to_addition(expr.child[i]);
    }
    return expr;
}
