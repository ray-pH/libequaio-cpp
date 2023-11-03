#include "doctest.h"
#include "libequaio.h"
#include "parser.h"

bool isToken(Token t, TokenType type, string value){
    return t.type == type && t.value == value;
}

TEST_CASE("Testing Parser") {
    int iota = 0;
    SUBCASE("Tokens"){
        Context ctx = {
            {"a", "b", "c"},
            {"+", "-", "*", "="},
            {"_"}
        };
        string str = "_ a + _ b * _ c";
        auto tokens = tokenize(str);
        SUBCASE("Tokenization"){
            iota = 0;
            CHECK(tokens.size() == 8);
            CHECK(isToken(tokens[iota++], TOKEN_SYMBOL, "_"));
            CHECK(isToken(tokens[iota++], TOKEN_SYMBOL, "a"));
            CHECK(isToken(tokens[iota++], TOKEN_SYMBOL, "+"));
            CHECK(isToken(tokens[iota++], TOKEN_SYMBOL, "_"));
            CHECK(isToken(tokens[iota++], TOKEN_SYMBOL, "b"));
            CHECK(isToken(tokens[iota++], TOKEN_SYMBOL, "*"));
            CHECK(isToken(tokens[iota++], TOKEN_SYMBOL, "_"));
            CHECK(isToken(tokens[iota++], TOKEN_SYMBOL, "c"));
        }

        auto normalized_token = normalize_tokens(tokens, ctx);
        SUBCASE("Normalization"){
            iota = 0;
            CHECK(normalized_token.size() == 18);
            CHECK(isToken(normalized_token[iota++], TOKEN_OPENPAREN, "("));
            CHECK(isToken(normalized_token[iota++], TOKEN_OPENPAREN, "("));
            CHECK(isToken(normalized_token[iota++], TOKEN_OPENPAREN, "("));
            CHECK(isToken(normalized_token[iota++], TOKEN_SYMBOL, "_"));
            CHECK(isToken(normalized_token[iota++], TOKEN_SYMBOL, "a"));
            CHECK(isToken(normalized_token[iota++], TOKEN_CLOSEPAREN, ")"));

            CHECK(isToken(normalized_token[iota++], TOKEN_SYMBOL, "+"));

            CHECK(isToken(normalized_token[iota++], TOKEN_OPENPAREN, "("));
            CHECK(isToken(normalized_token[iota++], TOKEN_SYMBOL, "_"));
            CHECK(isToken(normalized_token[iota++], TOKEN_SYMBOL, "b"));
            CHECK(isToken(normalized_token[iota++], TOKEN_CLOSEPAREN, ")"));
            CHECK(isToken(normalized_token[iota++], TOKEN_CLOSEPAREN, ")"));

            CHECK(isToken(normalized_token[iota++], TOKEN_SYMBOL, "*"));

            CHECK(isToken(normalized_token[iota++], TOKEN_OPENPAREN, "("));
            CHECK(isToken(normalized_token[iota++], TOKEN_SYMBOL, "_"));
            CHECK(isToken(normalized_token[iota++], TOKEN_SYMBOL, "c"));
            CHECK(isToken(normalized_token[iota++], TOKEN_CLOSEPAREN, ")"));
            CHECK(isToken(normalized_token[iota++], TOKEN_CLOSEPAREN, ")"));
        }

        SUBCASE("Find Corresponding Parenthesis"){
            string str = "((_ a) + _ b) * _ c";
            auto tokens = tokenize(str);

            int openpar_pos = 1;
            CHECK(4 == find_correspoding_closeparen(tokens, openpar_pos, tokens.size()));
            int closepar_pos = 8;
            CHECK(0 == find_correspoding_openparen(tokens, closepar_pos, -1));
        }

        SUBCASE("Parser"){
            string str0 = "a + b * c";
            auto expr0 = parse_expression(str0, ctx);
            CHECK(expr0.has_value());
            CHECK(expr0.value().to_string() == "((a + b) * c)");

            string str1 = "a + b = c * a";
            auto expr1 = parse_statement(str1, "=", ctx);
            CHECK(expr1.has_value());
            CHECK(expr1.value().to_string() == "(a + b) = (c * a)");
            CHECK(expr1.value() == parse_expression("(a + b) = (c * a)", ctx).value());
        }


    }
}
