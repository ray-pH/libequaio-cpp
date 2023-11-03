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
        SUBCASE("Turn subtraction to addition"){
            auto expr = parse_expression("a - b - a + c", ctx);
            CHECK(expr.has_value());
            auto expr_adds = Arithmetic::turn_subtraction_to_addition(expr.value());
            CHECK(expr_adds == parse_expression("a + (_ b) + (_ a) + c", ctx));
        }
        SUBCASE("Addition"){
        }
    }
}
