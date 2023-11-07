#include "doctest.h"
#include "expression.h"
#include "interface.h"
// #include "parser.h"
// #include "arithmetic.h"

#include <iostream>
using namespace std;

TEST_CASE("Interface"){

    SUBCASE("Arithmetic"){
        Context ctx = {
            {"x"},
            {"+", "-", "*", "/", "="},
            {"_"},
            true,
        };
        Task task = { ctx };
        task.set_target_eq("x = 3");
        task.set_current_eq("18 + (2 * x)  = 24");
    }
}
