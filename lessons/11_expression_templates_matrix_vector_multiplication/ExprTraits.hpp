#pragma once
#include "Vector.hpp"




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


// Specialized for Vector: store BY CONST REFERENCE. Vector is expensive
// to copy (a real heap buffer), and a named Vector passed into an
// expression outlives the expression using it in ordinary code -- so a
// reference is both safe and avoids an expensive copy.
template <>// <= this lets us fix the type(to)<Vector>(in this case) and avoid the copy
struct ExprTraits<Vector> { using ExprRef = const Vector&; };
// redefines ExprRef as "const Vectort&" for Vectors