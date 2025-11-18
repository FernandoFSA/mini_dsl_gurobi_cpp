#include <iostream>
#include "VariableFactory.h"
#include "VarMacro.h"
#include "Expr.h"
#include "IndexSets.h"
using namespace mini::dsl;
int main() {
    IndexSet I = IndexSet::range(0, 4);
    auto x = VariableFactory::create("x", VariableType::CONTINUOUS, 0.0, 10.0);
    std::string name = make_name("x_", 1);
    std::cout << name << "\n";
    expr e = x.as_expr() + 5.0;
    std::cout << e.to_string() << "\n";
}
