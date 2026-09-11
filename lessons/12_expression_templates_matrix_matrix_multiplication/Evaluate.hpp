#include "VecExpr.hpp"
#include "Vector.hpp"
#include "ExprTraits.hpp"
#include "Operators.hpp"







// The "DependentVar"-equivalent moment: forces a lazy expression tree
// into a real, concrete Vector.
template <typename Derived>
Vector evaluate(const VecExpr<Derived>& expr) {
    Vector result(expr.size());// allocate a real Vector called "result" to hold the result
    // double operator[](std::size_t i) const { return lhs_[i] + rhs_[i]; }
    for (std::size_t i = 0; i < expr.size(); ++i) result[i] = expr[i]; // very lazy evaluation: 
    // note the type of expr is VecExpr<Derived>, 
    // so expr[i] calls operator[](std::size_t i) const 
    // on the VecExpr<Derived> object, which calls self() to get the Derived object, 
    // and then calls operator[](std::size_t i) const on the Derived object, 
    // which is either a Vector or an AddExpr<LHS,RHS>.
    //
    // just loop over the expression and fill in the result Vector.  
    // This is where the actual computation happens, and where the expression tree is traversed.
    return result;
}

