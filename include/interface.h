#include <optional>
#include <string>
#include <vector>
#include <tuple>
#include "expression.h"
#include "arithmetic.h"

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
    void add_rule_expr(string name, Expression expr);
    void add_rule_eq(string name, string exprstr);
    // return false if failed
    bool set_target_eq(string exprstr);
    bool set_current_eq(string exprstr);
    void init_current_with_target_lhs();
    void set_current_expr(Expression expr, string msg);
    // userland

    bool apply_function_to_both_side_expr(Expression fexpr, string varname, string custom_name = "");
    bool apply_rule_expr(Expression expr, string custom_name = "");
    bool apply_function_to_both_side(string fstr, string varname, string custom_name = "");
    bool apply_rule(string rulename, string custom_name = "");
    bool try_swap_two_element(address addr1, address addr2, string custom_name = "");

    // arithmetics
    bool apply_arithmetic_to_both_side(Arithmetic::Operator op, string value, string custom_name = "");
    bool apply_arithmetic_calculation(string left, string right, Arithmetic::Operator op, string custom_name = "");
    bool apply_arithmetic_turn_subtraction_to_addition(string custom_name = "");
    bool apply_arithmetic_turn_addition_to_subtraction(string custom_name = "");
    bool apply_arithmetic_turn_division_to_multiplication(string custom_name = "");
    bool apply_arithmetic_turn_multiplication_to_division(string custom_name = "");
    bool apply_arithmetic_remove_assoc_parentheses(string custom_name = "");
};

struct Interface {
    Task task;
};
