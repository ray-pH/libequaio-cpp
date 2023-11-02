#include "libequaio.h"

vector<Token> tokenize(string str);
vector<Token> normalize_tokens(vector<Token> tokens, Context ctx);

int find_correspoding_closeparen(vector<Token> tokens, int start, int end);
int find_correspoding_openparen(vector<Token> tokens, int start, int end);
optional<Expression> parse_expression_from_tokens(vector<Token> tokens, Context ctx, int start, int end);
optional<Expression> parse_expression(string str, Context ctx);
optional<Expression> parse_statement(string str, string statement_symbol, Context ctx);
