#include <optional>
#include <string>
#include <vector>
#include <tuple>
#include "libequaio.h"

using std::optional;
using std::vector;
using std::string;
using std::map;
using std::tuple;

struct Task {
    Context context;
    map<string,Expression> rules;
    optional<Expression> target;
    optional<Expression> current;
    bool print_rhs_only;
    vector<tuple<Expression, string>> history;
    vector<string> error_messages;

    void print_state() const;
    void add_rule_eq(string name, string exprstr);
    // return false if failed
    bool set_target_eq(string exprstr);
    bool set_current_eq(string exprstr);
    void init_current_with_target_lhs();

    // userland

    void apply_function_to_both_side(string fstr, string varname, string custom_name = "");
    void apply_rule(string rulename, string custom_name = "");
};

struct Interface {
    Task task;
};
