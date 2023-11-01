#include <assert.h>
#include "utils.h"
#include "parser.h"

// optional<Expression> parse_expression_from_tokens(vector<Token> tokens, 
//       Context ctx, int start = 0, int end = -1, optional<Expression> prevexpr = std::nullopt)
// {
//     if (end == -1) end = tokens.size();
//     int rightmost_parsed = start;
//     optional<Expression> leftexpr;
//
//     Token token = tokens[start];
//
//     // check if current token is not an operator (either a single value or expr inside a bracket)
//     // PREFIX OPERATOR
//     if (vector_contain(token.value, ctx.prefix_operators)){
//         if (rightmost_parsed+1 >= end) return std::nullopt;
//         //todo
//     }
//     // BINARY INFIX OPERATOR
//     else if (vector_contain(token.value, ctx.binary_operators)){
//         if (!prevexpr.has_value()) return std::nullopt;
//         if (rightmost_parsed+1 >= end) return std::nullopt;
//         //todo
//     }
//
//     // VALUE
//             
// }
optional<Expression> parse_expression_from_tokens_old(vector<Token> tokens, Context ctx, int start = 0, int end = -1){
    if (end == -1) end = tokens.size();
    int rightmost_parsed = start;
    optional<Expression> leftexpr;

    switch (tokens[start].type){
        case TOKEN_OPENPAREN:{
            int closeparen_id = find_correspoding_closeparen(tokens, start, end);
            if (closeparen_id == -1) return std::nullopt;

            rightmost_parsed = closeparen_id;
            auto inner_expr = parse_expression_from_tokens_old(tokens, ctx, start+1, closeparen_id);
            leftexpr = inner_expr;
            break;
        }
        case TOKEN_SYMBOL:{
            Expression expr = {
                EXPRESSION_VALUE, 
                tokens[start].value, 
                true, 
                {}
            };

            rightmost_parsed = start;
            leftexpr = std::make_optional(expr);
            break;
        }
        case TOKEN_CLOSEPAREN:
            assert(false && "Unreachable");
        default:
            // assert unreachable
            assert(false && "Invalid token type");
    }

    if (!leftexpr.has_value()) return std::nullopt;
    if (rightmost_parsed+1 >= end) return leftexpr;

    // check if next token is a binary operator
    if (vector_contain(tokens[rightmost_parsed+1].value, ctx.binary_operators)){
        if (rightmost_parsed+2 >= end) return std::nullopt;

        auto rightexpr = parse_expression_from_tokens_old(tokens, ctx, rightmost_parsed+2, end);
        if (!rightexpr.has_value()) return std::nullopt;

        Expression expr = {
            EXPRESSION_OPERATOR_INFIX, 
            tokens[rightmost_parsed+1].value, 
            true, 
            {leftexpr.value(), rightexpr.value()}
        };
        return std::make_optional(expr);
    } else {
        // assume this is a prefix operator
        auto rightexpr = parse_expression_from_tokens_old(tokens, ctx, rightmost_parsed+1, end);
        if (!rightexpr.has_value()) return std::nullopt;

        Expression expr = {
            EXPRESSION_OPERATOR_PREFIX, 
            tokens[rightmost_parsed+1].value, 
            true, 
            {rightexpr.value()}
        };
        return std::make_optional(expr);
    }

}

optional<Expression> parse_expression(string str, Context ctx){
    auto tokens = tokenize(str);
    return parse_expression_from_tokens_old(tokens, ctx);
}

// ex: statement symbol is "=" for equality
optional<Expression> parse_statement(string str, string statement_symbol, Context ctx){
    // find location of statement symbol
    int statement_symbol_id = -1;
    for (size_t i = 0; i < str.length(); i++){
        if (str[i] == statement_symbol[0]){
            bool found = true;
            for (size_t j = 0; j < statement_symbol.length(); j++){
                if (str[i+j] != statement_symbol[j]){
                    found = false;
                    break;
                }
            }
            if (found){
                statement_symbol_id = i;
                break;
            }
        }
    }
    if (statement_symbol_id == -1) return std::nullopt;

    auto leftexpr = parse_expression(str.substr(0, statement_symbol_id), ctx);
    if (!leftexpr.has_value()) return std::nullopt;

    auto rightexpr = parse_expression(str.substr(statement_symbol_id+statement_symbol.length(), str.length()), ctx);
    if (!rightexpr.has_value()) return std::nullopt;

    return Expression{
        EXPRESSION_OPERATOR_INFIX, 
        statement_symbol, 
        false, 
        {leftexpr.value(), rightexpr.value()}
    };
}
