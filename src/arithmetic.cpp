#include "arithmetic.h"
#include "utils.h"
#include "parser.h"

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
        EXPRESSION_OPERATOR_BINARY,
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

// turn `a * b` into `a # (@b)`
// * is the `from_binary`
// # is the `to_binary`
// @ is the `to_unary`
Expression __turn_binary_to_binaryunary(Expression expr, string from_binary, string to_binary, string to_unary){
    bool is_from_binary = expr.type == EXPRESSION_OPERATOR_BINARY 
                        && expr.symbol == from_binary;
    if (is_from_binary){
        Expression newright = {
            EXPRESSION_OPERATOR_UNARY,
            to_unary,
            true,
            { __turn_binary_to_binaryunary(expr.children[1], from_binary, to_binary, to_unary) }
        };
        return {
            EXPRESSION_OPERATOR_BINARY,
            to_binary,
            expr.bracketed,
            {__turn_binary_to_binaryunary(expr.children[0], from_binary, to_binary, to_unary), newright}
        };
    }

    // apply recursively to all children
    for (size_t i = 0; i < expr.children.size(); i++){
        expr.children[i] = __turn_binary_to_binaryunary(expr.children[i], from_binary, to_binary, to_unary);
    }
    return expr;
}

// turn `a # (@b)` into `a * b`
// # is the `from_binary`
// @ is the `from_unary`
// * is the `to_binary`
Expression __turn_binaryunary_to_binary(Expression expr, string from_binary, string from_unary, string to_binary){
    bool is_from_binaryunary = expr.type == EXPRESSION_OPERATOR_BINARY 
                        && expr.symbol == from_binary;
    if (is_from_binaryunary){
        bool is_from_unary = expr.children[1].type == EXPRESSION_OPERATOR_UNARY
                            && expr.children[1].symbol == from_unary;
        if (is_from_unary){
            return {
                EXPRESSION_OPERATOR_BINARY,
                to_binary,
                expr.bracketed,
                {
                    __turn_binaryunary_to_binary(expr.children[0], from_binary, from_unary, to_binary),
                    __turn_binaryunary_to_binary(expr.children[1].children[0], from_binary, from_unary, to_binary)
                },
            };
        }
    }

    // apply recursively to all children
    for (size_t i = 0; i < expr.children.size(); i++){
        expr.children[i] = __turn_binaryunary_to_binary(expr.children[i], from_binary, from_unary, to_binary);
    }
    return expr;
}

// turn `a - b` into `a + (_ b)`
Expression Arithmetic::turn_subtraction_to_addition(Expression expr){
    return __turn_binary_to_binaryunary( expr, 
        operator_symbol.at(OPERATOR_SUB),
        operator_symbol.at(OPERATOR_ADD),
        operator_symbol.at(OPERATOR_MINUS)
    );
}

// turn `a + (_ b)` into `a - b`
Expression Arithmetic::turn_addition_to_subtraction(Expression expr){
    return __turn_binaryunary_to_binary( expr, 
        operator_symbol.at(OPERATOR_ADD),
        operator_symbol.at(OPERATOR_MINUS),
        operator_symbol.at(OPERATOR_SUB)
    );
}

// turn `a / b` into `a * (1/ b)`
Expression Arithmetic::turn_division_to_multiplication(Expression expr){
    return __turn_binary_to_binaryunary( expr, 
        operator_symbol.at(OPERATOR_DIV),
        operator_symbol.at(OPERATOR_MUL),
        operator_symbol.at(OPERATOR_RECIP)
    );
}

// turn `a * (1/ b)` into `a / b`
Expression Arithmetic::turn_multiplication_to_division(Expression expr){
    return __turn_binaryunary_to_binary( expr, 
        operator_symbol.at(OPERATOR_MUL),
        operator_symbol.at(OPERATOR_RECIP),
        operator_symbol.at(OPERATOR_DIV)
    );
}

Expression Arithmetic::remove_assoc_parentheses(Expression expr){
    auto newexpr = expr.copy();
    newexpr.strip_parentheses_for_associative_op(operator_symbol.at(OPERATOR_ADD));
    newexpr.strip_parentheses_for_associative_op(operator_symbol.at(OPERATOR_MUL));
    return newexpr;
}

Expression Arithmetic::create_rule_commute(int pos_i, int pos_j, int count, Operator op){
    vector<Token> tokens_from(2*count - 1);
    vector<Token> tokens_to(2*count - 1);

    // tokens_prev: _X0 + _X1 + _X2 + ... + _Xcount
    for (int i = 0; i < count; i++){
        tokens_from[2*i] = {TOKEN_SYMBOL, "_X" + std::to_string(i)};
        if(i < count-1) tokens_from[2*i + 1] = {TOKEN_SYMBOL, operator_symbol.at(op)};
    }

    // topent: _X0 + _X1 + _X2 + ... + _Xcount (with _Xi and _Xj swapped)
    for (int i = 0; i < count; i++){

        int id = i;
        if (i == pos_i) id = pos_j;
        else if (i == pos_j) id = pos_i;

        tokens_to[2*i] = {TOKEN_SYMBOL, "_X" + std::to_string(id)};
        if(i < count-1) tokens_to[2*i + 1] = {TOKEN_SYMBOL, operator_symbol.at(op)};
    }

    tokens_from = normalize_tokens(tokens_from, arithmetic_context);
    tokens_to = normalize_tokens(tokens_to, arithmetic_context);

    auto expr_from = parse_expression_from_tokens(tokens_from, arithmetic_context);
    if (!expr_from.has_value()) return {};

    auto expr_to = parse_expression_from_tokens(tokens_to, arithmetic_context);
    if (!expr_to.has_value()) return {};

    return Expression::create_equality(expr_from.value(), expr_to.value());
}
