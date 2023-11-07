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

// turn `a - b` into `a + (_ b)`
Expression Arithmetic::turn_subtraction_to_addition(Expression expr){
    bool is_subtraction = expr.type == EXPRESSION_OPERATOR_BINARY 
                        && expr.symbol == operator_symbol.at(OPERATOR_SUB);
    if (is_subtraction){
        Expression newright = {
            EXPRESSION_OPERATOR_UNARY,
            operator_symbol.at(OPERATOR_MINUS),
            true,
            {turn_subtraction_to_addition(expr.children[1])}
        };
        return {
            EXPRESSION_OPERATOR_BINARY,
            operator_symbol.at(OPERATOR_ADD),
            expr.bracketed,
            {turn_subtraction_to_addition(expr.children[0]), newright}
        };
    }

    // apply recursively to all children
    for (size_t i = 0; i < expr.children.size(); i++){
        expr.children[i] = turn_subtraction_to_addition(expr.children[i]);
    }
    return expr;
}

// turn `a + (_ b)` into `a - b`
Expression Arithmetic::turn_addition_to_subtraction(Expression expr){
    bool is_addition = expr.type == EXPRESSION_OPERATOR_BINARY 
                        && expr.symbol == operator_symbol.at(OPERATOR_ADD);
    if (is_addition){
        bool is_right_operand_a_minus = expr.children[1].type == EXPRESSION_OPERATOR_UNARY
                                        && expr.children[1].symbol == operator_symbol.at(OPERATOR_MINUS);
        if (is_right_operand_a_minus){
            return {
                EXPRESSION_OPERATOR_BINARY,
                operator_symbol.at(OPERATOR_SUB),
                expr.bracketed,
                {
                    turn_addition_to_subtraction(expr.children[0]), 
                    turn_addition_to_subtraction(expr.children[1].children[0])
                },
            };
        }
    }

    // apply recursively to all children
    for (size_t i = 0; i < expr.children.size(); i++){
        expr.children[i] = turn_addition_to_subtraction(expr.children[i]);
    }
    return expr;
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
