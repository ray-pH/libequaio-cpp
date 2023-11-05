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

Block __from_expression(Expression &rootexpr, address addr, Context ctx){
    (void)ctx;

    auto expr = rootexpr.at(addr);
    switch (expr.type){
        case EXPRESSION_OPERATOR_BINARY: {
            Block self = { VALUE, expr.symbol, {}, {addr, &rootexpr} };
            Block container = { BASIC, "", {self} };

            address addrleft(addr);
            addrleft.push_back(0);
            Block left  = __from_expression(rootexpr, addrleft, ctx);
            address addrright(addr);
            addrright.push_back(1);
            Block right = __from_expression(rootexpr, addrright, ctx);

            container.append(right.child);
            container.prepend(left.child);
            return container;
        }
        case EXPRESSION_OPERATOR_UNARY: {
            Block self = { VALUE, expr.symbol, {}, {addr, &rootexpr} };
            Block container = { BASIC, "", {self} };

            address addrinner(addr);
            addrinner.push_back(0);
            Block inner = __from_expression(rootexpr, addrinner, ctx);
            container.append(inner.child);
            return container;
        }
        case EXPRESSION_VALUE: {
            Block inner = { VALUE, expr.symbol, {}, {addr, &rootexpr}};
            Block container = { BASIC, "", {inner} };
            return container;
        }
    }
    return {};
}

Block BlockDisplay::from_expression(Expression expr, Context ctx){
    (void)ctx;
    return __from_expression(expr, {}, ctx);
}
