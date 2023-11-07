#include "expression.h"
#include <vector>
using std::vector;

namespace BlockDisplay{

typedef enum{
    BASIC,
    VALUE, // the only one that can have value (at least for now)
    FRAC,  // two children, for the top and the bottom
} Type;

struct MetaData{
    address addr;
    Expression* expr;
    address leftaddr;
    address rightaddr;
};

// Type type;
// string value;
// vector<Block> child;
// MetaData metadata;
struct Block{
    Type type;
    string value;
    vector<Block> children;
    MetaData metadata;

    void append(Block b);
    void append(vector<Block> b);
    void prepend(Block b);
    void prepend(vector<Block> b);
    string to_string() const;
};

std::ostream& operator<<(std::ostream& os, const Block& b);
Block from_expression(Expression expr, Context ctx);

}
