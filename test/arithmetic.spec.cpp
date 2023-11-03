#include "doctest.h"
#include "libequaio.h"
#include "parser.h"
#include "arithmetic.h"

#include <iostream>
using namespace std;

TEST_CASE("Arithmetic"){
    Context ctx = {
        {"a", "b", "c"},
        {"+", "-", "*", "="},
        {"_"},
        true,
    };

    optional<Expression> calc;
    SUBCASE("Calculation"){
        SUBCASE("integers"){
            calc = Arithmetic::create_calculation("27", "14", Arithmetic::OPERATOR_SUB);
            CHECK(calc.has_value());
            CHECK(calc.value() == parse_expression("27 - 14 = 13", ctx));

            calc = Arithmetic::create_calculation("27", "14", Arithmetic::OPERATOR_MUL);
            CHECK(calc.has_value());
            CHECK(calc.value() == parse_expression("27 * 14 = 378", ctx));
        }
        SUBCASE("floats"){
            //TODO: handle floats
        }
    }

    SUBCASE("Rearranging Commutative Operator"){
        SUBCASE("Creating the rule"){
            auto ctx = Arithmetic::arithmetic_context;
            Expression rule = Arithmetic::create_rule_commute(1,3, 5, Arithmetic::OPERATOR_ADD);
            auto expr_initial = parse_expression("0 + 1 + 2 + 3 + 4", ctx).value();
            auto expr_expect  = parse_expression("0 + 3 + 2 + 1 + 4", ctx).value();
            CHECK(expr_expect == expr_initial.apply_rule_equal(rule, ctx)[0]);
        }

        SUBCASE("Turn subtraction to addition"){
            auto expr = parse_expression("a - b - a + c", ctx);
            CHECK(expr.has_value());
            auto expr_adds = Arithmetic::turn_subtraction_to_addition(expr.value());
            CHECK(expr_adds == parse_expression("a + (_ b) + (_ a) + c", ctx));

            auto expr_subs = Arithmetic::turn_addition_to_subtraction(expr_adds);
            CHECK(expr_subs == parse_expression("a - b - a + c", ctx));
        }
        SUBCASE("Addition"){
        }
    }
}
