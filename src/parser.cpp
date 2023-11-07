#include <assert.h>
#include "utils.h"
#include "parser.h"

std::ostream& operator<<(std::ostream& os, const Token& token){
    switch (token.type){
        case TOKEN_SYMBOL:
            os << "TOKEN_SYMBOL: " << token.value; break;
        case TOKEN_OPENPAREN:
            os << "TOKEN_OPENPAREN"; break;
        case TOKEN_CLOSEPAREN:
            os << "TOKEN_CLOSEPAREN"; break;
    }
    return os;
}

vector<Token> tokenize(string str){
    vector<Token> tokens;
    string current_token = "";
    for (size_t i = 0; i < str.length(); i++){
        if (str[i] == '('){
            if (current_token != ""){
                tokens.push_back({TOKEN_SYMBOL, current_token});
                current_token = "";
            }
            tokens.push_back({TOKEN_OPENPAREN, ""});
        } else if (str[i] == ')'){
            if (current_token != ""){
                tokens.push_back({TOKEN_SYMBOL, current_token});
                current_token = "";
            }
            tokens.push_back({TOKEN_CLOSEPAREN, ""});
        } else if (str[i] == ' '){
            if (current_token != ""){
                tokens.push_back({TOKEN_SYMBOL, current_token});
                current_token = "";
            }
        } else {
            current_token += str[i];
        }
    }
    if (current_token != ""){
        tokens.push_back({TOKEN_SYMBOL, current_token});
    }
    return tokens;
}

vector<Token> _normalize_tokens_add_unary_brackets(vector<Token> tokens, Context ctx){
    size_t pointer = 0;
    while (pointer < tokens.size()-1){
        Token token = tokens[pointer]; 

        // skip non unary operators
        if (!vector_contain(token.value, ctx.unary_operators)){
            pointer++;
            continue;
        }

        // add brackets for unary operators
        tokens.insert(tokens.begin()+pointer, Token{TOKEN_OPENPAREN, "("});
        pointer++;

        Token next_token = tokens[pointer+1];
        if (next_token.type == TOKEN_OPENPAREN){
            int closeparen_index = find_correspoding_closeparen(tokens, pointer+1, tokens.size());
            if (closeparen_index == -1) return {};

            tokens.insert(tokens.begin()+closeparen_index, Token{TOKEN_CLOSEPAREN, ")"});
        } else {
            // just a single symbol
            tokens.insert(tokens.begin()+pointer+2, Token{TOKEN_CLOSEPAREN, ")"});
            // pointer++;
        }

        pointer++;
    }
    return tokens;
}

vector<Token> _normalize_tokes_add_binary_brackets(vector<Token> tokens, Context ctx){
    size_t pointer = 1;
    while (pointer < tokens.size()-1){
        Token token = tokens[pointer];

        // skip non binary operators
        if (!vector_contain(token.value, ctx.binary_operators)){
            pointer++;
            continue;
        }

        Token prev_token = tokens[pointer-1]; 
        if (prev_token.type == TOKEN_CLOSEPAREN){
            int openparen_index = find_correspoding_openparen(tokens, pointer-1, -1);
            if (openparen_index == -1) return {};

            tokens.insert(tokens.begin()+openparen_index, Token{TOKEN_OPENPAREN, "("});
            pointer++;
        }else{
            // just a single symbol
            tokens.insert(tokens.begin()+pointer-1, Token{TOKEN_OPENPAREN, "("});
            pointer++;
        }

        Token next_token = tokens[pointer+1];
        if (next_token.type == TOKEN_OPENPAREN){
            int closeparen_index = find_correspoding_closeparen(tokens, pointer+1, tokens.size());
            if (closeparen_index == -1) return {};

            tokens.insert(tokens.begin()+closeparen_index, Token{TOKEN_CLOSEPAREN, ")"});
        } else {
            // just a single symbol
            tokens.insert(tokens.begin()+pointer+2, Token{TOKEN_CLOSEPAREN, ")"});
            // pointer++;
        }

        pointer++;


    }
    return tokens;
}

vector<Token> normalize_tokens(vector<Token> tokens, Context ctx){
    auto normalized_unary = _normalize_tokens_add_unary_brackets(tokens, ctx);
    auto normalized_binary  = _normalize_tokes_add_binary_brackets(normalized_unary, ctx);
    return normalized_binary;
}


// start is the TOKEN_OPENPAREN location
int find_correspoding_closeparen(vector<Token> tokens, int start, int end){
    int depth = 0;
    for (int i = start; i < end; i++){
        if (tokens[i].type == TOKEN_OPENPAREN) depth++;
        else if (tokens[i].type == TOKEN_CLOSEPAREN) depth--;
        if (depth == 0) return i;
    }
    return -1;
}
// start is the TOKEN_CLOSEPAREN location
int find_correspoding_openparen(vector<Token> tokens, int start, int end){
    int depth = 0;
    for (int i = start; i > end; i--){
        if (tokens[i].type == TOKEN_CLOSEPAREN) depth++;
        else if (tokens[i].type == TOKEN_OPENPAREN) depth--;
        if (depth == 0) return i;
    }
    return -1;
}


optional<Expression> parse_expression_from_tokens(vector<Token> tokens, Context ctx, int start, int end)
{
    if (end == -1) end = tokens.size();
    Token token = tokens[start];

    //debug
    // cout << "trying to parse : ";
    // for (int i = start; i < end; i++){
    //     cout << tokens[i].value << " ";
    // }
    // cout << endl;
    //debug

    // UNARY OPERATOR
    bool is_unary_operator = vector_contain(token.value, ctx.unary_operators);
    if (is_unary_operator){
        if (start+1 >= end) return std::nullopt;
        auto rightexpr_maybe = parse_expression_from_tokens(tokens, ctx, start+1, end);
        if (!rightexpr_maybe.has_value()) return std::nullopt;
        Expression rightexpr = rightexpr_maybe.value();
        Expression expr = {
            EXPRESSION_OPERATOR_UNARY,
            token.value,
            false,
            {rightexpr},
        };
        return std::make_optional(expr);
    }

    // Make sure we don't find binary operator in the front
    bool is_binary_operator = vector_contain(token.value, ctx.binary_operators);
    if (is_binary_operator) return std::nullopt;

    // VALUE (single value or expr inside a bracked)
    Expression leftexpr;
    int rightmost_parsed = start;
    if (token.type == TOKEN_SYMBOL){
        leftexpr = {
            EXPRESSION_VALUE, 
            token.value, 
            false, 
            {}
        };
        rightmost_parsed = start;
    } else if (token.type == TOKEN_OPENPAREN){
        int closeparen_id = find_correspoding_closeparen(tokens, start, end);
        if (closeparen_id == -1) return std::nullopt;
        auto maybe_leftexpr = parse_expression_from_tokens(tokens, ctx, start+1, closeparen_id);
        if (!maybe_leftexpr.has_value()) return std::nullopt;
        leftexpr = maybe_leftexpr.value();
        rightmost_parsed = closeparen_id;
    }

    // if there is nothing left, return
    if (rightmost_parsed+1 >= end) return std::make_optional(leftexpr);

    // if there is something, make sure it's an binary operator
    Token next_token = tokens[rightmost_parsed+1];
    bool is_binary_operator_next = vector_contain(next_token.value, ctx.binary_operators);
    if (!is_binary_operator_next) return std::nullopt;

    // BINARY OPERATOR
    auto rightexpr_maybe = parse_expression_from_tokens(tokens, ctx, rightmost_parsed+2, end);
    if (!rightexpr_maybe.has_value()) return std::nullopt;
    Expression rightexpr = rightexpr_maybe.value();
    Expression expr = {
        EXPRESSION_OPERATOR_BINARY,
        next_token.value,
        true,
        {leftexpr, rightexpr},
    };
    return std::make_optional(expr);
}

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
            EXPRESSION_OPERATOR_BINARY, 
            tokens[rightmost_parsed+1].value, 
            true, 
            {leftexpr.value(), rightexpr.value()}
        };
        return std::make_optional(expr);
    } else {
        // assume this is a unary operator
        auto rightexpr = parse_expression_from_tokens_old(tokens, ctx, rightmost_parsed+1, end);
        if (!rightexpr.has_value()) return std::nullopt;

        Expression expr = {
            EXPRESSION_OPERATOR_UNARY, 
            tokens[rightmost_parsed+1].value, 
            true, 
            {rightexpr.value()}
        };
        return std::make_optional(expr);
    }

}

optional<Expression> parse_expression(string str, Context ctx){
    auto tokens = tokenize(str);
    auto normalized_tokens = normalize_tokens(tokens, ctx);
    return parse_expression_from_tokens(normalized_tokens, ctx);
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
        EXPRESSION_OPERATOR_BINARY, 
        statement_symbol, 
        false, 
        {leftexpr.value(), rightexpr.value()}
    };
}
