#pragma once
//#include "Vector.hpp"
//#include "Matrix.hpp"




// THE central decision of this lesson -- and the exact idea your old
// ExpressionTemplates repo was reaching for (its A_Traits/ExprRef
// pattern), just with the dangling-reference bug still live in it there.
//
// Default: store BY VALUE. Correct for expression NODES (AddExpr,
// etc.) -- cheap to copy (just a couple of members) and frequently
// temporaries themselves, e.g. the inner (a+b) in (a+b)+c. Storing them
// by value is what makes that safe: no reference to a temporary that's
// about to die.
template <typename T>
struct ExprTraits { using ExprRef = T; };
// defines ExprRef as just another name for T



/*

// Specialized for Vector: store BY CONST REFERENCE. Vector is expensive
// to copy (a real heap buffer), and a named Vector passed into an
// expression outlives the expression using it in ordinary code -- so a
// reference is both safe and avoids an expensive copy.
template <>// <= this lets us fix the type(to)<Vector>(in this case) and avoid the copy
struct ExprTraits<Vector> { using ExprRef = const Vector&; };
// redefines ExprRef as "const Vectort&" for Vectors




//
//// Specialized for Matrix: store by const reference
// Matrix joins the same storage-trait system as Vector -- expensive to
// copy, so store by reference when used as an operand.
template <>
struct ExprTraits<Matrix> { using ExprRef = const Matrix&; };

//
//Matrix owns a real heap buffer (potentially large), 
//so copying it into an expression node would be wasteful and pointless 
//— you want a reference instead. Without this specialization, 
//Matrix would fall through to the default rule (ExprRef = T, store by value) 
//the same way AddExpr/SubExpr do 
//— and a MatVecMultExpr storing a Matrix by value would deep-copy 
//   the entire matrix every time you wrote M * x. 
//This line exists purely to opt Matrix out of that default, the same way the Vector one does.



*/