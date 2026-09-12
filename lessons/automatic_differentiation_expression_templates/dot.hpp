#pragma once

#include "VecExpr.hpp"
#include "ExprTraits.hpp"







//--------------
// MULTIPLY dot(Vector , Vector)  <move to its own file because it eagerly reduces to a scalr>
//--------------
// The one real implementation.
template <typename LHS, typename RHS>
double dot(const VecExpr<LHS>& a, const VecExpr<RHS>& b) {
    double sum = 0.0;
    for (std::size_t i = 0; i < a.size(); ++i) sum += a[i] * b[i];
    return sum;
}

// operator* for Vector*Vector just delegates to dot() -- one
// implementation, two spellings, never at risk of disagreeing.
template <typename LHS, typename RHS>
double operator*(const VecExpr<LHS>& a, const VecExpr<RHS>& b) {
    return dot(a, b);
}
