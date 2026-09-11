#include <iostream>
#include <memory>

//#include "../include/tests_array.hpp"
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



int main() {
    Vector a(3), b(3), c(3);
    a[0] = 1; a[1] = 2; a[2] = 3;
    b[0] = 10; b[1] = 20; b[2] = 30;
    c[0] = 100; c[1] = 200; c[2] = 300;

    // a + b + c parses as (a + b) + c. The inner (a+b) is a TEMPORARY
    // AddExpr<Vector,Vector>. It gets copied (cheaply -- ExprTraits
    // says "by value" for expression nodes) into the outer
    // AddExpr<AddExpr<Vector,Vector>, Vector> BEFORE that temporary is
    // destroyed. That's the fix from lesson 1, finally applied where it
    // actually matters.
    Vector result = evaluate(a + b + c);

    std::cout << "a + b + c = [";
    for (std::size_t i = 0; i < result.size(); ++i) {
        std::cout << result[i];
        if (i + 1 < result.size()) std::cout << ", ";
    }
    std::cout << "]\n(expected [111, 222, 333])\n";



    Vector result2 = evaluate(c - b - a);

    std::cout << "c - b - a = [";
    for (std::size_t i = 0; i < result2.size(); ++i) {
        std::cout << result2[i];
        if (i + 1 < result2.size()) std::cout << ", ";
    }
    std::cout << "]\n(expected [89, 178, 267])\n";

    return 0;
}




/*

this is the key:

for (std::size_t i = 0; i < expr.size(); ++i) result[i] = expr[i];

* note the type of expr is VecExpr<Derived>,
* so expr[i] calls operator[](std::size_t i) const on the VecExpr<Derived> object,
* which calls self() to get the Derived object,
* and then calls operator[](std::size_t i) const on the Derived object, 
* which is either a Vector or an AddExpr<LHS,RHS>.
* 
* how does it resolve to AddExpr<LHS,RHS>::operator[](std::size_t i) const?  
  * the Derived type is AddExpr<LHS,RHS>, so self() returns a const AddExpr<LHS,RHS>&, 
  * and then operator[](std::size_t i) const 
  * on that object calls AddExpr<LHS,RHS>::operator[](std::size_t i) const, 
  * which returns lhs_[i] + rhs_[i], which are either Vectors or other AddExprs, 
  * and so on recursively until we hit the leaf Vectors, which return their actual data.
  * 
  * 
* why is the Derived type is AddExpr<LHS,RHS>?  
    * Answer: because we have an overload of operator+ that returns an AddExpr<LHS,RHS> 
	*   we have no other operator+ overloads for VecExprs, so the compiler chooses that one
	*   but how did it know it was a VecExpr<LHS> and VecExpr<RHS>?
	*       - I thought it figured it out purely because there were no other operator+ overloads
* but how does it match the right operands?  The operands look like vectors to me.
    Answer: 
 
	*       - because the operator+ is a template that takes two VecExprs,
    * when we add two VecExprs together.
        * because the expression a + b + c parses as (a + b) + c, 
        * so the inner (a+b) is an AddExpr<Vector,Vector>, 
        * and the outer (a+b)+c is an AddExpr<AddExpr<Vector,Vector>, Vector>, 
        * so the Derived type is AddExpr<AddExpr<Vector,Vector>, Vector> for the outer expression.
*/