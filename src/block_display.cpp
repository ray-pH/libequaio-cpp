#include "block_display.h"
#include <assert.h>
#include <iostream>

using namespace BlockDisplay;

void BlockDisplay::Block::append(Block b){ this->child.push_back(b); }
void BlockDisplay::Block::append(vector<Block> b){ this->child.insert(this->child.end(), b.begin(), b.end()); }
void BlockDisplay::Block::prepend(Block b){ this->child.insert(this->child.begin(), b); }
void BlockDisplay::Block::prepend(vector<Block> b){ this->child.insert(this->child.begin(), b.begin(), b.end()); }

string BlockDisplay::Block::to_string() const{
    switch (this->type){
        case BASIC:{
            string st = "";
            for (const auto& c : this->child){
                st += c.to_string() + " ";
            }
            return st;
        }
        case VALUE:
            return this->value;
        case FRAC:
            return "{" + this->child[0].to_string() + "}" 
                    + "/" + "{" + this->child[1].to_string() + "}";
    }
    return "";
}
std::ostream& BlockDisplay::operator<<(std::ostream& os, const Block& b){
    os << b.to_string();
    return os;
}



const Block openparen = { VALUE, "(" };
const Block closeparen = { VALUE, ")" };

Block BlockDisplay::from_expression(Expression expr, Context ctx){
    (void)ctx;
    switch (expr.type){
        case EXPRESSION_OPERATOR_BINARY: {
            Block self = { VALUE, expr.symbol };
            Block container = { BASIC, "", {self} };

            Block left  = from_expression(expr.child[0], ctx);
            Block right = from_expression(expr.child[1], ctx);

            container.append(right.child);
            container.prepend(left.child);
            return container;
        }
        case EXPRESSION_OPERATOR_UNARY: {
            Block self = { VALUE, expr.symbol };
            Block container = { BASIC, "", {self} };

            Block inner = from_expression(expr.child[0], ctx);
            container.append(inner.child);
            return container;
        }
        case EXPRESSION_VALUE: {
            Block inner = { VALUE, expr.symbol };
            Block container = { BASIC, "", {inner} };
            return container;
        }
    }
    return {};
}
